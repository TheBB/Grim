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
        grim_buffer_copy(buf, "nil", 3);
        return;
    case GRIM_FIXNUM_TAG:
    {
        char *z = NULL;
        int len = asprintf(&z, "%ld", grim_integer_extract(src));
        grim_buffer_copy(buf, z, len);
        free(z);
        return;
    }
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
            grim_encode_simple(buf, grim_complex_real(src), encoding);
            if (grim_nonnegative(grim_complex_imag(src)))
                grim_buffer_copy(buf, "+", 1);
            grim_encode_simple(buf, grim_complex_imag(src), encoding);
            return;
        case GRIM_BUFFER_TAG:
            grim_buffer_copy(buf, "#<buffer>", 9);
            return;
        }
        return;
    }

    grim_buffer_copy(buf, "#<undefined>", 12);

}

static void grim_encode_vector(grim_object buf, grim_object src, const char *encoding, printfunc printer) {
    grim_buffer_copy(buf, "#(", 2);
    size_t len = grim_vector_size(src);
    for (size_t i = 0; i < len; i++) {
        printer(buf, grim_vector_get(src, i), encoding);
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
        printer(buf, grim_car(src), encoding);
        src = grim_cdr(src);
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
    case GRIM_SYMBOL_TAG:
        grim_display_string(buf, grim_symbol_name(src), encoding);
        return;
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
    case GRIM_SYMBOL_TAG:
        grim_display_string(buf, grim_symbol_name(src), encoding);
        return;
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
        if (grim_strlen(a) != grim_strlen(b))
            return false;
        return !memcmp(grim_strptr(a), grim_strptr(b), grim_strlen(a));
    }

    return a == b;
}


bool grim_nonnegative(grim_object obj) {
    grim_type_t type = grim_type(obj);

    if (type == GRIM_INTEGER) {
        if (grim_integer_extractable(obj))
            return grim_integer_extract(obj) >= 0;
        return mpz_sgn(((grim_indirect *) obj)->bigint) >= 0;
    }

    if (type == GRIM_RATIONAL)
        return mpq_sgn(((grim_indirect *) obj)->rational) >= 0;

    if (type == GRIM_FLOAT)
        return grim_float_extract(obj) >= 0.0;

    assert(false);
}
