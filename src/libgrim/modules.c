#include <assert.h>

#include "grim.h"
#include "internal.h"


grim_object grim_eval_in_module(grim_object module, grim_object expr) {
    if (grim_type(expr) != GRIM_CONS)
        return expr;

    grim_object car = grim_car(expr);
    if (car == gs_i_moduleset) {
        grim_object cdr = grim_cdr(expr);
        grim_object name = grim_car(cdr);
        grim_object value = grim_eval_in_module(module, grim_car(grim_cdr(cdr)));
        grim_module_set(module, name, value);
        return value;
    }

    assert(false);
}

grim_object grim_build_module(grim_object name, grim_object code) {
    grim_object module = grim_module_create(name);

    code = grim_read_all(code);
    assert(code != grim_undefined);
    while (code != grim_nil) {
        grim_eval_in_module(module, grim_car(code));
        code = grim_cdr(code);
    }
    return module;
}
