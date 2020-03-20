#include "grim.h"
#include "internal.h"
#include "munit.h"
#include "test.h"


static MunitResult undefined(const MunitParameter params[], void *fixture) {
    munit_assert_int(grim_get_type(grim_undefined), ==, GRIM_UNDEFINED);
    munit_assert_int(grim_get_direct_tag(grim_undefined), ==, GRIM_UNDEFINED_TAG);
    return MUNIT_OK;
}

static MunitResult booleans(const MunitParameter params[], void *fixture) {
    munit_assert_int(grim_get_type(grim_true), ==, GRIM_BOOLEAN);
    munit_assert_int(grim_get_direct_tag(grim_true), ==, GRIM_TRUE_TAG);
    munit_assert_int(grim_get_type(grim_false), ==, GRIM_BOOLEAN);
    munit_assert_int(grim_get_direct_tag(grim_false), ==, GRIM_FALSE_TAG);
    return MUNIT_OK;
}

static MunitResult nil(const MunitParameter params[], void *fixture) {
    munit_assert_int(grim_get_type(grim_nil), ==, GRIM_NIL);
    munit_assert_int(grim_get_direct_tag(grim_nil), ==, GRIM_NIL_TAG);
    return MUNIT_OK;
}

static MunitResult fixnums(const MunitParameter params[], void *fixture) {
    grim_object zero = grim_pack_integer(0);
    munit_assert_int(grim_get_type(zero), ==, GRIM_INTEGER);
    munit_assert_int(grim_get_direct_tag(zero), ==, GRIM_FIXNUM_TAG);
    munit_assert_ullong(zero, ==, 1);
    munit_assert(grim_can_extract_integer(zero));
    munit_assert_llong(grim_extract_integer(zero), ==, 0);

    grim_object one = grim_pack_integer(1);
    munit_assert_int(grim_get_type(one), ==, GRIM_INTEGER);
    munit_assert_int(grim_get_direct_tag(one), ==, GRIM_FIXNUM_TAG);
    munit_assert_ullong(one, ==, 3);
    munit_assert(grim_can_extract_integer(one));
    munit_assert_llong(grim_extract_integer(one), ==, 1);

    grim_object negone = grim_pack_integer(-1);
    munit_assert_int(grim_get_type(negone), ==, GRIM_INTEGER);
    munit_assert_int(grim_get_direct_tag(negone), ==, GRIM_FIXNUM_TAG);
    munit_assert_ullong(negone, ==, UINTPTR_MAX);
    munit_assert(grim_can_extract_integer(negone));
    munit_assert_llong(grim_extract_integer(negone), ==, -1);

    grim_object max = grim_pack_integer(GRIM_FIXNUM_MAX);
    munit_assert_int(grim_get_type(max), ==, GRIM_INTEGER);
    munit_assert_int(grim_get_direct_tag(max), ==, GRIM_FIXNUM_TAG);
    munit_assert_ullong(max, ==, UINTPTR_MAX / 2);
    munit_assert(grim_can_extract_integer(max));
    munit_assert_llong(grim_extract_integer(max), ==, GRIM_FIXNUM_MAX);

    grim_object min = grim_pack_integer(GRIM_FIXNUM_MIN);
    munit_assert_int(grim_get_type(min), ==, GRIM_INTEGER);
    munit_assert_int(grim_get_direct_tag(min), ==, GRIM_FIXNUM_TAG);
    munit_assert_ullong(min, ==, UINTPTR_MAX - (UINTPTR_MAX / 2) + 1);
    munit_assert(grim_can_extract_integer(min));
    munit_assert_llong(grim_extract_integer(min), ==, GRIM_FIXNUM_MIN);

    return MUNIT_OK;
}

static MunitResult characters(const MunitParameter params[], void *fixture) {
    grim_object ch, alt;

    ch = grim_pack_character_name("a", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (97 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 97);

    ch = grim_pack_character_name("^H", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (8 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 8);

    ch = grim_pack_character_name("^?", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (127 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 127);

    ch = grim_pack_character_name("null", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 0);

    alt = grim_pack_character_name("^@", NULL);
    munit_assert_ullong(ch, ==, alt);

    ch = grim_pack_character_name("bell", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (7 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 7);

    ch = grim_pack_character_name("backspace", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (8 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 8);

    ch = grim_pack_character_name("tab", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (9 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 9);

    ch = grim_pack_character_name("linefeed", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (10 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 10);

    ch = grim_pack_character_name("verticaltab", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (11 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 11);

    ch = grim_pack_character_name("formfeed", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (12 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 12);

    ch = grim_pack_character_name("return", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (13 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 13);

    ch = grim_pack_character_name("escape", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (27 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 27);

    ch = grim_pack_character_name("space", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (32 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 32);

    ch = grim_pack_character_name("backslash", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (92 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 92);

    alt = grim_pack_character_name("\\", NULL);
    munit_assert_ullong(ch, ==, alt);

    ch = grim_pack_character_name("caret", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (94 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 94);

    alt = grim_pack_character_name("^", NULL);
    munit_assert_ullong(ch, ==, alt);

    ch = grim_pack_character_name("U000000f8", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (248 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 248);

    ch = grim_pack_character_name("u00e5", NULL);
    munit_assert_int(grim_get_type(ch), ==, GRIM_CHARACTER);
    munit_assert_int(grim_get_direct_tag(ch), ==, GRIM_CHARACTER_TAG);
    munit_assert_ullong(ch, ==, (229 << 8) | GRIM_CHARACTER_TAG);
    munit_assert_ulong(grim_extract_character(ch), ==, 229);

    return MUNIT_OK;
}

MunitTest suite_immediate_objects[] = {
    {"/undefined", undefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/booleans", booleans, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/nil", nil, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/fixnums", fixnums, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/characters", characters, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};
