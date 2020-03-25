#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "gmp.h"
#include "uniconv.h"
#include "zhash.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


struct ZHashTable *grim_symbol_table;


grim_tag grim_get_direct_tag(grim_object obj) {
    if ((obj & GRIM_FIXNUM_TAG) != 0)
        return GRIM_FIXNUM_TAG;
    return obj & 0x0f;
}

grim_tag grim_get_indirect_tag(grim_object obj) {
    return *((grim_tag *) obj);
}

grim_type grim_get_type(grim_object obj) {
    switch (grim_get_direct_tag(obj)) {
    case GRIM_FIXNUM_TAG: return GRIM_INTEGER;
    case GRIM_CHARACTER_TAG: return GRIM_CHARACTER;
    case GRIM_SYMBOL_TAG: return GRIM_SYMBOL;
    case GRIM_FALSE_TAG: case GRIM_TRUE_TAG: return GRIM_BOOLEAN;
    case GRIM_NIL_TAG: return GRIM_NIL;
    case GRIM_INDIRECT_TAG:
        switch (grim_get_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG: return GRIM_INTEGER;
        case GRIM_STRING_TAG: return GRIM_STRING;
        case GRIM_VECTOR_TAG: return GRIM_VECTOR;
        case GRIM_CONS_TAG: return GRIM_CONS;
        case GRIM_BUFFER_TAG: return GRIM_BUFFER;
        }
    default: case GRIM_UNDEFINED_TAG: return GRIM_UNDEFINED;
    }
}


static grim_indirect *grim_create_indirect(bool permanent) {
    grim_indirect *retval;
    if (permanent)
        retval = GC_MALLOC_UNCOLLECTABLE(sizeof(grim_indirect));
    else
        retval = GC_MALLOC(sizeof(grim_indirect));
    assert(retval);
    return retval;
}


// Integers
// -----------------------------------------------------------------------------

static void grim_finalize_bigint(void *obj, void *_) {
    (void)_;
    mpz_clear(((grim_indirect *) obj)->bigint);
}

static grim_indirect *grim_create_bigint() {
    grim_indirect *obj = grim_create_indirect(false);
    obj->tag = GRIM_BIGINT_TAG;
    mpz_init(obj->bigint);
    GC_REGISTER_FINALIZER(obj, grim_finalize_bigint, NULL, NULL, NULL);
    return obj;
}

bool grim_can_extract_integer(grim_object obj) {
    if (grim_get_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return true;
    return mpz_fits_slong_p(((grim_indirect *) obj)->bigint);
}

intmax_t grim_extract_integer(grim_object obj) {
    if (grim_get_direct_tag(obj) == GRIM_FIXNUM_TAG) {
        bool signbit = ((intptr_t)obj) < 0;
        intptr_t ret = obj >> 1;
        return signbit ? (INTPTR_MIN | ret) : ret;
    }
    return mpz_get_si(((grim_indirect *) obj)->bigint);
}

grim_object grim_pack_integer(intmax_t num) {
    if (num >= GRIM_FIXNUM_MIN && num <= GRIM_FIXNUM_MAX)
        return (grim_object) ((uintptr_t) num << 1) | GRIM_FIXNUM_TAG;
    grim_indirect *obj = grim_create_bigint();
    mpz_set_si(obj->bigint, num);
    return (grim_object) obj;
}


// Strings
// -----------------------------------------------------------------------------

static void grim_finalize_string(void *obj, void *_) {
    (void)_;
    free(((grim_indirect *) obj)->str);
}

static grim_indirect *grim_create_string() {
    grim_indirect *obj = grim_create_indirect(false);
    obj->tag = GRIM_STRING_TAG;
    obj->str = NULL;
    obj->strlen = 0;
    GC_REGISTER_FINALIZER(obj, grim_finalize_string, NULL, NULL, NULL);
    return obj;
}

grim_object grim_pack_string(const char *input, const char *encoding) {
    grim_indirect *obj = grim_create_string();
    if (!encoding)
        encoding = locale_charset();
    obj->str = u8_conv_from_encoding(encoding, iconveh_error, input, strlen(input) + 1, NULL, NULL, &obj->strlen);
    if (!obj->str)
        return grim_undefined;
    return (grim_object) obj;
}

grim_object grim_pack_string_escape(const char *input, const char *encoding) {
    grim_object str = grim_pack_string(input, encoding);
    grim_unescape_string(str);
    return str;
}

size_t grim_get_strlen(grim_object obj) {
    return ((grim_indirect *) obj)->strlen - 1;
}


// Vectors
// -----------------------------------------------------------------------------

grim_object grim_create_vector(size_t nelems) {
    grim_indirect *obj = grim_create_indirect(false);
    obj->tag = GRIM_VECTOR_TAG;
    assert((obj->vector_data = GC_MALLOC(nelems * sizeof(grim_object))));
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
    ((grim_indirect *) vec)->vector_data[index] = elt;
}

grim_object grim_vector_get(grim_object vec, size_t index) {
    return ((grim_indirect *) vec)->vector_data[index];
}


// Cons cells
// -----------------------------------------------------------------------------

grim_object grim_create_cons(grim_object car, grim_object cdr) {
    grim_indirect *obj = grim_create_indirect(false);
    obj->tag = GRIM_CONS_TAG;
    obj->car = car;
    obj->cdr = cdr;
    return (grim_object) obj;
}

grim_object grim_get_car(grim_object obj) {
    return ((grim_indirect *) obj)->car;
}

grim_object grim_get_cdr(grim_object obj) {
    return ((grim_indirect *) obj)->cdr;
}


// Symbols
// -----------------------------------------------------------------------------

static grim_object grim_create_symbol(uint8_t *name, size_t len) {
    grim_indirect *obj = grim_create_indirect(true);
    obj->symbolname = name;
    obj->symbollen = len;
    return ((grim_object) obj) | GRIM_SYMBOL_TAG;
}

grim_object grim_intern(const char *name, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    size_t u8len;
    uint8_t *u8name = u8_conv_from_encoding(encoding, iconveh_error, name, strlen(name) + 1, NULL, NULL, &u8len);
    grim_object sym = (grim_object) zhash_get(grim_symbol_table, (char *) u8name);
    if (sym != 0) {
        free(u8name);
        return sym;
    }
    sym = grim_create_symbol(u8name, u8len);
    zhash_set(grim_symbol_table, (char *) u8name, (void *) sym);
    return sym;
}

uint8_t *grim_get_symbol_name(grim_object obj) {
    grim_indirect *wrapped = (grim_indirect *) (obj - GRIM_SYMBOL_TAG);
    return wrapped->symbolname;
}


// Characters
// -----------------------------------------------------------------------------

grim_object grim_pack_character(ucs4_t ch) {
    return (((grim_object) ch) << 8) | GRIM_CHARACTER_TAG;
}

grim_object grim_pack_character_name(const char *name, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    size_t u8len;
    uint8_t *str = u8_conv_from_encoding(encoding, iconveh_error, name, strlen(name) + 1, NULL, NULL, &u8len);
    ucs4_t ch = grim_unescape_character(str);
    free(str);
    return grim_pack_character(ch);
}

ucs4_t grim_extract_character(grim_object obj) {
    return (ucs4_t) (obj >> 8);
}


// Buffers
// -----------------------------------------------------------------------------

static void grim_finalize_buffer(void *obj, void *_) {
    (void)_;
    free(((grim_indirect *) obj)->buf);
}

grim_object grim_create_buffer(size_t sizehint) {
    grim_indirect *obj = grim_create_indirect(false);
    obj->tag = GRIM_BUFFER_TAG;
    assert((obj->buf = malloc(sizehint)));
    obj->buflen = 0;
    obj->bufcap = sizehint;
    GC_REGISTER_FINALIZER(obj, grim_finalize_buffer, NULL, NULL, NULL);
    return (grim_object) obj;
}

void grim_dump_buffer(FILE *stream, grim_object obj) {
    grim_indirect *ind = (grim_indirect *) obj;
    fprintf(stream, "%*s", (int) ind->buflen, ind->buf);
}

void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint) {
    grim_indirect *ind = (grim_indirect *) obj;
    size_t required = ind->buflen + sizehint;
    if (ind->bufcap < required) {
        assert((ind->buf = realloc(ind->buf, required)));
        ind->bufcap = required;
    }
}

void grim_buffer_copy(grim_object obj, const char *data, size_t length) {
    grim_buffer_ensure_free_capacity(obj, length);
    grim_indirect *ind = (grim_indirect *) obj;
    memcpy(ind->buf + ind->buflen, data, length);
    ind->buflen += length;
}
