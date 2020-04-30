#include <math.h>

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

static MunitResult rawint(const MunitParameter params[], void *fixture) {
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

static MunitResult rawfloat(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_float_read("3.1415");
    gta_check_float(num, 3.1415);

    num = grim_float_read("3.1_4##");
    gta_check_float(num, 3.14);

    num = grim_float_read("3_14##e-4");
    gta_check_float(num, 3.14);

    return MUNIT_OK;
}

static MunitResult parse_floats(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_read(grim_string_pack("15.0", NULL, false));
    gta_check_float(num, 15.0);

    num = grim_read(grim_string_pack("+3.1415", NULL, false));
    gta_check_float(num, 3.1415);

    num = grim_read(grim_string_pack("-2.71828", NULL, false));
    gta_check_float(num, -2.71828);

    num = grim_read(grim_string_pack("15.123e-3", NULL, false));
    gta_check_float(num, 0.015123);

    num = grim_read(grim_string_pack("15.123e-2", NULL, false));
    gta_check_float(num, 0.15123);

    num = grim_read(grim_string_pack("15.123e-1", NULL, false));
    gta_check_float(num, 1.5123);

    num = grim_read(grim_string_pack("15.123e+0", NULL, false));
    gta_check_float(num, 15.123);

    num = grim_read(grim_string_pack("-15.123e1", NULL, false));
    gta_check_float(num, -151.23);

    num = grim_read(grim_string_pack("+15.123e2", NULL, false));
    gta_check_float(num, 1512.3);

    num = grim_read(grim_string_pack("15.123e3", NULL, false));
    gta_check_float(num, 15123.0);

    num = grim_read(grim_string_pack("1e3", NULL, false));
    gta_check_float(num, 1000.0);

    num = grim_read(grim_string_pack("15.23##", NULL, false));
    gta_check_float(num, 15.23);

    num = grim_read(grim_string_pack("1#.####", NULL, false));
    gta_check_float(num, 10.0);

    num = grim_read(grim_string_pack("1_000_000.53", NULL, false));
    gta_check_float(num, 1000000.53);

    num = grim_read(grim_string_pack(".129", NULL, false));
    gta_check_float(num, 0.129);

    num = grim_read(grim_string_pack("129.", NULL, false));
    gta_check_float(num, 129.0);

    return MUNIT_OK;
}

static MunitResult parse_ints(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_read(grim_string_pack("0", NULL, false));
    gta_check_fixnum(num, 0);

    num = grim_read(grim_string_pack("-1", NULL, false));
    gta_check_fixnum(num, -1);

    num = grim_read(grim_string_pack("51", NULL, false));
    gta_check_fixnum(num, 51);

    num = grim_read(grim_string_pack("+9", NULL, false));
    gta_check_fixnum(num, 9);

    num = grim_read(grim_string_pack("1_000", NULL, false));
    gta_check_fixnum(num, 1000);

    num = grim_read(grim_string_pack("1_###", NULL, false));
    gta_check_fixnum(num, 1000);

    num = grim_read(grim_string_pack("999999999999999999999999999999999999999", NULL, false));
    gta_check_bigint(num, "999999999999999999999999999999999999999");

    return MUNIT_OK;
}

static MunitResult parse_exact(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_read(grim_string_pack("#e153", NULL, false));
    gta_check_fixnum(num, 153);

    num = grim_read(grim_string_pack("#e153.1", NULL, false));
    gta_is_rational(num);
    gta_check_fixnum(grim_rational_num(num), 1531);
    gta_check_fixnum(grim_rational_den(num), 10);

    num = grim_read(grim_string_pack("#e153e-1", NULL, false));
    gta_is_rational(num);
    gta_check_fixnum(grim_rational_num(num), 153);
    gta_check_fixnum(grim_rational_den(num), 10);

    num = grim_read(grim_string_pack("153/10", NULL, false));
    gta_is_rational(num);
    gta_check_fixnum(grim_rational_num(num), 153);
    gta_check_fixnum(grim_rational_den(num), 10);

    num = grim_read(grim_string_pack("#e153.12e1", NULL, false));
    gta_is_rational(num);
    gta_check_fixnum(grim_rational_num(num), 7656);
    gta_check_fixnum(grim_rational_den(num), 5);

    return MUNIT_OK;
}

static MunitResult parse_inexact(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_read(grim_string_pack("#i12", NULL, false));
    gta_check_float(num, 12.0);

    num = grim_read(grim_string_pack("#i2/3", NULL, false));
    gta_check_float_approx(num, 2.0/3.0, 16);

    return MUNIT_OK;
}

static MunitResult parse_complex(const MunitParameter params[], void *fixture) {
    grim_object num;

    num = grim_read(grim_string_pack("+i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 0);
    gta_check_fixnum(grim_complex_imag(num), 1);

    num = grim_read(grim_string_pack("-i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 0);
    gta_check_fixnum(grim_complex_imag(num), -1);

    num = grim_read(grim_string_pack("1+i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 1);
    gta_check_fixnum(grim_complex_imag(num), 1);

    num = grim_read(grim_string_pack("243892304723894732947328914348329794+i", NULL, false));
    gta_is_complex(num);
    gta_check_bigint(grim_complex_real(num), "243892304723894732947328914348329794");
    gta_check_fixnum(grim_complex_imag(num), 1);

    num = grim_read(grim_string_pack("1+238942394871249810234783294732849324723i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 1);
    gta_check_bigint(grim_complex_imag(num), "238942394871249810234783294732849324723");

    num = grim_read(grim_string_pack("1-1.0i", NULL, false));
    gta_is_complex(num);
    gta_check_float(grim_complex_real(num), 1.0);
    gta_check_float(grim_complex_imag(num), -1.0);

    num = grim_read(grim_string_pack("1.0-1/2i", NULL, false));
    gta_is_complex(num);
    gta_check_float(grim_complex_real(num), 1.0);
    gta_check_float(grim_complex_imag(num), -0.5);

    num = grim_read(grim_string_pack("1-1/2i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 1.0);
    gta_is_rational(grim_complex_imag(num));
    gta_check_fixnum(grim_rational_num(grim_complex_imag(num)), -1);
    gta_check_fixnum(grim_rational_den(grim_complex_imag(num)), 2);

    num = grim_read(grim_string_pack("#e1.0+1/2i", NULL, false));
    gta_is_complex(num);
    gta_check_fixnum(grim_complex_real(num), 1);
    gta_is_rational(grim_complex_imag(num));
    gta_check_fixnum(grim_rational_num(grim_complex_imag(num)), 1);
    gta_check_fixnum(grim_rational_den(grim_complex_imag(num)), 2);

    num = grim_read(grim_string_pack("#i2/5-2i", NULL, false));
    gta_is_complex(num);
    gta_check_float_approx(grim_complex_real(num), 0.4, 16);
    gta_check_float(grim_complex_imag(num), -2.0);

    num = grim_read(grim_string_pack("2@45", NULL, false));
    gta_is_complex(num);
    gta_check_float_approx(grim_complex_real(num), sqrt(2.0), 15);
    gta_check_float_approx(grim_complex_imag(num), sqrt(2.0), 15);

    return MUNIT_OK;
}

static MunitTest tests_numbers[] = {
    gta_basic(floats),
    gta_basic(bigints),
    gta_basic(rationals),
    gta_basic(complex),
    gta_endtests,
};

static MunitTest tests_read[] = {
    gta_basic(rawint),
    gta_basic(rawfloat),
    gta_test("floats", parse_floats),
    gta_test("ints", parse_ints),
    gta_test("exact", parse_exact),
    gta_test("complex", parse_complex),
    gta_endtests,
};

static MunitSuite subsuites[] = {
    {"/read", tests_read, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    gta_endsuite,
};

MunitSuite suite_numbers = {
    "/numbers",
    tests_numbers,
    subsuites,
    1, MUNIT_SUITE_OPTION_NONE,
};
