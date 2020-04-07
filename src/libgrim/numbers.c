#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "gmp.h"

#include "grim.h"
#include "internal.h"


// Constants
// -----------------------------------------------------------------------------

#define IGNORE -1

static int DIGIT_VALUE[] = {
    // The period is useful for parsing exact rationals in decimal notation
    ['.'] = IGNORE,
    ['_'] = IGNORE,

    ['#'] = 0,
    ['0'] = 0,
    ['1'] = 1,
    ['2'] = 2,
    ['3'] = 3,
    ['4'] = 4,
    ['5'] = 5,
    ['6'] = 6,
    ['7'] = 7,
    ['8'] = 8,
    ['9'] = 9,
    ['a'] = 10,
    ['b'] = 11,
    ['c'] = 12,
    ['d'] = 13,
    ['e'] = 14,
    ['f'] = 15,
};

// The largest number of digits in an unsigned integer for which we
// can be sure that it fits in a fixnum.  This is computed when
// initializing at runtime.
size_t grim_fixnum_max_ndigits[] = {
    [2] = 0,
    [8] = 0,
    [10] = 0,
    [16] = 0,
};


// Floats
// -----------------------------------------------------------------------------

grim_object grim_float_pack(double num) {
    grim_indirect *obj = grim_indirect_create(false);
    obj->floating = num;
    return (grim_object) obj;
}

double grim_float_extract(grim_object obj) {
    return I(obj)->floating;
}


// Integers
// -----------------------------------------------------------------------------

static void grim_bigint_finalize(void *obj, void *_) {
    (void)_;
    mpz_clear(I(obj)->bigint);
}

grim_indirect *grim_bigint_create() {
    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_BIGINT_TAG;
    mpz_init(obj->bigint);
    GC_REGISTER_FINALIZER(obj, grim_bigint_finalize, NULL, NULL, NULL);
    return obj;
}

bool grim_integer_extractable(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return true;
    return mpz_fits_slong_p(I(obj)->bigint);
}

intmax_t grim_integer_extract(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG) {
        bool signbit = ((intptr_t)obj) < 0;
        intptr_t ret = obj >> 1;
        return signbit ? (INTPTR_MIN | ret) : ret;
    }
    return mpz_get_si(I(obj)->bigint);
}

grim_object grim_integer_pack(intmax_t num) {
    if (num >= GRIM_FIXNUM_MIN && num <= GRIM_FIXNUM_MAX)
        return (grim_object) ((uintptr_t) num << 1) | GRIM_FIXNUM_TAG;
    grim_indirect *obj = grim_bigint_create();
    mpz_set_si(obj->bigint, num);
    return (grim_object) obj;
}

static grim_object grim_integer_normalize(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return obj;
    if (grim_integer_extractable(obj))
        return grim_integer_pack(grim_integer_extract(obj));
    return obj;
}

grim_object grim_integer_read(const char *str, int base) {
    size_t len = strlen(str);

    if (len <= grim_fixnum_max_ndigits[base]) {
        intmax_t value = 0;
        for (; *str; str++) {
            int digit = DIGIT_VALUE[(int) *str];
            if (digit == IGNORE)
                continue;
            value = base * value + digit;
        }
        return grim_integer_pack(value);
    }

    // GMP understands a subset of our syntax:
    // Normalize zero digits and ignorable characters
    char *dup = strdup(str), *tgt = dup;
    for (char *src = dup; *src; src++) {
        int digit = DIGIT_VALUE[(int) *src];
        if (digit == IGNORE)
            continue;
        if (digit == 0)
            *tgt = '0';
        else
            *tgt = *src;
        tgt++;
    }
    *tgt = 0;

    grim_indirect *obj = grim_bigint_create();
    assert(!mpz_set_str(obj->bigint, dup, base));
    free(dup);

    return grim_integer_normalize((grim_object) obj);
}

static void grim_integer_to_mpz(mpz_t tgt, grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        mpz_set_si(tgt, grim_integer_extract(obj));
    else
        mpz_set(tgt, I(obj)->bigint);
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
    mpq_clear(I(obj)->rational);
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

    if (!mpz_cmp_ui(mpq_denref(obj->rational), 1))
        return grim_mpz_to_integer(mpq_numref(obj->rational));
    return (grim_object) obj;
}

grim_object grim_rational_num(grim_object obj) {
    return grim_mpz_to_integer(mpq_numref(I(obj)->rational));
}

grim_object grim_rational_den(grim_object obj) {
    return grim_mpz_to_integer(mpq_denref(I(obj)->rational));
}


// Complex numbers
// -----------------------------------------------------------------------------

grim_object grim_complex_pack(grim_object real, grim_object imag) {
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


// Miscellaneous
// -----------------------------------------------------------------------------

bool grim_nonnegative(grim_object obj) {
    grim_type_t type = grim_type(obj);

    if (type == GRIM_INTEGER) {
        if (grim_integer_extractable(obj))
            return grim_integer_extract(obj) >= 0;
        return mpz_sgn(I(obj)->bigint) >= 0;
    }

    if (type == GRIM_RATIONAL)
        return mpq_sgn(I(obj)->rational) >= 0;

    if (type == GRIM_FLOAT)
        return grim_float_extract(obj) >= 0.0;

    assert(false);
}

grim_object grim_negate_i(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return grim_integer_pack(-grim_integer_extract(obj));

    grim_type_t type = grim_type(obj);
    if (type == GRIM_FLOAT)
        I(obj)->floating = -I(obj)->floating;
    else if (type == GRIM_INTEGER)
        mpz_neg(I(obj)->bigint, I(obj)->bigint);
    else if (type == GRIM_RATIONAL)
        mpq_neg(I(obj)->rational, I(obj)->rational);
    else if (type == GRIM_COMPLEX) {
        I(obj)->real = grim_negate_i(I(obj)->real);
        I(obj)->imag = grim_negate_i(I(obj)->imag);
    }

    return obj;
}

grim_object grim_asfloat(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return grim_float_pack((double) grim_integer_extract(obj));

    grim_type_t type = grim_type(obj);

    if (type == GRIM_FLOAT)
        return obj;

    if (type == GRIM_INTEGER)
        return grim_float_pack((double) mpz_get_d(I(obj)->bigint));

    if (type == GRIM_RATIONAL)
        return grim_float_pack((double) mpq_get_d(I(obj)->rational));

    assert(false);
}
