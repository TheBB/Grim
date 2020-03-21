#include "test.h"


static MunitResult undefined(const MunitParameter params[], void *fixture) {
    gta_is_undefined(grim_undefined);
    gta_check_repr(grim_undefined, GRIM_UNDEFINED_TAG);
    return MUNIT_OK;
}

static MunitResult booleans(const MunitParameter params[], void *fixture) {
    gta_is_true(grim_true);
    gta_check_repr(grim_true, GRIM_TRUE_TAG);

    gta_is_false(grim_false);
    gta_check_repr(grim_false, GRIM_FALSE_TAG);

    return MUNIT_OK;
}

static MunitResult nil(const MunitParameter params[], void *fixture) {
    gta_is_nil(grim_nil);
    gta_check_repr(grim_nil, GRIM_NIL_TAG);
    return MUNIT_OK;
}

static MunitResult fixnums(const MunitParameter params[], void *fixture) {
    grim_object zero = grim_pack_integer(0);
    gta_check_fixnum(zero, 0);
    gta_check_repr(zero, 1);

    grim_object one = grim_pack_integer(1);
    gta_check_fixnum(one, 1);
    gta_check_repr(one, 3);

    grim_object negone = grim_pack_integer(-1);
    gta_check_fixnum(negone, -1);
    gta_check_repr(negone, UINTPTR_MAX);

    grim_object max = grim_pack_integer(GRIM_FIXNUM_MAX);
    gta_check_fixnum(max, GRIM_FIXNUM_MAX);
    gta_check_repr(max, UINTPTR_MAX / 2);

    grim_object min = grim_pack_integer(GRIM_FIXNUM_MIN);
    gta_check_fixnum(min, GRIM_FIXNUM_MIN);
    gta_check_repr(min, UINTPTR_MAX - (UINTPTR_MAX / 2) + 1);

    return MUNIT_OK;
}

static MunitResult characters(const MunitParameter params[], void *fixture) {
    grim_object ch, alt;

    ch = grim_pack_character_name("a", NULL);
    gta_check_char(ch, 97);
    gta_check_repr(ch, (97 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("^H", NULL);
    gta_check_char(ch, 8);
    gta_check_repr(ch, (8 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("^?", NULL);
    gta_check_char(ch, 127);
    gta_check_repr(ch, (127 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("null", NULL);
    gta_check_char(ch, 0);
    gta_check_repr(ch, GRIM_CHARACTER_TAG);
    gta_check_repr(ch, grim_pack_character_name("^@", NULL));

    ch = grim_pack_character_name("bell", NULL);
    gta_check_char(ch, 7);
    gta_check_repr(ch, (7 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("backspace", NULL);
    gta_check_char(ch, 8);
    gta_check_repr(ch, (8 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("tab", NULL);
    gta_check_char(ch, 9);
    gta_check_repr(ch, (9 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("linefeed", NULL);
    gta_check_char(ch, 10);
    gta_check_repr(ch, (10 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("verticaltab", NULL);
    gta_check_char(ch, 11);
    gta_check_repr(ch, (11 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("formfeed", NULL);
    gta_check_char(ch, 12);
    gta_check_repr(ch, (12 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("return", NULL);
    gta_check_char(ch, 13);
    gta_check_repr(ch, (13 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("escape", NULL);
    gta_check_char(ch, 27);
    gta_check_repr(ch, (27 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("space", NULL);
    gta_check_char(ch, 32);
    gta_check_repr(ch, (32 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("backslash", NULL);
    gta_check_char(ch, 92);
    gta_check_repr(ch, (92 << 8) | GRIM_CHARACTER_TAG);
    gta_check_repr(ch, grim_pack_character_name("\\", NULL));

    ch = grim_pack_character_name("caret", NULL);
    gta_check_char(ch, 94);
    gta_check_repr(ch, (94 << 8) | GRIM_CHARACTER_TAG);
    gta_check_repr(ch, grim_pack_character_name("^", NULL));

    ch = grim_pack_character_name("U000000f8", NULL);
    gta_check_char(ch, 248);
    gta_check_repr(ch, (248 << 8) | GRIM_CHARACTER_TAG);

    ch = grim_pack_character_name("u00e5", NULL);
    gta_check_char(ch, 229);
    gta_check_repr(ch, (229 << 8) | GRIM_CHARACTER_TAG);

    return MUNIT_OK;
}

MunitTest tests_immediate_objects[] = {
    {"/undefined", undefined, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/booleans", booleans, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/nil", nil, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/fixnums", fixnums, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/characters", characters, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};
