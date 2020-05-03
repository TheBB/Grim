#include "grim.h"
#include "test.h"


static MunitResult identity(const MunitParameter params[], void *fixture) {
    grim_object func = grim_module_get(grim_builtin_module, grim_intern("identity", NULL));
    grim_object res;

    res = grim_call_1(func, grim_nil);
    gta_check_repr(res, grim_nil);

    return MUNIT_OK;
}


MunitTest tests_builtins[] = {
    gta_basic(identity),
    gta_endtests,
};

MunitSuite suite_builtins = {
    "/builtins",
    tests_builtins,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
