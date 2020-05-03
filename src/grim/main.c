#include <locale.h>
#include <stdio.h>

#include "grim.h"
#include "internal.h"

int main(int argc, char **argv) {
    setlocale(LC_ALL, "");
    if (argc < 2) {
        printf("Usage: grim FILENAME\n");
        return 1;
    }

    grim_init();

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Unable to open %s\n", argv[1]);
        return 1;
    }

    grim_object code = grim_read_file(file);
    fclose(file);

    grim_object name = grim_intern("--main--", NULL);
    grim_object module = grim_build_module(name, code);
    grim_print(module, "UTF-8");
    printf("\n");
    grim_print(grim_module_cell(module, grim_intern("a", NULL), true), "UTF-8");
    printf("\n");
    grim_print(grim_module_cell(module, grim_intern("b", NULL), true), "UTF-8");
    printf("\n");
}
