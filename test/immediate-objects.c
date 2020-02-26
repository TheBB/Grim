#include "munit.h"

MunitResult empty(const MunitParameter params[], void *fixture) {
    munit_assert_int(1, ==, 1);
    return MUNIT_OK;
}

MunitTest tests[] = {
    {"/empty", empty, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL},
    { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite suite = {
    "/immediate-objects", tests, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char * const *argv) {
    return munit_suite_main(&suite, NULL, argc, argv);
}
