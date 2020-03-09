#include <assert.h>
#include <stdalign.h>
#include <stdio.h>

#include "gc.h"
#include "gmp.h"
#include "unistdio.h"
#include "zhash.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;
const grim_object grim_nil = GRIM_NIL_TAG;


void grim_init() {
    grim_symbol_table = zcreate_hash_table();
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
    case GRIM_SYMBOL_TAG:
        ulc_fprintf(stream, "%U", grim_get_symbol_name(obj));
        return;
    case GRIM_CHARACTER_TAG:
        grim_fprint_escape_character(stream, grim_extract_character(obj), NULL);
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_get_indirect_tag(obj)) {
        case GRIM_BIGINT_TAG:
            mpz_out_str(stream, 10, ((grim_indirect *) obj)->bigint);
            return;
        case GRIM_STRING_TAG:
            grim_fprint_escape_string(stream, obj, NULL);
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
            return;
        }
        case GRIM_CONS_TAG:
        {
            fprintf(stream, "(");
            bool needs_space = false;
            while (grim_get_type(obj) == GRIM_CONS) {
                if (needs_space)
                    fprintf(stream, " ");
                grim_fprint(stream, grim_get_car(obj));
                obj = grim_get_cdr(obj);
                needs_space = true;
            }
            if (grim_get_type(obj) != GRIM_NIL) {
                fprintf(stream, " . ");
                grim_fprint(stream, obj);
            }
            fprintf(stream, ")");
            return;
        }
        }
    default: case GRIM_UNDEFINED_TAG:
        fprintf(stream, "#undefined");
        return;
    }
}

void grim_print(grim_object obj) {
    grim_fprint(stdout, obj);
}
