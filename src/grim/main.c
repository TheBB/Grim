#include <locale.h>
#include <stdio.h>

#include "grim.h"

int main() {
    setlocale(LC_ALL, "");
    grim_init();

    grim_print(grim_undefined);
    printf("\n");
    grim_print(grim_false);
    printf("\n");
    grim_print(grim_true);
    printf("\n");
    grim_print(grim_nil);
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

    grim_object z = grim_pack_string("å", NULL);
    grim_print(z);
    printf("\n");

    grim_object v = grim_create_vector(3);
    grim_vector_set(v, 0, grim_false);
    grim_vector_set(v, 1, grim_true);
    grim_vector_set(v, 2, grim_nil);
    grim_print(v);
    printf("\n");

    grim_object c = grim_create_cons(grim_pack_integer(0), grim_pack_integer(1));
    grim_print(c);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(0), grim_nil);
    grim_print(c);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(1), c);
    grim_print(c);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(2), c);
    grim_print(c);
    printf("\n");
}
