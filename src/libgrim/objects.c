#include <assert.h>
#include <stdbool.h>
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

grim_tag_t grim_indirect_tag(grim_object obj) {
    return *((grim_tag_t *) obj);
}

grim_type_t grim_type(grim_object obj) {
    switch (grim_direct_tag(obj)) {
    case GRIM_FIXNUM_TAG: return GRIM_INTEGER;
    case GRIM_CHARACTER_TAG: return GRIM_CHARACTER;
    case GRIM_SYMBOL_TAG: return GRIM_SYMBOL;
    case GRIM_FALSE_TAG: case GRIM_TRUE_TAG: return GRIM_BOOLEAN;
    case GRIM_NIL_TAG: return GRIM_NIL;
    case GRIM_INDIRECT_TAG:
        switch (grim_indirect_tag(obj)) {
        case GRIM_FLOAT_TAG: return GRIM_FLOAT;
        case GRIM_BIGINT_TAG: return GRIM_INTEGER;
        case GRIM_RATIONAL_TAG: return GRIM_RATIONAL;
        case GRIM_COMPLEX_TAG: return GRIM_COMPLEX;
        case GRIM_STRING_TAG: return GRIM_STRING;
        case GRIM_VECTOR_TAG: return GRIM_VECTOR;
        case GRIM_CONS_TAG: return GRIM_CONS;
        case GRIM_BUFFER_TAG: return GRIM_BUFFER;
        case GRIM_HASHTABLE_TAG: return GRIM_HASHTABLE;
        }
    default: case GRIM_UNDEFINED_TAG: return GRIM_UNDEFINED;
    }
}


static grim_indirect *grim_indirect_create(bool permanent) {
    grim_indirect *retval;
    if (permanent)
        retval = GC_MALLOC_UNCOLLECTABLE(sizeof(grim_indirect));
    else
        retval = GC_MALLOC(sizeof(grim_indirect));
    assert(retval);
    return retval;
}


// Floats
// -----------------------------------------------------------------------------

grim_object grim_float_pack(double num) {
    grim_indirect *obj = grim_indirect_create(false);
    obj->floating = num;
    return (grim_object) obj;
}

double grim_float_extract(grim_object obj) {
    return ((grim_indirect *) obj)->floating;
}


// Integers
// -----------------------------------------------------------------------------

static void grim_bigint_finalize(void *obj, void *_) {
    (void)_;
    mpz_clear(((grim_indirect *) obj)->bigint);
}

static grim_indirect *grim_bigint_create() {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_BIGINT_TAG;
    mpz_init(obj->bigint);
    GC_REGISTER_FINALIZER(obj, grim_bigint_finalize, NULL, NULL, NULL);
    return obj;
}

bool grim_integer_extractable(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return true;
    return mpz_fits_slong_p(((grim_indirect *) obj)->bigint);
}

intmax_t grim_integer_extract(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG) {
        bool signbit = ((intptr_t)obj) < 0;
        intptr_t ret = obj >> 1;
        return signbit ? (INTPTR_MIN | ret) : ret;
    }
    return mpz_get_si(((grim_indirect *) obj)->bigint);
}

grim_object grim_integer_pack(intmax_t num) {
    if (num >= GRIM_FIXNUM_MIN && num <= GRIM_FIXNUM_MAX)
        return (grim_object) ((uintptr_t) num << 1) | GRIM_FIXNUM_TAG;
    grim_indirect *obj = grim_bigint_create();
    mpz_set_si(obj->bigint, num);
    return (grim_object) obj;
}

static void grim_integer_to_mpz(mpz_t tgt, grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        mpz_set_si(tgt, grim_integer_extract(obj));
    else
        mpz_set(tgt, ((grim_indirect *) obj)->bigint);
}

static grim_object grim_mpz_to_integer(mpz_t src) {
    if (mpz_fits_slong_p(src))
        return grim_integer_pack(mpz_get_si(src));
    grim_indirect *obj = grim_bigint_create();
    mpz_set(obj->bigint, src);
    return (grim_object) obj;
}


// Rationals
// -----------------------------------------------------------------------------

static void grim_rational_finalize(void *obj, void *_) {
    (void)_;
    mpq_clear(((grim_indirect *) obj)->rational);
}

static grim_indirect *grim_rational_create() {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_RATIONAL_TAG;
    mpq_init(obj->rational);
    GC_REGISTER_FINALIZER(obj, grim_rational_finalize, NULL, NULL, NULL);
    return obj;
}

grim_object grim_rational_pack(grim_object numerator, grim_object denominator) {
    assert(grim_type(numerator) == GRIM_INTEGER);
    assert(grim_type(denominator) == GRIM_INTEGER);

    if (grim_integer_extractable(denominator)) {
        intmax_t denom = grim_integer_extract(denominator);
        assert(denom != 0);
        if (denom == 1)
            return numerator;
    }

    mpz_t temp;
    mpq_t num, denom;
    mpz_init(temp);

    mpq_init(num);
    grim_integer_to_mpz(temp, numerator);
    mpq_set_z(num, temp);

    mpq_init(denom);
    grim_integer_to_mpz(temp, denominator);
    mpq_set_z(denom, temp);

    grim_indirect *obj = grim_rational_create();
    mpq_div(obj->rational, num, denom);

    mpz_clear(temp);
    mpq_clear(num);
    mpq_clear(denom);

    return (grim_object) obj;
}

grim_object grim_rational_numerator(grim_object obj) {
    return grim_mpz_to_integer(mpq_numref(((grim_indirect *) obj)->rational));
}

grim_object grim_rational_denominator(grim_object obj) {
    return grim_mpz_to_integer(mpq_denref(((grim_indirect *) obj)->rational));
}


// Complex numbers
// -----------------------------------------------------------------------------

grim_object grim_complex_create(grim_object real, grim_object imag) {
    if (grim_type(imag) == GRIM_INTEGER &&
        grim_integer_extractable(imag) &&
        grim_integer_extract(imag) == 0)
        return real;

    if (grim_type(imag) == GRIM_FLOAT && grim_float_extract(imag) == 0.0)
        return real;

    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_COMPLEX_TAG;
    obj->real = real;
    obj->imag = imag;
    return (grim_object) obj;
}

grim_object grim_complex_real(grim_object obj) {
    return ((grim_indirect *) obj)->real;
}

grim_object grim_complex_imag(grim_object obj) {
    return ((grim_indirect *) obj)->imag;
}


// Strings
// -----------------------------------------------------------------------------

static void grim_string_finalize(void *obj, void *_) {
    (void)_;
    free(((grim_indirect *) obj)->str);
}

static grim_indirect *grim_string_create() {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_STRING_TAG;
    obj->str = NULL;
    obj->strlen = 0;
    GC_REGISTER_FINALIZER(obj, grim_string_finalize, NULL, NULL, NULL);
    return obj;
}

grim_object grim_string_pack(const char *input, const char *encoding, bool unescape) {
    grim_indirect *obj = grim_string_create();
    if (!encoding) {
        obj->strlen = strlen(input);
        obj->str = malloc((obj->strlen + 1) * sizeof(char));
        strcpy((char *) obj->str, input);
    }
    else
        obj->str = u8_conv_from_encoding(encoding, iconveh_error, input, strlen(input), NULL, NULL, &obj->strlen);
    assert(obj->str);
    grim_object str = (grim_object) obj;
    if (unescape)
        grim_unescape_string(str);
    return str;
}

size_t grim_strlen(grim_object obj) {
    return ((grim_indirect *) obj)->strlen;
}

size_t grim_set_strlen(grim_object obj, size_t length) {
    return ((grim_indirect *) obj)->strlen = length;
}

uint8_t *grim_strptr(grim_object obj) {
    return ((grim_indirect *) obj)->str;
}


// Vectors
// -----------------------------------------------------------------------------

grim_object grim_vector_create(size_t nelems) {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_VECTOR_TAG;
    assert((obj->vectordata = GC_MALLOC(nelems * sizeof(grim_object))));
    obj->vectorlen = nelems;
    grim_object vec = (grim_object) obj;
    for (size_t i = 0; i < nelems; i++)
        grim_vector_set(vec, i, grim_undefined);
    return vec;
}

size_t grim_vector_size(grim_object vec) {
    return ((grim_indirect *) vec)->vectorlen;
}

void grim_vector_set(grim_object vec, size_t index, grim_object elt) {
    ((grim_indirect *) vec)->vectordata[index] = elt;
}

grim_object grim_vector_get(grim_object vec, size_t index) {
    return ((grim_indirect *) vec)->vectordata[index];
}


// Cons cells
// -----------------------------------------------------------------------------

grim_object grim_cons_create(grim_object car, grim_object cdr) {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_CONS_TAG;
    obj->car = car;
    obj->cdr = cdr;
    return (grim_object) obj;
}

grim_object grim_car(grim_object obj) {
    return ((grim_indirect *) obj)->car;
}

grim_object grim_cdr(grim_object obj) {
    return ((grim_indirect *) obj)->cdr;
}


// Symbols
// -----------------------------------------------------------------------------

static grim_object grim_symbol_create(grim_object name) {
    grim_indirect *obj = grim_indirect_create(true);
    obj->symbolname = name;
    return ((grim_object) obj) | GRIM_SYMBOL_TAG;
}

grim_object grim_intern(const char *name, const char *encoding) {
    grim_object str = grim_string_pack(name, encoding, false);
    grim_object sym = grim_hashtable_get(grim_symbol_table, str);
    if (sym != grim_undefined)
        return sym;
    sym = grim_symbol_create(str);
    grim_hashtable_set(grim_symbol_table, str, sym);
    return sym;
}

grim_object grim_symbol_name(grim_object obj) {
    grim_indirect *wrapped = (grim_indirect *) (obj - GRIM_SYMBOL_TAG);
    return wrapped->symbolname;
}


// Characters
// -----------------------------------------------------------------------------

grim_object grim_character_pack(ucs4_t ch) {
    return (((grim_object) ch) << 8) | GRIM_CHARACTER_TAG;
}

grim_object grim_character_pack_name(const char *name, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    size_t u8len;
    uint8_t *str = u8_conv_from_encoding(encoding, iconveh_error, name, strlen(name), NULL, NULL, &u8len);
    ucs4_t ch = grim_unescape_character(str, u8len);
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
    free(((grim_indirect *) obj)->buf);
}

grim_object grim_buffer_create(size_t sizehint) {
    if (sizehint < GRIM_BUFFER_MIN_SIZE)
        sizehint = GRIM_BUFFER_MIN_SIZE;
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_BUFFER_TAG;
    assert((obj->buf = malloc(sizehint)));
    obj->buflen = 0;
    obj->bufcap = sizehint;
    GC_REGISTER_FINALIZER(obj, grim_buffer_finalize, NULL, NULL, NULL);
    return (grim_object) obj;
}

void grim_buffer_dump(FILE *stream, grim_object obj) {
    grim_indirect *ind = (grim_indirect *) obj;
    fprintf(stream, "%*s", (int) ind->buflen, ind->buf);
}

void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint) {
    grim_indirect *ind = (grim_indirect *) obj;
    size_t required = ind->buflen + sizehint;
    while (ind->bufcap < required) {
        size_t newsize = (size_t) (ind->bufcap * GRIM_BUFFER_GROWTH_FACTOR);
        printf("%lu\n", newsize);
        assert((ind->buf = realloc(ind->buf, newsize)));
        ind->bufcap = newsize;
    }
}

void grim_buffer_copy(grim_object obj, const char *data, size_t length) {
    grim_buffer_ensure_free_capacity(obj, length);
    grim_indirect *ind = (grim_indirect *) obj;
    memcpy(ind->buf + ind->buflen, data, length);
    ind->buflen += length;
}

char *grim_bufptr(grim_object obj) {
    return ((grim_indirect *) obj)->buf;
}

size_t grim_buflen(grim_object obj) {
    return ((grim_indirect *) obj)->buflen;
}


// Hash tables
// -----------------------------------------------------------------------------

#define GRIM_HASHTABLE_MAX_FILL (0.9)
#define GRIM_HASHTABLE_GROWTH_FACTOR (1.5)
#define GRIM_HASHTABLE_MIN_SIZE (1024)

grim_object grim_hashtable_create(size_t sizehint) {
    if (sizehint < GRIM_HASHTABLE_MIN_SIZE)
        sizehint = GRIM_HASHTABLE_MIN_SIZE;
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_HASHTABLE_TAG;
    obj->hashnodes = GC_MALLOC(sizehint * sizeof(grim_hashnode *));
    assert(obj->hashnodes);
    obj->hashcap = sizehint;
    obj->hashfill = 0;
    for (size_t i = 0; i < sizehint; i++)
        obj->hashnodes[i] = NULL;
    return (grim_object) obj;
}

size_t grim_hashtable_size(grim_object table) {
    return ((grim_indirect *)table)->hashfill;
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

static void grim_hashtable_grow(grim_indirect *ind) {
    size_t oldsize = ind->hashcap;
    grim_hashnode **oldnodes = ind->hashnodes;
    size_t newsize = (size_t) (oldsize * GRIM_BUFFER_GROWTH_FACTOR);
    grim_hashnode **newnodes = GC_MALLOC(newsize * sizeof(grim_hashnode *));
    for (size_t i = 0; i < oldsize; i++) {
        grim_hashnode *srcnode = oldnodes[i];
        while (srcnode) {
            grim_hashnode **tgtnode = grim_hashtable_node(newnodes, srcnode->key, srcnode->hash, newsize, true);
            (*tgtnode)->value = srcnode->value;
            srcnode = srcnode->next;
        }
    }
    ind->hashcap = newsize;
    ind->hashnodes = newnodes;
}

bool grim_hashtable_has(grim_object table, grim_object key) {
    grim_indirect *ind = (grim_indirect *) table;
    grim_hashnode **node = grim_hashtable_node(ind->hashnodes, key, grim_hash(key, 0), ind->hashcap, false);
    return (*node) ? true : false;
}

grim_object grim_hashtable_get(grim_object table, grim_object key) {
    grim_indirect *ind = (grim_indirect *) table;
    grim_hashnode **node = grim_hashtable_node(ind->hashnodes, key, grim_hash(key, 0), ind->hashcap, false);
    return (*node) ? (*node)->value : grim_undefined;
}

void grim_hashtable_set(grim_object table, grim_object key, grim_object value) {
    grim_indirect *ind = (grim_indirect *) table;
    size_t hash = grim_hash(key, 0);
    grim_hashnode **node = grim_hashtable_node(ind->hashnodes, key, hash, ind->hashcap, true);
    if ((*node)->value == grim_undefined && ind->hashfill > ind->hashcap * GRIM_HASHTABLE_MAX_FILL) {
        grim_hashtable_grow(ind);
        node = grim_hashtable_node(ind->hashnodes, key, hash, ind->hashcap, true);
    }
    if ((*node)->value == grim_undefined)
        ind->hashfill++;
    (*node)->value = value;
}

void grim_hashtable_unset(grim_object table, grim_object key) {
    grim_indirect *ind = (grim_indirect *) table;
    size_t hash = grim_hash(key, 0);
    grim_hashnode **node = grim_hashtable_node(ind->hashnodes, key, hash, ind->hashcap, false);
    if (*node) {
        *node = (*node)->next;
        ind->hashfill--;
    }
}
