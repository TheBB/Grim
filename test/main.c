#include "test.h"

static MunitSuite suites[] = {
    {"/immediate-objects", tests_immediate_objects, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/numbers", tests_numbers, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/strings", tests_strings, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {"/hashtables", tests_hashtables, NULL, 1, MUNIT_SUITE_OPTION_NONE},
    {NULL, NULL, NULL, 0, MUNIT_SUITE_OPTION_NONE}
};

static const MunitSuite suite = {"/grim", NULL, suites, 1, MUNIT_SUITE_OPTION_NONE};

int main(int argc, char * const *argv) {
    return munit_suite_main(&suite, NULL, argc, argv);
}
