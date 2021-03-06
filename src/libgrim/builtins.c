#include <assert.h>

#include "grim.h"
#include "internal.h"


grim_object grim_builtin_module;


grim_object gf_add(int nargs, const grim_object *args) {
    if (nargs == 0)
        return grim_integer_pack(0);
    grim_object sum = args[0];
    for (int i = 1; i < nargs; i++)
        sum = grim_add(sum, args[i], false);
    return sum;
}

grim_object gf_sub(int nargs, const grim_object *args) {
    if (nargs == 0)
        return grim_integer_pack(0);
    grim_object sum = args[0];
    for (int i = 1; i < nargs; i++)
        sum = grim_add(sum, args[i], true);
    return sum;
}
