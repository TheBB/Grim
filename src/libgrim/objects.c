#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "gmp.h"
#include "uniconv.h"

#include "grim.h"
#include "internal.h"


grim_object grim_symbol_table;


grim_tag_t grim_direct_tag(grim_object obj) {
    if ((obj & GRIM_FIXNUM_TAG) != 0)
        return GRIM_FIXNUM_TAG;
    return obj & 0x0f;
}

grim_type_t grim_type(grim_object obj) {
    switch (grim_direct_tag(obj)) {
    case GRIM_FIXNUM_TAG: return GRIM_INTEGER;
    case GRIM_CHARACTER_TAG: return GRIM_CHARACTER;
    case GRIM_SYMBOL_TAG: return GRIM_SYMBOL;
    case GRIM_FALSE_TAG: case GRIM_TRUE_TAG: return GRIM_BOOLEAN;
    case GRIM_NIL_TAG: return GRIM_NIL;
    case GRIM_INDIRECT_TAG:
        switch (I_tag(obj)) {
        case GRIM_FLOAT_TAG: return GRIM_FLOAT;
        case GRIM_BIGINT_TAG: return GRIM_INTEGER;
        case GRIM_RATIONAL_TAG: return GRIM_RATIONAL;
        case GRIM_COMPLEX_TAG: return GRIM_COMPLEX;
        case GRIM_STRING_TAG: return GRIM_STRING;
        case GRIM_VECTOR_TAG: return GRIM_VECTOR;
        case GRIM_CONS_TAG: return GRIM_CONS;
        case GRIM_BUFFER_TAG: return GRIM_BUFFER;
        case GRIM_HASHTABLE_TAG: return GRIM_HASHTABLE;
        case GRIM_CELL_TAG: return GRIM_CELL;
        case GRIM_MODULE_TAG: return GRIM_MODULE;
        case GRIM_CFUNC_TAG: case GRIM_LFUNC_TAG: return GRIM_FUNCTION;
        case GRIM_FRAME_TAG: return GRIM_FRAME;
        }
    default: case GRIM_UNDEFINED_TAG: return GRIM_UNDEFINED;
    }
}


grim_object grim_indirect_create(bool permanent) {
    grim_indirect *retval;
    if (permanent)
        retval = GC_MALLOC_UNCOLLECTABLE(sizeof(grim_indirect));
    else
        retval = GC_MALLOC(sizeof(grim_indirect));
    assert(retval);
    return (grim_object) retval;
}


// Strings
// -----------------------------------------------------------------------------

static void grim_string_finalize(void *obj, void *_) {
    (void)_;
    free(I_str(obj));
}

static grim_object grim_string_create() {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_STRING_TAG;
    I_setstr(obj, NULL);
    I_strlen(obj) = 0;
    GC_REGISTER_FINALIZER((void*) obj, grim_string_finalize, NULL, NULL, NULL);
    return obj;
}

grim_object grim_string_pack(const char *input, const char *encoding, bool unescape) {
    return grim_nstring_pack(input, strlen(input), encoding, unescape);
}

grim_object grim_nstring_pack(const char *input, size_t length, const char *encoding, bool unescape) {
    grim_object obj = grim_string_create();
    if (!encoding) {
        I_strlen(obj) = length;
        I_setstr(obj, malloc((I_strlen(obj) + 1) * sizeof(char)));
        strcpy((char *) I_str(obj), input);
    }
    else
        I_setstr(obj, u8_conv_from_encoding(encoding, iconveh_error, input, length, NULL, NULL, &I_strlen(obj)));
    assert(I_str(obj));
    if (unescape)
        grim_unescape_string(obj);
    return obj;
}


// Vectors
// -----------------------------------------------------------------------------

grim_object grim_vector_create(size_t nelems) {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_VECTOR_TAG;
    assert((I_vectordata(obj) = GC_MALLOC(nelems * sizeof(grim_object))));
    I_vectorlen(obj) = nelems;
    for (size_t i = 0; i < nelems; i++)
        I_vectordata(obj)[i] = grim_undefined;
    return obj;
}


// Cons cells
// -----------------------------------------------------------------------------

grim_object grim_cons_pack(grim_object car, grim_object cdr) {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_CONS_TAG;
    I_car(obj) = car;
    I_cdr(obj) = cdr;
    return obj;
}


// Symbols
// -----------------------------------------------------------------------------

static grim_object grim_symbol_create(grim_object name) {
    grim_object obj = grim_indirect_create(true) | GRIM_SYMBOL_TAG;
    I_symbolname(obj) = name;
    return obj;
}

grim_object grim_intern(const char *name, const char *encoding) {
    return grim_nintern(name, strlen(name), encoding);
}

grim_object grim_nintern(const char *name, size_t length, const char *encoding) {
    grim_object str = grim_nstring_pack(name, length, encoding, false);
    grim_object sym = grim_hashtable_get(grim_symbol_table, str);
    if (sym != grim_undefined)
        return sym;
    sym = grim_symbol_create(str);
    grim_hashtable_set(grim_symbol_table, str, sym);
    return sym;
}


// Characters
// -----------------------------------------------------------------------------

grim_object grim_character_pack(ucs4_t ch) {
    return (((grim_object) ch) << 8) | GRIM_CHARACTER_TAG;
}

grim_object grim_character_pack_name(const char *name, const char *encoding) {
    return grim_ncharacter_pack_name(name, strlen(name), encoding);
}

grim_object grim_ncharacter_pack_name(const char *name, size_t length, const char *encoding) {
    uint8_t *str;
    size_t u8len;
    if (!encoding) {
        str = (uint8_t *) name;
        u8len = length;
    }
    else
        str = u8_conv_from_encoding(encoding, iconveh_error, name, length, NULL, NULL, &u8len);
    ucs4_t ch = grim_unescape_character(str, u8len);
    if (encoding)
        free(str);
    return grim_character_pack(ch);
}

ucs4_t grim_character_extract(grim_object obj) {
    return (ucs4_t) (obj >> 8);
}


// Buffers
// -----------------------------------------------------------------------------

#define GRIM_BUFFER_GROWTH_FACTOR (1.5)
#define GRIM_BUFFER_MIN_SIZE (1024)

static void grim_buffer_finalize(void *obj, void *_) {
    (void)_;
    free(I_buf(obj));
}

grim_object grim_buffer_create(size_t sizehint) {
    if (sizehint < GRIM_BUFFER_MIN_SIZE)
        sizehint = GRIM_BUFFER_MIN_SIZE;
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_BUFFER_TAG;
    assert((I_buf(obj) = malloc(sizehint)));
    I_buflen(obj) = 0;
    I_bufcap(obj) = sizehint;
    GC_REGISTER_FINALIZER((void*) obj, grim_buffer_finalize, NULL, NULL, NULL);
    return obj;
}

void grim_buffer_dump(FILE *stream, grim_object obj) {
    fprintf(stream, "%*s", (int) I_buflen(obj), I_buf(obj));
}

void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint) {
    size_t required = I_buflen(obj) + sizehint;
    while (I_bufcap(obj) < required) {
        size_t newsize = (size_t) (I_bufcap(obj) * GRIM_BUFFER_GROWTH_FACTOR);
        assert((I_buf(obj) = realloc(I_buf(obj), newsize)));
        I_bufcap(obj) = newsize;
    }
}

void grim_buffer_copy(grim_object obj, const char *data, size_t length) {
    grim_buffer_ensure_free_capacity(obj, length);
    memcpy(I_buf(obj) + I_buflen(obj), data, length);
    I_buflen(obj) += length;
}


// Hash tables
// -----------------------------------------------------------------------------

#define GRIM_HASHTABLE_MAX_FILL (0.9)
#define GRIM_HASHTABLE_GROWTH_FACTOR (1.5)
#define GRIM_HASHTABLE_MIN_SIZE (1024)

grim_object grim_hashtable_create(size_t sizehint) {
    if (sizehint < GRIM_HASHTABLE_MIN_SIZE)
        sizehint = GRIM_HASHTABLE_MIN_SIZE;
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_HASHTABLE_TAG;
    I_hashnodes(obj) = GC_MALLOC(sizehint * sizeof(grim_hashnode *));
    assert(I_hashnodes(obj));
    I_hashcap(obj) = sizehint;
    I_hashfill(obj) = 0;
    for (size_t i = 0; i < sizehint; i++)
        I_hashnodes(obj)[i] = NULL;
    return obj;
}

static grim_hashnode **grim_hashtable_node(grim_hashnode **nodes, grim_object key, size_t hash, size_t length, bool create) {
    size_t index = hash % length;
    grim_hashnode **node = &nodes[index];
    while ((*node) && !grim_equal(key, (*node)->key))
        node = &(*node)->next;
    if (*node || !create)
        return node;
    grim_hashnode *newnode = GC_MALLOC(sizeof(grim_hashnode));
    newnode->key = key;
    newnode->value = grim_undefined;
    newnode->hash = hash;
    newnode->next = NULL;
    *node = newnode;
    return node;
}

static void grim_hashtable_grow(grim_object table) {
    size_t oldsize = I_hashcap(table);
    grim_hashnode **oldnodes = I_hashnodes(table);
    size_t newsize = (size_t) (oldsize * GRIM_BUFFER_GROWTH_FACTOR);
    grim_hashnode **newnodes = GC_MALLOC(newsize * sizeof(grim_hashnode *));
    for (size_t i = 0; i < oldsize; i++) {
        grim_hashnode *srcnode = oldnodes[i];
        while (srcnode) {
            grim_hashnode **tgtnode = grim_hashtable_node(
                newnodes, srcnode->key, srcnode->hash, newsize, true);
            (*tgtnode)->value = srcnode->value;
            srcnode = srcnode->next;
        }
    }
    I_hashcap(table) = newsize;
    I_hashnodes(table) = newnodes;
}

bool grim_hashtable_has(grim_object table, grim_object key) {
    grim_hashnode **node = grim_hashtable_node(
        I_hashnodes(table), key, grim_hash(key, 0), I_hashcap(table), false);
    return (*node) ? true : false;
}

grim_object grim_hashtable_get(grim_object table, grim_object key) {
    grim_hashnode **node = grim_hashtable_node(
        I_hashnodes(table), key, grim_hash(key, 0), I_hashcap(table), false);
    return (*node) ? (*node)->value : grim_undefined;
}

void grim_hashtable_set(grim_object table, grim_object key, grim_object value) {
    size_t hash = grim_hash(key, 0);
    grim_hashnode **node = grim_hashtable_node(I_hashnodes(table), key, hash, I_hashcap(table), true);
    if ((*node)->value == grim_undefined && I_hashfill(table) > I_hashcap(table) * GRIM_HASHTABLE_MAX_FILL) {
        grim_hashtable_grow(table);
        node = grim_hashtable_node(I_hashnodes(table), key, hash, I_hashcap(table), true);
    }
    if ((*node)->value == grim_undefined)
        I_hashfill(table)++;
    (*node)->value = value;
}

void grim_hashtable_unset(grim_object table, grim_object key) {
    size_t hash = grim_hash(key, 0);
    grim_hashnode **node = grim_hashtable_node(I_hashnodes(table), key, hash, I_hashcap(table), false);
    if (*node) {
        *node = (*node)->next;
        I_hashfill(table)--;
    }
}


// Cells
// -----------------------------------------------------------------------------

grim_object grim_cell_pack(grim_object value) {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_CELL_TAG;
    I_cellvalue(obj) = value;
    return obj;
}


// Modules
// -----------------------------------------------------------------------------

grim_object grim_module_create(grim_object name) {
    grim_object ind = grim_indirect_create(false);
    I_tag(ind) = GRIM_MODULE_TAG;
    I_modulename(ind) = name;
    I_modulemembers(ind) = grim_hashtable_create(0);
    return ind;
}

grim_object grim_module_cell(grim_object module, grim_object name, bool require) {
    if (!grim_hashtable_has(I_modulemembers(module), name)) {
        assert(!require);
        grim_object cell = grim_cell_pack(grim_undefined);
        grim_hashtable_set(I_modulemembers(module), name, cell);
        return cell;
    }
    return grim_hashtable_get(I_modulemembers(module), name);
}

grim_object grim_module_get(grim_object module, grim_object name) {
    return I_cellvalue(grim_module_cell(module, name, true));
}

void grim_module_set(grim_object module, grim_object name, grim_object value) {
    I_cellvalue(grim_module_cell(module, name, false)) = value;
}


// Functions
// -----------------------------------------------------------------------------

grim_object grim_cfunc_create(grim_cfunc *cfunc, uint8_t nargs, bool variadic) {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_CFUNC_TAG;
    I_cfunc(obj) = cfunc;
    I_nargs(obj) = nargs;
    I_variadic(obj) = variadic;
    return obj;
}

grim_object grim_lfunc_create(grim_object bytecode, grim_object refs, uint8_t nlocals, uint8_t nargs, bool variadic) {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_LFUNC_TAG;
    I_bytecode(obj) = bytecode;
    I_funcrefs(obj) = refs;
    I_nlocals(obj) = nlocals;
    I_nargs(obj) = nargs;
    I_variadic(obj) = variadic;
    return obj;
}


// Frames
// -----------------------------------------------------------------------------

grim_object grim_frame_create(grim_object func, grim_object parent) {
    grim_object frame = grim_indirect_create(false);
    I_tag(frame) = GRIM_FRAME_TAG;
    I_framefunc(frame) = func;

    size_t nargs = I_nargs(func) + I_variadic(func);
    I_framestack(frame) = grim_vector_create(nargs + I_nlocals(func) + GRIM_STACK_SIZE);
    I_parentframe(frame) = parent;
    return frame;
}
