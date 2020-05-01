#include "test.h"

int main(int argc, char * const *argv) {
    MunitSuite suites[] = {
        suite_immediate_objects,
        suite_numbers,
        suite_strings,
        suite_symbols,
        suite_lists,
        suite_hashtables,
        gta_endsuite,
    };

    MunitSuite suite = {"/grim", NULL, suites, 1, MUNIT_SUITE_OPTION_NONE};
    return munit_suite_main(&suite, NULL, argc, argv);
}

void *gt_setup(const MunitParameter params[], void *fixture) {
    grim_init();
    return NULL;
}
