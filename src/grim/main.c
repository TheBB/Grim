#include <locale.h>
#include <stdio.h>

#include "grim.h"

int main() {
    setlocale(LC_ALL, "");
    grim_init();

    grim_display(grim_undefined, NULL);
    printf("\n");
    grim_display(grim_false, NULL);
    printf("\n");
    grim_display(grim_true, NULL);
    printf("\n");
    grim_display(grim_nil, NULL);
    printf("\n");

    grim_display(grim_pack_integer(-2), NULL);
    printf("\n");

    grim_display(grim_pack_integer(-1), NULL);
    printf("\n");

    grim_display(grim_pack_integer(0), NULL);
    printf("\n");

    grim_display(grim_pack_integer(1), NULL);
    printf("\n");

    grim_display(grim_pack_integer(2), NULL);
    printf("\n");

    grim_display(grim_pack_integer(4611686018427387903), NULL);
    printf("\n");

    grim_display(grim_pack_integer(-4611686018427387904), NULL);
    printf("\n");

    grim_display(grim_pack_integer(4611686018427387904), NULL);
    printf("\n");

    grim_display(grim_pack_integer(-4611686018427387905), NULL);
    printf("\n");

    grim_object z = grim_pack_string_escape("zz", NULL);
    /* grim_object z = grim_pack_string_escape("æ\\U000000f8\\u00e5", NULL); */
    /* grim_object z = grim_pack_string_escape("abc\\n", NULL); */
    /* grim_object z = grim_pack_string_escape("abc\\u00e5\\n", NULL); */
    grim_print(z, NULL);
    printf("\n");

    grim_object v = grim_create_vector(3);
    grim_vector_set(v, 0, grim_false);
    grim_vector_set(v, 1, grim_true);
    grim_vector_set(v, 2, grim_nil);
    grim_display(v, NULL);
    printf("\n");

    grim_object c = grim_create_cons(grim_pack_integer(0), grim_pack_integer(1));
    grim_display(c, NULL);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(0), grim_nil);
    grim_display(c, NULL);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(1), c);
    grim_display(c, NULL);
    printf("\n");

    c = grim_create_cons(grim_pack_integer(2), c);
    grim_display(c, NULL);
    printf("\n");

    grim_object s = grim_intern("dingbob", NULL);
    printf("%lx\n", s);

    grim_object ss = grim_intern("dingbob", NULL);
    printf("%lx\n", ss);

    grim_display(s, NULL);
    printf("\n");

    grim_object d = grim_pack_character(0);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character('\n');
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character('a');
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character(' ');
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("^@", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("null", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("\\", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("^", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("x", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("U000000f8", NULL);
    grim_print(d, NULL);
    printf("\n");

    d = grim_pack_character_name("u00e5", NULL);
    grim_print(d, NULL);
    printf("\n");
}
