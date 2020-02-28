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

    grim_print(grim_pack_integer(-2));
    printf("\n");

    grim_print(grim_pack_integer(-1));
    printf("\n");

    grim_print(grim_pack_integer(0));
    printf("\n");

    grim_print(grim_pack_integer(1));
    printf("\n");

    grim_print(grim_pack_integer(2));
    printf("\n");

    grim_print(grim_pack_integer(4611686018427387903));
    printf("\n");

    grim_print(grim_pack_integer(-4611686018427387904));
    printf("\n");

    grim_print(grim_pack_integer(4611686018427387904));
    printf("\n");

    grim_print(grim_pack_integer(-4611686018427387905));
    printf("\n");
}
