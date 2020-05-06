#include "grim.h"
#include "internal.h"
#include "test.h"


static MunitResult identity(const MunitParameter params[], void *fixture) {
    const char code[] = { GRIM_BC_LOAD_ARG, 0, GRIM_BC_RETURN };
    grim_object bytecode = grim_buffer_create(0);
    grim_buffer_copy(bytecode, code, 3);

    grim_object func = grim_lfunc_create(bytecode, grim_vector_create(0), 0, 1, false);
    grim_object result = grim_call_1(func, grim_integer_pack(1));
    gta_check_fixnum(result, 1);

    return MUNIT_OK;
}


static MunitResult add(const MunitParameter params[], void *fixture){
    const char code[] = {
        GRIM_BC_LOAD_ARG, 0,       // Push first argument
        GRIM_BC_LOAD_ARG, 1,       // Push second argument
        GRIM_BC_LOAD_REF_CELL, 0,  // Push addition function
        GRIM_BC_CALL, 2,           // Call
        GRIM_BC_RETURN             // Return
    };
    grim_object bytecode = grim_buffer_create(0);
    grim_buffer_copy(bytecode, code, 9);

    grim_object refs = grim_vector_create(1);
    I_vectorelt(refs, 0) = grim_module_cell(grim_builtin_module, grim_intern("+", NULL), true);

    grim_object func = grim_lfunc_create(bytecode, refs, 0, 2, false);
    grim_object result = grim_call_2(func, grim_integer_pack(1), grim_integer_pack(1));
    gta_check_fixnum(result, 2);

    return MUNIT_OK;
}


static MunitResult constant(const MunitParameter params[], void *fixture){
    const char code[] = {
        GRIM_BC_LOAD_ARG, 0,       // Push first argument
        GRIM_BC_LOAD_REF, 0,       // Push constant
        GRIM_BC_LOAD_REF_CELL, 1,  // Push addition function
        GRIM_BC_CALL, 2,           // Call
        GRIM_BC_RETURN             // Return
    };
    grim_object bytecode = grim_buffer_create(0);
    grim_buffer_copy(bytecode, code, 9);

    grim_object refs = grim_vector_create(2);
    I_vectorelt(refs, 0) = grim_integer_pack(5);
    I_vectorelt(refs, 1) = grim_module_cell(grim_builtin_module, grim_intern("+", NULL), true);

    grim_object func = grim_lfunc_create(bytecode, refs, 0, 1, false);
    grim_object result = grim_call_1(func, grim_integer_pack(1));
    gta_check_fixnum(result, 6);

    return MUNIT_OK;
}


static MunitResult local(const MunitParameter params[], void *fixture){
    const char code[] = {
        GRIM_BC_LOAD_ARG, 0,       // Push first argument
        GRIM_BC_STORE_LOCAL, 0,    // Store as local
        GRIM_BC_LOAD_LOCAL, 0,     // Load local again
        GRIM_BC_RETURN             // Return
    };
    grim_object bytecode = grim_buffer_create(0);
    grim_buffer_copy(bytecode, code, 7);

    grim_object func = grim_lfunc_create(bytecode, grim_vector_create(0), 1, 1, false);
    grim_object result = grim_call_1(func, grim_integer_pack(15));
    gta_check_fixnum(result, 15);

    return MUNIT_OK;
}


static MunitTest tests_bytecode[] = {
    gta_basic(identity),
    gta_basic(add),
    gta_basic(constant),
    gta_basic(local),
    gta_endtests,
};

MunitSuite suite_bytecode = {
    "/bytecode",
    tests_bytecode,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
