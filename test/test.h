#include "munit.h"

#include "grim.h"
#include "internal.h"

extern MunitTest tests_immediate_objects[];
extern MunitTest tests_strings[];

#define gta_check_repr(c, v)                                                   \
    do {                                                                       \
        munit_assert_ullong(c, ==, v);                                         \
    } while (0)

#define gta_is_undefined(c)                                                    \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_UNDEFINED);                    \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_UNDEFINED_TAG);          \
    } while (0)

#define gta_is_bool(c)                                                         \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_BOOLEAN);                      \
    } while (0)

#define gta_is_false(c)                                                        \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_bool(y);                                                        \
        munit_assert_int(grim_direct_tag(y), ==, GRIM_FALSE_TAG);              \
    } while (0)

#define gta_is_true(c)                                                         \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_bool(y);                                                        \
        munit_assert_int(grim_direct_tag(y), ==, GRIM_TRUE_TAG);               \
    } while (0)

#define gta_is_nil(c)                                                          \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_NIL);                          \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_NIL_TAG);                \
    } while (0)

#define gta_is_int(c)                                                          \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_INTEGER);                      \
    } while (0)

#define gta_is_fixnum(c)                                                       \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_int(y);                                                         \
        munit_assert_int(grim_direct_tag(y), ==, GRIM_FIXNUM_TAG);             \
    } while (0)

#define gta_check_fixnum(c, v)                                                 \
    do {                                                                       \
        grim_object x = (c);                                                   \
        gta_is_fixnum(x);                                                      \
        munit_assert(grim_integer_extractable(x));                             \
        munit_assert_llong(grim_integer_extract(x), ==, v);                    \
    } while (0)

#define gta_is_char(c)                                                         \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_CHARACTER);                    \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_CHARACTER_TAG);          \
    } while (0)

#define gta_check_char(c, v)                                                   \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_char(y);                                                        \
        munit_assert_ulong(grim_character_extract(y), ==, v);                  \
    } while (0)

#define gta_is_string(c)                                                       \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_STRING);                       \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_STRING_TAG);           \
    } while (0)

#define gta_check_string(c, l, v)                                              \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_string(y);                                                      \
        size_t L = (l);                                                        \
        uint8_t *w = (uint8_t *)(v);                                           \
        munit_assert_ullong(((grim_indirect *)y)->strlen, ==, L);              \
        munit_assert_ullong(grim_strlen(y), ==, L);                            \
        for (size_t i = 0; i < L; i++)                                         \
            munit_assert_uint8(((grim_indirect *)y)->str[i], ==, w[i]);        \
    } while (0)

#define gta_is_buffer(c)                                                       \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_BUFFER);                       \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_BUFFER_TAG);           \
    } while (0)

#define gta_check_buffer(c, l, v)                                              \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_buffer(y);                                                      \
        size_t L = (l);                                                        \
        char *w = (char *)(v);                                                 \
        munit_assert_ullong(((grim_indirect *)y)->buflen, ==, L);              \
        for (size_t i = 0; i <= L; i++)                                        \
            munit_assert_char(((grim_indirect *)y)->buf[i], ==, w[i]);         \
    } while (0)
