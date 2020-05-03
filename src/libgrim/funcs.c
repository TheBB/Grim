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
        grim_display_string(buf, I_symbolname(src), encoding);
        return;
    case GRIM_INDIRECT_TAG:
        switch (I_tag(src)) {
        case GRIM_FLOAT_TAG:
        {
            char *z = NULL;
            int len = asprintf(&z, "%f", I_floating(src));
            grim_buffer_copy(buf, z, len);
            free(z);
            return;
        }
        case GRIM_BIGINT_TAG:
            grim_encode_mpz(buf, I_bigint(src));
            return;
        case GRIM_RATIONAL_TAG:
            grim_encode_mpz(buf, mpq_numref(I_rational(src)));
            grim_buffer_copy(buf, "/", 1);
            grim_encode_mpz(buf, mpq_denref(I_rational(src)));
            return;
        case GRIM_COMPLEX_TAG:
            grim_encode_simple(buf, I_real(src), encoding);
            if (grim_nonnegative(I_imag(src)))
                grim_buffer_copy(buf, "+", 1);
            grim_encode_simple(buf, I_imag(src), encoding);
            grim_buffer_copy(buf, "i", 1);
            return;
        case GRIM_BUFFER_TAG:
            grim_buffer_copy(buf, "#<buffer>", 9);
            return;
        case GRIM_CELL_TAG:
            grim_buffer_copy(buf, "#<cell [", 8);
            grim_encode_print(buf, I_cellvalue(src), encoding);
            grim_buffer_copy(buf, "]>", 2);
            return;
        case GRIM_MODULE_TAG:
            grim_buffer_copy(buf, "#<module [", 10);
            grim_encode_simple(buf, I_modulename(src), encoding);
            grim_buffer_copy(buf, "]>", 2);
            return;
        case GRIM_CFUNC_TAG:
        case GRIM_LFUNC_TAG:
            grim_buffer_copy(buf, "#<function>", 11);
            return;
        case GRIM_FRAME_TAG:
            grim_buffer_copy(buf, "#<frame>", 8);
            return;
        }
        return;
    }

    grim_buffer_copy(buf, "#<undefined>", 12);
}


static void grim_encode_vector(grim_object buf, grim_object src, const char *encoding, printfunc printer) {
    grim_buffer_copy(buf, "#(", 2);
    size_t len = I_vectorlen(src);
    for (size_t i = 0; i < len; i++) {
        printer(buf, I_vectordata(src)[i], encoding);
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
        printer(buf, I_car(src), encoding);
        src = I_cdr(src);
        needs_space = true;
    }
    if (src != grim_nil) {
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
        switch (I_tag(src)) {
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
        switch (I_tag(src)) {
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
    if (I_tag(a) != I_tag(b))
        return false;
    switch (I_tag(a)) {
    case GRIM_BIGINT_TAG:
        return !mpz_cmp(I_bigint(a), I_bigint(b));
    case GRIM_STRING_TAG:
        if (I_strlen(a) != I_strlen(b))
            return false;
        return !memcmp(I_str(a), I_str(b), I_strlen(a));
    }

    return a == b;
}


grim_object grim_read_file(FILE *file) {
    // TODO: Handle encoding
    grim_object buf = grim_buffer_create(0);
    while (!feof(file)) {
        grim_buffer_ensure_free_capacity(buf, 1024);
        size_t nread = fread(&I_bufend(buf), 1, 1024, file);
        I_buflen(buf) += nread;
    }

    // Slightly hacky, but buffers and strings are identical in memory
    I_tag(buf) = GRIM_STRING_TAG;
    return buf;
}
