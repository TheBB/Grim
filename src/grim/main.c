#include <stdio.h>

#include "grim.h"

int main() {
    grim_init();

    grim_print(grim_undefined);
    printf("\n");
    grim_print(grim_false);
    printf("\n");
    grim_print(grim_true);
    printf("\n");

    grim_print(grim_pack_fixnum(-2));
    printf("\n");

    grim_print(grim_pack_fixnum(-1));
    printf("\n");

    grim_print(grim_pack_fixnum(0));
    printf("\n");

    grim_print(grim_pack_fixnum(1));
    printf("\n");

    grim_print(grim_pack_fixnum(2));
    printf("\n");

    /* grim_print(grim_pack_fixnum(0)); */
    /* printf("\n"); */

    /* grim_print(grim_pack_fixnum(1)); */
    /* printf("\n"); */

    /* grim_print(grim_pack_fixnum(-1)); */
    /* printf("\n"); */
}
