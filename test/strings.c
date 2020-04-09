#include "grim.h"
#include "test.h"


static MunitResult basic(const MunitParameter params[], void *fixture) {
    grim_object str;

    str = grim_string_pack("", NULL, false);
    gta_check_string(str, 0, "");

    str = grim_string_pack("", NULL, true);
    gta_check_string(str, 0, "");

    str = grim_string_pack("abcdef", NULL, false);
    gta_check_string(str, 6, "abcdef");

    str = grim_string_pack("xyz", NULL, true);
    gta_check_string(str, 3, "xyz");

    return MUNIT_OK;
}

static MunitResult escape(const MunitParameter params[], void *fixture) {
    grim_object str;

    str = grim_string_pack("\\a\\0", "UTF-8", false);
    gta_check_string(str, 4, "\\a\\0");

    str = grim_string_pack("\\a\\0", "UTF-8", true);
    gta_check_string(str, 2, "\x07\x00");

    str = grim_string_pack("\\\"", "UTF-8", true);
    gta_check_string(str, 1, "\"");

    str = grim_string_pack("\\a\\b\\t\\n\\v\\f\\r\\^T\\^_\\\"\\\\\\^?\\e", "UTF-8", true);
    gta_check_string(str, 13, "\x07\x08\x09\x0a\x0b\x0c\x0d\x14\x1f\x22\x5c\x7f\x1b");

    str = grim_string_pack("\\U000000f8", "UTF-8", true);
    gta_check_string(str, 2, "\xc3\xb8");

    str = grim_string_pack("\\u00e5", "UTF-8", true);
    gta_check_string(str, 2, "\xc3\xa5");

    return MUNIT_OK;
}

static MunitResult display(const MunitParameter params[], void *fixture) {
    grim_object str, buf;

    str = grim_string_pack("abc", "UTF-8", false);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 3, "abc");

    str = grim_string_pack("\\a\\0", "UTF-8", false);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 4, "\\a\\0");

    str = grim_string_pack("\\a\\0", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 2, "\x07\x00");

    str = grim_string_pack("\\a\\b\\t\\n\\v\\f\\r\\^T\\^_\\\"\\\\\\^?\\e", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 13, "\x07\x08\x09\x0a\x0b\x0c\x0d\x14\x1f\x22\x5c\x7f\x1b");

    str = grim_string_pack("\\U000000f8", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 2, "\xc3\xb8");

    str = grim_string_pack("\\u00e5", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_display(buf, str, "UTF-8");
    gta_check_buffer(buf, 2, "\xc3\xa5");

    return MUNIT_OK;
}

static MunitResult print(const MunitParameter params[], void *fixture) {
    grim_object str, buf;

    str = grim_string_pack("abc", "UTF-8", false);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 5, "\"abc\"");

    str = grim_string_pack("\\a\\0", "UTF-8", false);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 8, "\"\\\\a\\\\0\"");

    str = grim_string_pack("\\a\\0", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 6, "\"\\a\\0\"");

    str = grim_string_pack("\\\"", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 4, "\"\\\"\"");

    str = grim_string_pack("\\a\\b\\t\\n\\v\\f\\r\\^T\\^_\\\"\\\\\\^?\\e", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 31, "\"\\a\\b\\t\\n\\v\\f\\r\\^T\\^_\\\"\\\\\\^?\\e\"");

    str = grim_string_pack("\\U000000f8", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 4, "\"\xc3\xb8\"");

    str = grim_string_pack("\\u00e5", "UTF-8", true);
    buf = grim_buffer_create(0);
    grim_encode_print(buf, str, "UTF-8");
    gta_check_buffer(buf, 4, "\"\xc3\xa5\"");

    return MUNIT_OK;
}

MunitTest tests_strings[] = {
    gta_basic(basic),
    gta_basic(escape),
    gta_basic(display),
    gta_basic(print),
    gta_endtests,
};

MunitSuite suite_strings = {
    "/strings",
    tests_strings,
    NULL,
    1, MUNIT_SUITE_OPTION_NONE,
};
