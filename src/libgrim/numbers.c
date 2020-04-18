#include <assert.h>
#include <math.h>
#include <stdio.h>
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

grim_object grim_float_read(const char *str) {
    // Scanf understands a subset of our syntax:
    // Normalize zero digits and ignorable characters
    char *dup = strdup(str), *tgt = dup;
    for (char *src = dup; *src; src++) {
        if (*src == '_')
            continue;
        if (*src == '#')
            *tgt = '0';
        else
            *tgt = *src;
        tgt++;
    }
    *tgt = 0;

    double value;
    sscanf(dup, "%lf", &value);
    free(dup);

    return grim_float_pack(value);
}

double grim_to_double(grim_object num) {
    if (grim_direct_tag(num) == GRIM_FIXNUM_TAG)
        return grim_integer_extract(num);
    else if (grim_type(num) == GRIM_FLOAT)
        return grim_float_extract(num);
    else if (grim_type(num) == GRIM_INTEGER)
        return mpz_get_d(I(num)->bigint);
    else if (grim_type(num) == GRIM_RATIONAL)
        return mpq_get_d(I(num)->rational);
    assert(false);
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

    if (grim_type(real) == GRIM_FLOAT && grim_type(imag) != GRIM_FLOAT)
        imag = grim_float_pack(grim_to_double(imag));
    else if (grim_type(imag) == GRIM_FLOAT && grim_type(real) != GRIM_FLOAT)
        real = grim_float_pack(grim_to_double(real));

    grim_indirect *obj = grim_indirect_create(false);
    obj->tag = GRIM_COMPLEX_TAG;
    obj->real = real;
    obj->imag = imag;
    return (grim_object) obj;
}

grim_object grim_complex_real(grim_object num) {
    return I(num)->real;
}

grim_object grim_complex_imag(grim_object num) {
    return I(num)->imag;
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

grim_object grim_scinot_pack(grim_object scale, int base, intmax_t exp, bool exact) {
    assert(grim_type(scale) == GRIM_INTEGER);

    if (!exact) {
        double s = grim_to_double(scale);
        return grim_float_pack(s * pow(base, exp));
    }

    if (exp == 0)
        return scale;

    mpz_t temp;
    mpz_init_set_si(temp, base);
    grim_object retval;

    if (exp < 0) {
        mpz_pow_ui(temp, temp, -exp);
        retval = grim_rational_pack(scale, grim_mpz_to_integer(temp));
    }
    else {
        mpz_pow_ui(temp, temp, exp);
        if (grim_integer_extractable(scale))
            mpz_mul_si(temp, temp, grim_integer_extract(scale));
        else {
            mpz_t s;
            mpz_init(s);
            grim_integer_to_mpz(s, scale);
            mpz_mul(temp, temp, s);
            mpz_clear(s);
        }
        retval = grim_mpz_to_integer(temp);
    }

    mpz_clear(temp);
    return retval;
}

bool grim_is_exact(grim_object num) {
    grim_type_t type = grim_type(num);
    if (type == GRIM_INTEGER || type == GRIM_RATIONAL)
        return true;
    if (type == GRIM_FLOAT)
        return false;
    if (type == GRIM_COMPLEX)
        return grim_is_exact(I(num)->real) && grim_is_exact(I(num)->imag);
    assert(false);
}
