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
    gta_check_fixnum(grim_rational_num(rat), 1);
    gta_check_fixnum(grim_rational_den(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(1), grim_integer_pack(-2));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_num(rat), -1);
    gta_check_fixnum(grim_rational_den(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(-1), grim_integer_pack(2));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_num(rat), -1);
    gta_check_fixnum(grim_rational_den(rat), 2);

    rat = grim_rational_pack(grim_integer_pack(2), grim_integer_pack(4));
    gta_is_rational(rat);
    gta_check_fixnum(grim_rational_num(rat), 1);
    gta_check_fixnum(grim_rational_den(rat), 2);

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
    gta_check_fixnum(I(cpl)->real, 1);
    gta_check_fixnum(I(cpl)->imag, 1);

    cpl = grim_complex_pack(grim_float_pack(3.1415), grim_float_pack(0.0));
    gta_check_float(cpl, 3.1415);

    cpl = grim_complex_pack(grim_float_pack(0.0), grim_float_pack(-1.0));
    gta_is_complex(cpl);
    gta_check_float(I(cpl)->real, 0.0);
    gta_check_float(I(cpl)->imag, -1.0);

    return MUNIT_OK;
}


static MunitResult readint(const MunitParameter params[], void *fixture) {
    grim_init();
    grim_object num;

    num = grim_integer_read("1001#", 2);
    gta_check_fixnum(num, 18);

    num = grim_integer_read("2.01", 8);
    gta_check_fixnum(num, 129);

    num = grim_integer_read("27_91#", 10);
    gta_check_fixnum(num, 27910);

    num = grim_integer_read("7..1_6", 16);
    gta_check_fixnum(num, 1814);

    num = grim_integer_read("999999999999999999999999999999999", 10);
    gta_check_bigint(num, "999999999999999999999999999999999");

    num = grim_integer_read("99.99999._999999..999999999999__999#####", 10);
    gta_check_bigint(num, "999999999999999999999999999900000");

    return MUNIT_OK;
}


static MunitResult readfloat(const MunitParameter params[], void *fixture) {
    grim_init();
    grim_object num;

    num = grim_float_read("3.1415");
    gta_check_float(num, 3.1415);

    num = grim_float_read("3.1_4##");
    gta_check_float(num, 3.14);

    num = grim_float_read("3_14##e-4");
    gta_check_float(num, 3.14);

    return MUNIT_OK;
}

MunitTest tests_numbers[] = {
    {"/floats", floats, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/bigints", bigints, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/rationals", rationals, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/complex", complex, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/readint", readint, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/readfloat", readfloat, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
