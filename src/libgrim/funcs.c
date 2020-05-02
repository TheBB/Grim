#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gmp.h"
#include "unistdio.h"

#include "grim.h"
#include "internal.h"


typedef void printfunc(grim_object buf, grim_object src, const char *encoding);


static void grim_encode_mpz(grim_object buf, mpz_t num) {
    char *z = NULL;
    int len = gmp_asprintf(&z, "%Zd", num);
    grim_buffer_copy(buf, z, len);
    free(z);
}

static void grim_encode_simple(grim_object buf, grim_object src, const char *encoding) {
    (void) encoding;

    switch (grim_direct_tag(src)) {
    case GRIM_FALSE_TAG:
        grim_buffer_copy(buf, "#f", 2);
        return;
    case GRIM_TRUE_TAG:
        grim_buffer_copy(buf, "#t", 2);
        return;
    case GRIM_NIL_TAG:
        grim_buffer_copy(buf, "()", 3);
        return;
    case GRIM_FIXNUM_TAG:
    {
        char *z = NULL;
        int len = asprintf(&z, "%ld", grim_integer_extract(src));
        grim_buffer_copy(buf, z, len);
        free(z);
        return;
    }
    case GRIM_SYMBOL_TAG:
        grim_display_string(buf, grim_symbol_name(src), encoding);
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_indirect_tag(src)) {
        case GRIM_FLOAT_TAG:
        {
            char *z = NULL;
            int len = asprintf(&z, "%f", grim_float_extract(src));
            grim_buffer_copy(buf, z, len);
            free(z);
            return;
        }
        case GRIM_BIGINT_TAG:
            grim_encode_mpz(buf, ((grim_indirect *) src)->bigint);
            return;
        case GRIM_RATIONAL_TAG:
            grim_encode_mpz(buf, mpq_numref(((grim_indirect *) src)->rational));
            grim_buffer_copy(buf, "/", 1);
            grim_encode_mpz(buf, mpq_denref(((grim_indirect *) src)->rational));
            return;
        case GRIM_COMPLEX_TAG:
            grim_encode_simple(buf, I(src)->real, encoding);
            if (grim_nonnegative(I(src)->imag))
                grim_buffer_copy(buf, "+", 1);
            grim_encode_simple(buf, I(src)->imag, encoding);
            grim_buffer_copy(buf, "i", 1);
            return;
        case GRIM_BUFFER_TAG:
            grim_buffer_copy(buf, "#<buffer>", 9);
            return;
        case GRIM_CELL_TAG:
            grim_buffer_copy(buf, "#<cell [", 8);
            grim_encode_print(buf, grim_cell_extract(src), encoding);
            grim_buffer_copy(buf, "]>", 2);
            return;
        case GRIM_MODULE_TAG:
            grim_buffer_copy(buf, "#<module [", 10);
            grim_encode_simple(buf, I(src)->modulename, encoding);
            grim_buffer_copy(buf, "]>", 2);
            return;
        }
        return;
    }

    grim_buffer_copy(buf, "#<undefined>", 12);
}


static void grim_encode_vector(grim_object buf, grim_object src, const char *encoding, printfunc printer) {
    grim_buffer_copy(buf, "#(", 2);
    size_t len = I(src)->vectorlen;
    for (size_t i = 0; i < len; i++) {
        printer(buf, I(src)->vectordata[i], encoding);
        if (i + 1 < len)
            grim_buffer_copy(buf, " ", 1);
    }
    grim_buffer_copy(buf, ")", 1);
}


static void grim_encode_cons(grim_object buf, grim_object src, const char *encoding, printfunc printer) {
    grim_buffer_copy(buf, "(", 1);
    bool needs_space = false;
    while (grim_type(src) == GRIM_CONS) {
        if (needs_space)
            grim_buffer_copy(buf, " ", 1);
        printer(buf, I(src)->car, encoding);
        src = I(src)->cdr;
        needs_space = true;
    }
    if (grim_type(src) != GRIM_NIL) {
        grim_buffer_copy(buf, " . ", 3);
        printer(buf, src, encoding);
    }
    grim_buffer_copy(buf, ")", 1);
}


void grim_encode_display(grim_object buf, grim_object src, const char *encoding) {
    switch (grim_direct_tag(src)) {
    case GRIM_CHARACTER_TAG:
        grim_display_character(buf, src, encoding);
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_indirect_tag(src)) {
        case GRIM_STRING_TAG:
            grim_display_string(buf, src, encoding);
            return;
        case GRIM_VECTOR_TAG:
            grim_encode_vector(buf, src, encoding, grim_encode_display);
            return;
        case GRIM_CONS_TAG:
            grim_encode_cons(buf, src, encoding, grim_encode_display);
            return;
        }
    }
    grim_encode_simple(buf, src, encoding);
}


void grim_encode_print(grim_object buf, grim_object src, const char *encoding) {
    switch (grim_direct_tag(src)) {
    case GRIM_CHARACTER_TAG:
        grim_print_character(buf, src, encoding);
        return;
    case GRIM_INDIRECT_TAG:
        switch (grim_indirect_tag(src)) {
        case GRIM_STRING_TAG:
            grim_print_string(buf, src, encoding);
            return;
        case GRIM_VECTOR_TAG:
            grim_encode_vector(buf, src, encoding, grim_encode_print);
            return;
        case GRIM_CONS_TAG:
            grim_encode_cons(buf, src, encoding, grim_encode_print);
            return;
        }
    }

    grim_encode_simple(buf, src, encoding);
}


void grim_display(grim_object obj, const char *encoding) {
    grim_object buf = grim_buffer_create(1024);
    grim_encode_display(buf, obj, encoding);
    grim_buffer_dump(stdout, buf);
}


void grim_print(grim_object obj, const char *encoding) {
    grim_object buf = grim_buffer_create(1024);
    grim_encode_print(buf, obj, encoding);
    grim_buffer_dump(stdout, buf);
}


bool grim_equal(grim_object a, grim_object b) {
    if (grim_direct_tag(a) != GRIM_INDIRECT_TAG &&
        grim_direct_tag(b) != GRIM_INDIRECT_TAG)
        return a == b;
    if (grim_indirect_tag(a) != grim_indirect_tag(b))
        return false;
    switch (grim_indirect_tag(a)) {
    case GRIM_BIGINT_TAG:
        return !mpz_cmp(((grim_indirect *)a)->bigint, ((grim_indirect *)b)->bigint);
    case GRIM_STRING_TAG:
        if (I(a)->strlen != I(b)->strlen)
            return false;
        return !memcmp(I(a)->str, I(b)->str, I(a)->strlen);
    }

    return a == b;
}
