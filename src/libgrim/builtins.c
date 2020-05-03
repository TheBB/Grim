#include <assert.h>

#include "grim.h"
#include "internal.h"


grim_object grim_builtin_module;


grim_object gf_identity(int nargs, const grim_object *args) {
    (void) nargs;
    return args[0];
}
