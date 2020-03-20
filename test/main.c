#include "test.h"

static const MunitSuite suite = {
    "/immediate-objects", suite_immediate_objects, NULL, 1, MUNIT_SUITE_OPTION_NONE
};

int main(int argc, char * const *argv) {
    return munit_suite_main(&suite, NULL, argc, argv);
}
