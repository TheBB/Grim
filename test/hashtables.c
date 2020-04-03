#include "grim.h"
#include "internal.h"
#include "test.h"

static MunitResult insert(const MunitParameter params[], void *fixture) {
    grim_init();

    grim_object table = grim_hashtable_create(0);
    gta_check_hashtable(table, 0);

    grim_object key1 = grim_string_pack("alpha", NULL, false);
    grim_hashtable_set(table, key1, grim_true);
    gta_check_hashtable(table, 1);

    grim_object key2 = grim_integer_pack(1);
    grim_hashtable_set(table, key2, grim_true);
    gta_check_hashtable(table, 2);

    grim_object key3 = grim_intern("beta", NULL);
    grim_hashtable_set(table, key3, grim_true);
    gta_check_hashtable(table, 3);

    grim_object key4 = grim_character_pack_name("newline", NULL);
    grim_hashtable_set(table, key4, grim_true);
    gta_check_hashtable(table, 4);

    return MUNIT_OK;
}

static MunitResult retrieve(const MunitParameter params[], void *fixture) {
    grim_init();

    grim_object table = grim_hashtable_create(0);
    gta_check_hashtable(table, 0);

    grim_object key1 = grim_string_pack("alpha", NULL, false);
    grim_object val1 = grim_integer_pack(0);
    grim_hashtable_set(table, key1, val1);

    grim_object key2 = grim_integer_pack(1);
    grim_object val2 = grim_integer_pack(1);
    grim_hashtable_set(table, key2, val2);

    grim_object key3 = grim_intern("pan", NULL);
    grim_object val3 = grim_integer_pack(2);
    grim_hashtable_set(table, key3, val3);

    grim_object key4 = grim_character_pack_name("space", NULL);
    grim_object val4 = grim_integer_pack(3);
    grim_hashtable_set(table, key4, val4);
    gta_check_hashtable(table, 4);

    gta_check_hashtable(table, 4);

    munit_assert(grim_hashtable_has(table, key1));
    munit_assert(grim_hashtable_has(table, key2));
    munit_assert(grim_hashtable_has(table, key3));
    munit_assert(grim_hashtable_has(table, key4));
    munit_assert(grim_hashtable_has(table, grim_string_pack("alpha", NULL, true)));
    munit_assert(grim_hashtable_has(table, grim_intern("pan", NULL)));

    munit_assert(!grim_hashtable_has(table, grim_integer_pack(0)));
    munit_assert(!grim_hashtable_has(table, grim_string_pack("pan", NULL, false)));
    munit_assert(!grim_hashtable_has(table, grim_intern("alpha", NULL)));

    gta_check_fixnum(grim_hashtable_get(table, key1), 0);
    gta_check_fixnum(grim_hashtable_get(table, key2), 1);
    gta_check_fixnum(grim_hashtable_get(table, key3), 2);
    gta_check_fixnum(grim_hashtable_get(table, key4), 3);
    gta_check_fixnum(grim_hashtable_get(table, grim_string_pack("alpha", NULL, true)), 0);
    gta_check_fixnum(grim_hashtable_get(table, grim_intern("pan", NULL)), 2);

    return MUNIT_OK;
}

static MunitResult overwrite(const MunitParameter params[], void *fixture) {
    grim_object table = grim_hashtable_create(0);
    gta_check_hashtable(table, 0);

    grim_object key1 = grim_string_pack("alpha", NULL, false);
    grim_object key2 = grim_string_pack("beta", NULL, false);

    grim_hashtable_set(table, key1, grim_false);
    grim_hashtable_set(table, key2, grim_true);

    gta_check_hashtable(table, 2);
    gta_is_false(grim_hashtable_get(table, key1));
    gta_is_true(grim_hashtable_get(table, key2));

    grim_hashtable_set(table, key1, grim_true);
    grim_hashtable_set(table, key2, grim_false);

    gta_check_hashtable(table, 2);
    gta_is_true(grim_hashtable_get(table, key1));
    gta_is_false(grim_hashtable_get(table, key2));

    return MUNIT_OK;
}

static MunitResult delete(const MunitParameter params[], void *fixture) {
    grim_object table = grim_hashtable_create(0);
    gta_check_hashtable(table, 0);

    grim_object key1 = grim_string_pack("alpha", NULL, false);
    grim_object key2 = grim_string_pack("beta", NULL, false);
    grim_object key3 = grim_string_pack("gamma", NULL, false);

    grim_hashtable_set(table, key1, grim_false);
    grim_hashtable_set(table, key2, grim_true);
    gta_check_hashtable(table, 2);

    grim_hashtable_unset(table, key3);
    gta_check_hashtable(table, 2);
    munit_assert(grim_hashtable_has(table, key1));
    munit_assert(grim_hashtable_has(table, key2));
    munit_assert(!grim_hashtable_has(table, key3));

    grim_hashtable_unset(table, key2);
    gta_check_hashtable(table, 1);
    munit_assert(grim_hashtable_has(table, key1));
    munit_assert(!grim_hashtable_has(table, key2));
    munit_assert(!grim_hashtable_has(table, key3));

    grim_hashtable_unset(table, key1);
    gta_check_hashtable(table, 0);
    munit_assert(!grim_hashtable_has(table, key1));
    munit_assert(!grim_hashtable_has(table, key2));
    munit_assert(!grim_hashtable_has(table, key3));

    return MUNIT_OK;
}

static MunitResult stress(const MunitParameter params[], void *fixture) {
    grim_object table = grim_hashtable_create(0);
    for (intmax_t i = 0; i < 4000; i++)
        grim_hashtable_set(table, grim_integer_pack(i), grim_integer_pack(i));
    gta_check_hashtable(table, 4000);
    for (intmax_t i = 0; i < 4000; i++)
        gta_check_fixnum(grim_hashtable_get(table, grim_integer_pack(i)), i);
    return MUNIT_OK;
}

MunitTest tests_hashtables[] = {
    {"/insert", insert, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/retrieve", retrieve, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/overwrite", overwrite, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/delete", delete, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {"/stress", stress, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
};
