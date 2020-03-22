#include "test.h"

static MunitResult basic(const MunitParameter params[], void *fixture) {
    grim_object str;

    str = grim_pack_string("", NULL);
    gta_check_string(str, 0, "");

    str = grim_pack_string_escape("", NULL);
    gta_check_string(str, 0, "");

    str = grim_pack_string("abcdef", NULL);
    gta_check_string(str, 6, "abcdef");

    str = grim_pack_string_escape("xyz", NULL);
    gta_check_string(str, 3, "xyz");

    return MUNIT_OK;
}

static MunitResult escape(const MunitParameter params[], void *fixture) {
    grim_object str;

    str = grim_pack_string("\\a\\0", NULL);
    gta_check_string(str, 4, "\\a\\0");

    str = grim_pack_string_escape("\\a\\0", NULL);
    gta_check_string(str, 2, "\x07\x00");

    str = grim_pack_string_escape("\\a\\b\\t\\n\\v\\f\\r\\^T\\^_\\\"\\\\\\^?\\e", NULL);
    gta_check_string(str, 13, "\x07\x08\x09\x0a\x0b\x0c\x0d\x14\x1f\x22\x5c\x7f\x1b");

    str = grim_pack_string_escape("\\U000000f8", NULL);
    gta_check_string(str, 2, "\xc3\xb8");

    str = grim_pack_string_escape("\\u00e5", NULL);
    gta_check_string(str, 2, "\xc3\xa5");

    return MUNIT_OK;
}

MunitTest tests_strings[] = {
    {"/basic", basic, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/escape", escape, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL },
};
