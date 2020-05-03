#include "grim.h"
#include "test.h"


static MunitResult read_simple(const MunitParameter params[], void *fixture) {
    grim_object code, vec;

    code = grim_string_pack("#()", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 0);

    code = grim_string_pack("#(alpha)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_symbol(I_vectorelt(vec, 0), 5, "alpha");

    code = grim_string_pack("#(1)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_fixnum(I_vectorelt(vec, 0), 1);

    code = grim_string_pack("#(-1)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_fixnum(I_vectorelt(vec, 0), -1);

    code = grim_string_pack("#(1.15e1)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_float(I_vectorelt(vec, 0), 11.5);

    code = grim_string_pack("#(1/2)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_is_rational(I_vectorelt(vec, 0));
    gta_check_fixnum(grim_rational_num(I_vectorelt(vec, 0)), 1);
    gta_check_fixnum(grim_rational_den(I_vectorelt(vec, 0)), 2);

    code = grim_string_pack("#(1+2i)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_is_complex(I_vectorelt(vec, 0));
    gta_check_fixnum(I_real(I_vectorelt(vec, 0)), 1);
    gta_check_fixnum(I_imag(I_vectorelt(vec, 0)), 2);

    code = grim_string_pack("#(\"dingbob\")", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_string(I_vectorelt(vec, 0), 7, "dingbob");

    code = grim_string_pack("#(#\\linefeed)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 1);
    gta_check_char(I_vectorelt(vec, 0), 10);

    return MUNIT_OK;
}

static MunitResult read_multi(const MunitParameter params[], void *fixture) {
    grim_object code, vec;

    code = grim_string_pack("#(a 1 \"delta\" #\\tab 1.15 #f)", "UTF-8", false);
    vec = grim_read(code);
    gta_check_vector(vec, 6);
    gta_check_symbol(I_vectorelt(vec, 0), 1, "a");
    gta_check_fixnum(I_vectorelt(vec, 1), 1);
    gta_check_string(I_vectorelt(vec, 2), 5, "delta");
    gta_check_char(I_vectorelt(vec, 3), 9);
    gta_check_float(I_vectorelt(vec, 4), 1.15);
    gta_check_repr(I_vectorelt(vec, 5), grim_false);

    return MUNIT_OK;
}


static MunitTest tests_vectors[] = {
    gta_test("simple", read_simple),
    gta_test("multi", read_multi),
    gta_endtests,
};

MunitSuite suite_vectors = {
    "/vectors",
    tests_vectors,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
