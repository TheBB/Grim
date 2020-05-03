#include "grim.h"
#include "internal.h"
#include "test.h"


static grim_object builtin(const char *name) {
    return grim_module_get(grim_builtin_module, grim_intern(name, NULL));
}


static MunitResult add(const MunitParameter params[], void *fixture) {
    grim_object a, func = builtin("+");

    a = grim_call_0(func);
    gta_check_fixnum(a, 0);

    a = grim_call_1(func, grim_integer_pack(0));
    gta_check_fixnum(a, 0);

    a = grim_call_1(func, grim_float_pack(15.1));
    gta_check_float(a, 15.1);

    a = grim_call_2(func, grim_integer_pack(1), grim_integer_pack(-1));
    gta_check_fixnum(a, 0);

    a = grim_call_2(func, grim_integer_pack(GRIM_FIXNUM_MAX), grim_integer_pack(GRIM_FIXNUM_MIN));
    gta_check_fixnum(a, -1);

    a = grim_call_2(func, grim_integer_pack(GRIM_FIXNUM_MAX), grim_integer_pack(GRIM_FIXNUM_MAX));
    gta_is_bigint(a);
    a = grim_call_2(func, a, grim_integer_pack(GRIM_FIXNUM_MIN));
    gta_check_fixnum(a, GRIM_FIXNUM_MAX - 1);

    a = grim_call_2(func, grim_integer_pack(GRIM_FIXNUM_MIN), grim_integer_pack(GRIM_FIXNUM_MIN));
    gta_is_bigint(a);
    a = grim_call_2(func, a, grim_integer_pack(GRIM_FIXNUM_MAX));
    gta_is_bigint(a);
    a = grim_call_2(func, a, grim_integer_pack(GRIM_FIXNUM_MAX));
    gta_check_fixnum(a, -2);

    return MUNIT_OK;
}


MunitTest tests_builtins[] = {
    gta_basic(add),
    gta_endtests,
};

MunitSuite suite_builtins = {
    "/builtins",
    tests_builtins,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
