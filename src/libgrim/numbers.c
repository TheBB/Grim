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
    grim_object obj = grim_indirect_create(false);
    I_floating(obj) = num;
    return obj;
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
        return I_floating(num);
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

grim_object grim_bigint_create() {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_BIGINT_TAG;
    mpz_init(I_bigint(obj));
    GC_REGISTER_FINALIZER((void*)obj, grim_bigint_finalize, NULL, NULL, NULL);
    return obj;
}

bool grim_integer_extractable(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return true;
    return mpz_fits_slong_p(I_bigint(obj));
}

intmax_t grim_integer_extract(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG) {
        bool signbit = ((intptr_t)obj) < 0;
        intptr_t ret = obj >> 1;
        return signbit ? (INTPTR_MIN | ret) : ret;
    }
    return mpz_get_si(I_bigint(obj));
}

grim_object grim_integer_pack(intmax_t num) {
    if (num >= GRIM_FIXNUM_MIN && num <= GRIM_FIXNUM_MAX)
        return (grim_object) ((uintptr_t) num << 1) | GRIM_FIXNUM_TAG;
    grim_object obj = grim_bigint_create();
    mpz_set_si(I_bigint(obj), num);
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

    grim_object obj = grim_bigint_create();
    assert(!mpz_set_str(I_bigint(obj), dup, base));
    free(dup);

    return grim_integer_normalize(obj);
}

static void grim_integer_to_mpz(mpz_t tgt, grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        mpz_set_si(tgt, grim_integer_extract(obj));
    else
        mpz_set(tgt, I_bigint(obj));
}

static grim_object grim_mpz_to_integer(mpz_t src) {
    if (mpz_fits_slong_p(src))
        return grim_integer_pack(mpz_get_si(src));
    grim_object obj = grim_bigint_create();
    mpz_set(I_bigint(obj), src);
    return obj;
}


// Rationals
// -----------------------------------------------------------------------------

static void grim_rational_finalize(void *obj, void *_) {
    (void)_;
    mpq_clear(I_rational(obj));
}

static grim_object grim_rational_create() {
    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_RATIONAL_TAG;
    mpq_init(I_rational(obj));
    GC_REGISTER_FINALIZER((void*) obj, grim_rational_finalize, NULL, NULL, NULL);
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

    grim_object obj = grim_rational_create();
    mpq_div(I_rational(obj), num, denom);

    mpz_clear(temp);
    mpq_clear(num);
    mpq_clear(denom);

    if (!mpz_cmp_ui(mpq_denref(I_rational(obj)), 1))
        return grim_mpz_to_integer(mpq_numref(I_rational(obj)));
    return obj;
}

grim_object grim_rational_num(grim_object obj) {
    return grim_mpz_to_integer(mpq_numref(I_rational(obj)));
}

grim_object grim_rational_den(grim_object obj) {
    return grim_mpz_to_integer(mpq_denref(I_rational(obj)));
}


// Complex numbers
// -----------------------------------------------------------------------------

grim_object grim_complex_pack(grim_object real, grim_object imag) {
    if (grim_type(imag) == GRIM_INTEGER &&
        grim_integer_extractable(imag) &&
        grim_integer_extract(imag) == 0)
        return real;

    if (grim_type(imag) == GRIM_FLOAT && I_floating(imag) == 0.0)
        return real;

    if (grim_type(real) == GRIM_FLOAT && grim_type(imag) != GRIM_FLOAT)
        imag = grim_float_pack(grim_to_double(imag));
    else if (grim_type(imag) == GRIM_FLOAT && grim_type(real) != GRIM_FLOAT)
        real = grim_float_pack(grim_to_double(real));

    grim_object obj = grim_indirect_create(false);
    I_tag(obj) = GRIM_COMPLEX_TAG;
    I_real(obj) = real;
    I_imag(obj) = imag;
    return obj;
}


// Miscellaneous
// -----------------------------------------------------------------------------

bool grim_nonnegative(grim_object obj) {
    grim_type_t type = grim_type(obj);

    if (type == GRIM_INTEGER) {
        if (grim_integer_extractable(obj))
            return grim_integer_extract(obj) >= 0;
        return mpz_sgn(I_bigint(obj)) >= 0;
    }

    if (type == GRIM_RATIONAL)
        return mpq_sgn(I_rational(obj)) >= 0;

    if (type == GRIM_FLOAT)
        return I_floating(obj) >= 0.0;

    assert(false);
}

grim_object grim_negate_i(grim_object obj) {
    if (grim_direct_tag(obj) == GRIM_FIXNUM_TAG)
        return grim_integer_pack(-grim_integer_extract(obj));

    grim_type_t type = grim_type(obj);
    if (type == GRIM_FLOAT)
        I_floating(obj) = -I_floating(obj);
    else if (type == GRIM_INTEGER)
        mpz_neg(I_bigint(obj), I_bigint(obj));
    else if (type == GRIM_RATIONAL)
        mpq_neg(I_rational(obj), I_rational(obj));
    else if (type == GRIM_COMPLEX) {
        I_real(obj) = grim_negate_i(I_real(obj));
        I_imag(obj) = grim_negate_i(I_imag(obj));
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
        return grim_is_exact(I_real(num)) && grim_is_exact(I_imag(num));
    assert(false);
}


grim_object grim_add(grim_object a, grim_object b) {
    grim_type_t ta = grim_type(a), tb = grim_type(b);
    switch (ta) {
    case GRIM_COMPLEX: switch(tb) {
        case GRIM_COMPLEX:
            return grim_complex_pack(grim_add(I_real(a), I_real(b)), grim_add(I_imag(a), I_imag(b)));
        case GRIM_FLOAT: case GRIM_RATIONAL: case GRIM_INTEGER:
            return grim_complex_pack(grim_add(I_real(a), b), I_imag(a));
        default:
            return grim_undefined;
        }
    case GRIM_FLOAT:
        switch (tb) {
        case GRIM_COMPLEX:
            return grim_complex_pack(grim_add(a, I_real(b)), I_imag(b));
        case GRIM_FLOAT:
            return grim_float_pack(I_floating(a) + I_floating(b));
        case GRIM_RATIONAL: case GRIM_INTEGER:
            return grim_float_pack(I_floating(a) + grim_to_double(b));
        default:
            return grim_undefined;
        }
    case GRIM_RATIONAL:
        switch (tb) {
        case GRIM_COMPLEX:
            return grim_complex_pack(grim_add(a, I_real(b)), I_imag(b));
        case GRIM_FLOAT:
            return grim_float_pack(grim_to_double(a) + I_floating(b));
        case GRIM_RATIONAL: {
            // TODO: grim_rational_normalize
            grim_object retval = grim_rational_create();
            mpq_add(I_rational(retval), I_rational(a), I_rational(b));
            return retval;
        }
        case GRIM_INTEGER: {
            grim_object retval = grim_rational_create();
            if (grim_integer_extractable(a))
                mpq_set_si(I_rational(retval), grim_integer_extract(a), 1);
            else
                mpq_set_z(I_rational(retval), I_bigint(a));
            mpq_add(I_rational(retval), I_rational(retval), I_rational(b));
            return retval;
        }
        default:
            return grim_undefined;
        }
    case GRIM_INTEGER:
        switch (tb) {
        case GRIM_COMPLEX:
            return grim_complex_pack(grim_add(a, I_real(b)), I_imag(b));
        case GRIM_FLOAT:
            return grim_float_pack(grim_to_double(a) + I_floating(b));
        case GRIM_RATIONAL: {
            grim_object retval = grim_rational_create();
            if (grim_integer_extractable(b))
                mpq_set_si(I_rational(retval), grim_integer_extract(b), 1);
            else
                mpq_set_z(I_rational(retval), I_bigint(b));
            mpq_add(I_rational(retval), I_rational(retval), I_rational(a));
            return retval;
        }
        case GRIM_INTEGER: {
            bool exa = grim_direct_tag(a) == GRIM_FIXNUM_TAG;
            bool exb = grim_direct_tag(b) == GRIM_FIXNUM_TAG;
            if (exa && exb)
                return grim_integer_pack(grim_integer_extract(a) + grim_integer_extract(b));
            else {
                grim_object retval = grim_bigint_create();

                mpz_t temp;
                mpz_init(temp);
                grim_integer_to_mpz(temp, a);

                if (grim_integer_extractable(b))
                    mpz_set_si(I_bigint(retval), grim_integer_extract(b));
                else
                    grim_integer_to_mpz(I_bigint(retval), b);

                mpz_add(I_bigint(retval), I_bigint(retval), temp);
                mpz_clear(temp);
                return grim_integer_normalize(retval);
            }
        }
        default:
            return grim_undefined;
        }
    default:
        return grim_undefined;
    }
}
