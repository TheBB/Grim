#include <assert.h>
#include <stdalign.h>
#include <stdio.h>

#include "gc.h"
#include "gmp.h"

#include "grim.h"
#include "internal.h"


const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;


void grim_init() {
    assert(GRIM_ALIGN >= 16);
    GC_INIT();
}

void grim_fprint(grim_object obj, FILE *stream) {
    switch(grim_get_direct_tag(obj)) {
    case GRIM_FALSE_TAG:
        fprintf(stream, "#f");
        return;
    case GRIM_TRUE_TAG:
        fprintf(stream, "#t");
        return;
    case GRIM_FIXNUM_TAG:
        fprintf(stream, "%ld", grim_extract_integer(obj));
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_get_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG:
            mpz_out_str(stream, 10, ((grim_indirect *) obj)->bigint);
            return;
        }
    default: case GRIM_UNDEFINED_TAG:
        fprintf(stream, "#undefined");
        return;
    }
}

void grim_print(grim_object obj) {
    grim_fprint(obj, stdout);
}
