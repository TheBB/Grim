#include "grim.h"
#include "test.h"


static MunitResult equality(const MunitParameter params[], void *fixture) {
    grim_object a = grim_intern("dingbob", "UTF-8");
    gta_check_symbol(a, 7, "dingbob");

    grim_object b = grim_intern("dingbob", "UTF-8");
    gta_check_symbol(b, 7, "dingbob");

    gta_check_repr(a, b);

    return MUNIT_OK;
}

static MunitResult read(const MunitParameter params[], void *fixture) {
    grim_object code, a, b;

    code = grim_string_pack("dingbob", "UTF-8", false);
    a = grim_read(code);
    gta_check_symbol(a, 7, "dingbob");

    b = grim_read(code);
    gta_check_symbol(b, 7, "dingbob");

    gta_check_repr(a, b);

    return MUNIT_OK;
}


static MunitTest tests_symbols[] = {
    gta_basic(equality),
    gta_basic(read),
    gta_endtests,
};

MunitSuite suite_symbols = {
    "/symbols",
    tests_symbols,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
