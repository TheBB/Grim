#include <assert.h>
#include <stdalign.h>
#include <stdio.h>

#include "gc.h"
#include "gmp.h"
#include "unistdio.h"

#include "grim.h"
#include "internal.h"


const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;
const grim_object grim_nil = GRIM_NIL_TAG;


void grim_init() {
    assert(GRIM_ALIGN >= 16);
    GC_INIT();
}

void grim_fprint(FILE *stream, grim_object obj) {
    switch(grim_get_direct_tag(obj)) {
    case GRIM_FALSE_TAG:
        fprintf(stream, "#f");
        return;
    case GRIM_TRUE_TAG:
        fprintf(stream, "#t");
        return;
    case GRIM_NIL_TAG:
        fprintf(stream, "nil");
        return;
    case GRIM_FIXNUM_TAG:
        fprintf(stream, "%ld", grim_extract_integer(obj));
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_get_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG:
            mpz_out_str(stream, 10, ((grim_indirect *) obj)->bigint);
            return;
        case GRIM_STRING_TAG:
            fprintf(stream, "%d %d %d\n",
                    ((grim_indirect *) obj)->str[0],
                    ((grim_indirect *) obj)->str[1],
                    ((grim_indirect *) obj)->str[2]
            );
            ulc_fprintf(stream, "\"%U\"", ((grim_indirect *) obj)->str);
            return;
        case GRIM_VECTOR_TAG:
            {
                fprintf(stream, "#(");
                size_t len = grim_vector_size(obj);
                for (size_t i = 0; i < len; i++) {
                    grim_fprint(stream, grim_vector_get(obj, i));
                    if (i + 1 < len)
                        fprintf(stream, " ");
                }
                fprintf(stream, ")");
            }
            return;
        }
    default: case GRIM_UNDEFINED_TAG:
        fprintf(stream, "#undefined");
        return;
    }
}

void grim_print(grim_object obj) {
    grim_fprint(stdout, obj);
}
