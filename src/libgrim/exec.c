#include <assert.h>

#include "grim.h"
#include "internal.h"


grim_object grim_top_frame;


#define NEXT_INSTRUCTION() (*(bytecode++))
#define NEXT_OFFSET() ((size_t) (*(bytecode++)))
#define PUSH(v) do { *(stack++) = (v); } while (0)
#define POP() (*(--stack))


static grim_object grim_exec_frame(grim_object frame) {
    grim_object func = I_framefunc(frame);

    // Avoid as much indirection as we can: load all pointers
    grim_object *refs = I_vectordata(I_funcrefs(func));
    grim_object *args = I_vectordata(I_framestack(frame));
    grim_object *locals = &args[I_nargs(func) + I_variadic(func)];
    grim_object *stack = &locals[I_nlocals(func)];
    const char *bytecode = I_buf(I_bytecode(func));

    while (true) {
        switch (NEXT_INSTRUCTION()) {
        case GRIM_BC_LOAD_REF:
            PUSH(refs[NEXT_OFFSET()]);
            break;
        case GRIM_BC_LOAD_REF_CELL:
            PUSH(I_cellvalue(refs[NEXT_OFFSET()]));
            break;
        case GRIM_BC_LOAD_ARG:
            PUSH(args[NEXT_OFFSET()]);
            break;
        case GRIM_BC_STORE_LOCAL:
            locals[NEXT_OFFSET()] = POP();
            break;
        case GRIM_BC_CALL: {
            size_t nargs = NEXT_OFFSET();
            grim_object func = POP();
            grim_object retval = grim_call(func, nargs, stack - nargs);
            for (size_t i = 0; i < nargs; i++)
                (void) POP();
            PUSH(retval);
            break;
        }
        case GRIM_BC_RETURN:
            return POP();
        }
    }
}


grim_object grim_call(grim_object func, size_t nargs, const grim_object *args) {
    assert(grim_type(func) == GRIM_FUNCTION);
    if (I_variadic(func))
        assert(nargs >= I_nargs(func));
    else
        assert(nargs == I_nargs(func));
    if (I_tag(func) == GRIM_CFUNC_TAG)
        return I_cfunc(func)(nargs, args);

    grim_object frame = grim_frame_create(func, grim_top_frame);
    for (size_t i = 0; i < I_nargs(func); i++)
        I_vectorelt(I_framestack(frame), i) = args[i];
    if (I_variadic(func)) {
        grim_object head = grim_nil, tail;
        for (size_t i = I_nargs(func); i < nargs; i++) {
            if (head == grim_nil)
                head = tail = grim_cons_pack(args[i], grim_nil);
            else {
                grim_object newtail = grim_cons_pack(args[i], grim_nil);
                I_cdr(tail) = newtail;
                tail = newtail;
            }
        }
        I_vectorelt(I_framestack(frame), I_nargs(func)) = head;
    }

    grim_top_frame = frame;
    grim_object retval = grim_exec_frame(frame);
    grim_top_frame = I_parentframe(frame);
    return retval;
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
