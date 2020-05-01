#include <locale.h>
#include <stdio.h>

#include "grim.h"
#include "internal.h"

int main() {
    setlocale(LC_ALL, "");
    grim_init();

    grim_object z = grim_string_pack("(#\\a)", NULL, false);
    grim_object y = grim_read(z);
    grim_print(y, "UTF-8");
    printf("\n");
}
