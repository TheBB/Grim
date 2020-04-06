#include "grim.h"
#include "test.h"


static MunitResult floats(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_float_pack(0.0);
    gta_check_float(num, 0.0);

    num = grim_float_pack(-1.0);
    gta_check_float(num, -1.0);

    num = grim_float_pack(3.1415);
    gta_check_float(num, 3.1415);

    return MUNIT_OK;
}


static MunitResult bigints(const MunitParameter params[], void *fixture) {
    grim_object max = grim_integer_pack(4611686018427387904);
    gta_check_bigint(max, "4611686018427387904");

    grim_object min = grim_integer_pack(-4611686018427387905);
    gta_check_bigint(min, "-4611686018427387905");

    return MUNIT_OK;
}


static MunitResult rationals(const MunitParameter params[], void *fixture) {
    grim_object rat;

    rat = grim_rational_pack(grim_integer_pack(2), grim_integer_pack(1));
    gta_check_fixnum(rat, 2);

    rat = grim_rational_pack(grim_integer_pack(1), grim_integer_pack(2));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_numerator(rat), 1);
    gta_check_fixnum(grim_rational_denominator(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(1), grim_integer_pack(-2));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_numerator(rat), -1);
    gta_check_fixnum(grim_rational_denominator(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(-1), grim_integer_pack(2));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_numerator(rat), -1);
    gta_check_fixnum(grim_rational_denominator(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(2), grim_integer_pack(4));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_numerator(rat), 1);
    gta_check_fixnum(grim_rational_denominator(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(4), grim_integer_pack(2));
    gta_check_fixnum(rat, 2);

    return MUNIT_OK;
}


static MunitResult complex(const MunitParameter params[], void *fixture) {
    grim_object cpl;

    cpl = grim_complex_pack(grim_integer_pack(1), grim_integer_pack(0));
    gta_check_fixnum(cpl, 1);

    cpl = grim_complex_pack(grim_integer_pack(1), grim_integer_pack(1));
    gta_is_complex(cpl);
    gta_check_fixnum(grim_complex_real(cpl), 1);
    gta_check_fixnum(grim_complex_imag(cpl), 1);

    cpl = grim_complex_pack(grim_float_pack(3.1415), grim_float_pack(0.0));
    gta_check_float(cpl, 3.1415);

    cpl = grim_complex_pack(grim_float_pack(0.0), grim_float_pack(-1.0));
    gta_is_complex(cpl);
    gta_check_float(grim_complex_real(cpl), 0.0);
    gta_check_float(grim_complex_imag(cpl), -1.0);

    return MUNIT_OK;
}


MunitTest tests_numbers[] = {
    {"/floats", floats, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/bigints", bigints, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/rationals", rationals, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/complex", complex, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
