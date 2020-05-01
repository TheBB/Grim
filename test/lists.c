#include "grim.h"
#include "test.h"


static MunitResult read_simple(const MunitParameter params[], void *fixture) {
    grim_object code, list;

    code = grim_string_pack("()", "UTF-8", false);
    list = grim_read(code);
    gta_check_repr(list, grim_nil);

    code = grim_string_pack("(alpha)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_symbol(grim_car(list), 5, "alpha");
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(1)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_fixnum(grim_car(list), 1);
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(-1)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_fixnum(grim_car(list), -1);
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(1.15e1)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_float(grim_car(list), 11.5);
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(1/2)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_is_rational(grim_car(list));
    gta_check_fixnum(grim_rational_num(grim_car(list)), 1);
    gta_check_fixnum(grim_rational_den(grim_car(list)), 2);
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(1+2i)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_is_complex(grim_car(list));
    gta_check_fixnum(grim_complex_real(grim_car(list)), 1);
    gta_check_fixnum(grim_complex_imag(grim_car(list)), 2);
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(\"dingbob\")", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_string(grim_car(list), 7, "dingbob");
    gta_check_repr(grim_cdr(list), grim_nil);

    code = grim_string_pack("(#\\linefeed)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_char(grim_car(list), 10);
    gta_check_repr(grim_cdr(list), grim_nil);

    return MUNIT_OK;
}

static MunitResult read_multi(const MunitParameter params[], void *fixture) {
    grim_object code, list;

    code = grim_string_pack("(a 1 \"delta\" #\\tab 1.15 #f)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_symbol(grim_car(list), 1, "a"); list = grim_cdr(list);
    gta_check_fixnum(grim_car(list), 1); list = grim_cdr(list);
    gta_check_string(grim_car(list), 5, "delta"); list = grim_cdr(list);
    gta_check_char(grim_car(list), 9); list = grim_cdr(list);
    gta_check_float(grim_car(list), 1.15); list = grim_cdr(list);
    gta_check_repr(grim_car(list), grim_false);
    gta_check_repr(grim_cdr(list), grim_nil);

    return MUNIT_OK;
}

static MunitResult read_dotted(const MunitParameter params[], void *fixture) {
    grim_object code, list;

    code = grim_string_pack("(a . b)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_symbol(grim_car(list), 1, "a");
    gta_check_symbol(grim_cdr(list), 1, "b");

    code = grim_string_pack("(a b . c)", "UTF-8", false);
    list = grim_read(code);
    gta_is_cons(list);
    gta_check_symbol(grim_car(list), 1, "a"); list = grim_cdr(list);
    gta_check_symbol(grim_car(list), 1, "b");
    gta_check_symbol(grim_cdr(list), 1, "c");

    return MUNIT_OK;
}


static MunitTest tests_lists[] = {
    gta_test("simple", read_simple),
    gta_test("multi", read_multi),
    gta_test("dotted", read_dotted),
    gta_endtests,
};

MunitSuite suite_lists = {
    "/lists",
    tests_lists,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
