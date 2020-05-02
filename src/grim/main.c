#include <locale.h>
#include <stdio.h>

#include "grim.h"
#include "internal.h"

int main() {
    setlocale(LC_ALL, "");
    grim_init();

    grim_object name = grim_intern("--main--", NULL);
    grim_object code = grim_string_pack("(%module-set! a 1)", NULL, false);
    grim_object m = grim_build_module(name, code);
    grim_print(m, "UTF-8");
    printf("\n");
    grim_print(grim_module_cell(m, grim_intern("a", NULL), true), "UTF-8");
    printf("\n");
}
