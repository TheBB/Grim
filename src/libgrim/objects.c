#include <assert.h>
#include <stdbool.h>

#include "gc.h"
#include "gmp.h"

#include "grim.h"
#include "internal.h"


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
    case GRIM_INDIRECT_TAG:
        switch (grim_get_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG: return GRIM_INTEGER;
        }
    default: case GRIM_UNDEFINED_TAG: return GRIM_UNDEFINED;
    }
}


grim_indirect *grim_create_indirect() {
    return (grim_indirect *) GC_MALLOC(sizeof(grim_indirect));
}

void grim_finalize_bigint(void *obj, void *_) {
    (void)_;
    mpz_clear(((grim_indirect *) obj)->bigint);
}

grim_indirect *grim_create_bigint() {
    grim_indirect *obj = grim_create_indirect();
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
