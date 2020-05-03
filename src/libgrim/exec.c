#include <assert.h>

#include "grim.h"
#include "internal.h"


grim_object grim_top_frame;


grim_object grim_call(grim_object func, int nargs, const grim_object *args) {
    assert(grim_type(func) == GRIM_FUNCTION);
    assert(nargs >= I_minargs(func));
    if (!I_varargs(func))
        assert(nargs <= I_maxargs(func));
    if (I_tag(func) == GRIM_CFUNC_TAG)
        return I_cfunc(func)(nargs, args);
    return grim_undefined;
}

grim_object grim_call_0(grim_object func) {
    return grim_call(func, 0, NULL);
}

grim_object grim_call_1(grim_object func, grim_object arg) {
    return grim_call(func, 1, &arg);
}

grim_object grim_call_2(grim_object func, grim_object arg1, grim_object arg2) {
    grim_object args[2] = {arg1, arg2};
    return grim_call(func, 2, args);
}
