#include "grim.h"
#include "internal.h"
#include "test.h"


static MunitResult simple(const MunitParameter params[], void *fixture) {
    const char code[] = { GRIM_BC_LOAD_ARG, 0, GRIM_BC_RETURN };
    grim_object bytecode = grim_buffer_create(0);
    grim_buffer_copy(bytecode, code, 3);

    grim_object func = grim_lfunc_create(bytecode, grim_vector_create(0), 0, 1, false);
    grim_object result = grim_call_1(func, grim_integer_pack(1));
    gta_check_fixnum(result, 1);

    return MUNIT_OK;
}


static MunitTest tests_bytecode[] = {
    gta_basic(simple),
    gta_endtests,
};

MunitSuite suite_bytecode = {
    "/bytecode",
    tests_bytecode,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
