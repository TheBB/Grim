#include "munit.h"

#include "grim.h"
#include "internal.h"

extern MunitSuite suite_immediate_objects;
extern MunitSuite suite_numbers;
extern MunitSuite suite_strings;
extern MunitSuite suite_symbols;
extern MunitSuite suite_hashtables;

void *gt_setup(const MunitParameter params[], void *fixture);

#define gta_basic(name)                                                        \
    { ("/" #name), name, gt_setup, NULL, MUNIT_TEST_OPTION_NONE, NULL, }

#define gta_test(name, func)                                                   \
    { ("/" name), func, gt_setup, NULL, MUNIT_TEST_OPTION_NONE, NULL, }

#define gta_endtests                                                           \
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }

#define gta_endsuite                                                           \
    { NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE }

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

#define gta_is_float(c)                                                        \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_FLOAT);                        \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_FLOAT_TAG);            \
    } while (0)

#define gta_check_float(c, v)                                                  \
    do {                                                                       \
        grim_object y = (c);                                                   \
        double w = v;                                                          \
        gta_is_float(y);                                                       \
        munit_assert_double(grim_float_extract(y), ==, w);                     \
    } while (0)

#define gta_check_float_approx(c, v, n)                                        \
    do {                                                                       \
        grim_object y = (c);                                                   \
        double w = v;                                                          \
        gta_is_float(y);                                                       \
        munit_assert_double_equal(grim_float_extract(y), w, n);                \
    } while (0)

#define gta_is_bigint(c)                                                       \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_int(y);                                                         \
        munit_assert_int(grim_direct_tag(y), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(y), ==, GRIM_BIGINT_TAG);           \
    } while (0)

#define gta_check_bigint(c, v)                                                 \
    do {                                                                       \
        grim_object x = (c);                                                   \
        const char *w = v;                                                     \
        gta_is_bigint(x);                                                      \
        mpz_t t;                                                               \
        mpz_init_set_str(t, w, 10);                                            \
        int res = mpz_cmp(t, I(x)->bigint);                                    \
        mpz_clear(t);                                                          \
        munit_assert_int(res, ==, 0);                                          \
    } while (0)

#define gta_is_rational(c)                                                     \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_RATIONAL);                     \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_RATIONAL_TAG);         \
    } while (0)

#define gta_is_complex(c)                                                      \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_COMPLEX);                      \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_COMPLEX_TAG);          \
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
        munit_assert_ullong(I(y)->strlen, ==, L);                              \
        munit_assert_ullong(I(y)->strlen, ==, L);                              \
        for (size_t i = 0; i < L; i++)                                         \
            munit_assert_uint8(I(y)->str[i], ==, w[i]);                        \
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
        munit_assert_ullong(I(y)->buflen, ==, L);                              \
        for (size_t i = 0; i < L; i++)                                         \
            munit_assert_char(I(y)->buf[i], ==, w[i]);                         \
    } while (0)

#define gta_is_symbol(c)                                                       \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_SYMBOL);                       \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_SYMBOL_TAG);             \
    } while (0)

#define gta_check_symbol(c, l, v)                                              \
    do {                                                                       \
        grim_object x = (c);                                                   \
        gta_is_symbol(x);                                                      \
        gta_check_string(grim_symbol_name(x), l, v);                           \
    } while (0)

#define gta_is_hashtable(c)                                                    \
    do {                                                                       \
        grim_object z = (c);                                                   \
        munit_assert_int(grim_type(z), ==, GRIM_HASHTABLE);                    \
        munit_assert_int(grim_direct_tag(z), ==, GRIM_INDIRECT_TAG);           \
        munit_assert_int(grim_indirect_tag(z), ==, GRIM_HASHTABLE_TAG);        \
    } while (0)

#define gta_check_hashtable(c, l)                                              \
    do {                                                                       \
        grim_object y = (c);                                                   \
        gta_is_hashtable(y);                                                   \
        size_t L = (l);                                                        \
        munit_assert_ullong(I(y)->hashfill, ==, L);                            \
    } while (0)
