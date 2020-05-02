#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#include "grim.h"

typedef uint8_t grim_tag_t;

#define GRIM_ALIGN alignof(max_align_t)
#define GRIM_FIXNUM_MAX (INTPTR_MAX / 2)
#define GRIM_FIXNUM_MIN (INTPTR_MIN / 2)

// Immediate types
enum {
    GRIM_INDIRECT_TAG  = 0b0000,
    GRIM_FIXNUM_TAG    = 0b0001,
    GRIM_CHARACTER_TAG = 0b0010,
    GRIM_SYMBOL_TAG    = 0b0100,
    GRIM_UNDEFINED_TAG = 0b0110,
    GRIM_FALSE_TAG     = 0b1000,
    GRIM_TRUE_TAG      = 0b1010,
    GRIM_NIL_TAG       = 0b1100,
};

// Indirect types
enum {
    GRIM_FLOAT_TAG     = 0x00,
    GRIM_BIGINT_TAG    = 0x01,
    GRIM_RATIONAL_TAG  = 0x02,
    GRIM_COMPLEX_TAG   = 0x03,
    GRIM_STRING_TAG    = 0x04,
    GRIM_VECTOR_TAG    = 0x05,
    GRIM_CONS_TAG      = 0x06,
    GRIM_BUFFER_TAG    = 0x07,
    GRIM_HASHTABLE_TAG = 0x08,
    GRIM_CELL_TAG      = 0x09,
    GRIM_MODULE_TAG    = 0x10,
};

struct grim_hashnode_t {
    grim_object key;
    grim_object value;
    uint64_t hash;
    struct grim_hashnode_t *next;
};

typedef struct grim_hashnode_t grim_hashnode;

typedef struct {
    grim_tag_t tag;
    union {
        // GRIM_FLOAT_TAG
        struct {
            double floating;
        };

        // GRIM_BIGINT_TAG
        struct {
            mpz_t bigint;
        };

        // GRIM_RATIONAL_TAG
        struct {
            mpq_t rational;
        };

        // GRIM_COMPLEX_TAG
        struct {
            grim_object real;
            grim_object imag;
        };

        // GRIM_STRING_TAG
        struct {
            uint8_t *str;
            size_t strlen;
        };

        // GRIM_VECTOR_TAG
        struct {
            grim_object *vectordata;
            size_t vectorlen;
        };

        // GRIM_CONS_TAG
        struct {
            grim_object car;
            grim_object cdr;
        };

        // GRIM_SYMBOL_TAG (via direct)
        struct {
            grim_object symbolname;
        };

        // GRIM_BUFFER_TAG
        struct {
            char *buf;
            size_t buflen;
            size_t bufcap;
        };

        // GRIM_HASHTABLE_TAG
        struct {
            grim_hashnode **hashnodes;
            size_t hashcap;
            size_t hashfill;
        };

        // GRIM_CELL_TAG
        struct {
            grim_object cellvalue;
        };

        // GRIM_MODULE_TAG
        struct {
            grim_object modulename;
            grim_object modulemembers;
        };
    };
} grim_indirect;

#define I(c) ((grim_indirect *) c)

extern grim_object grim_symbol_table;
extern grim_object gs_i_moduleset;

extern size_t grim_fixnum_max_ndigits[];

grim_indirect *grim_indirect_create(bool permanent);

grim_tag_t grim_direct_tag(grim_object obj);
grim_tag_t grim_indirect_tag(grim_object obj);

void grim_buffer_dump(FILE *stream, grim_object obj);
void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint);
void grim_buffer_copy(grim_object obj, const char *data, size_t length);

void grim_encode_display(grim_object buf, grim_object src, const char *encoding);
void grim_encode_print(grim_object buf, grim_object src, const char *encoding);

ucs4_t grim_unescape_character(uint8_t *str, size_t length);
void grim_unescape_string(grim_object str);
int grim_peek_char(ucs4_t *retval, grim_object str, size_t offset);

void grim_display_character(grim_object buf, grim_object src, const char *encoding);
void grim_display_string(grim_object buf, grim_object src, const char *encoding);
void grim_print_character(grim_object buf, grim_object src, const char *encoding);
void grim_print_string(grim_object buf, grim_object src, const char *encoding);

uint64_t grim_hash(grim_object obj, uint64_t h);

double grim_to_double(grim_object num);
grim_object grim_negate_i(grim_object obj);
grim_object grim_scinot_pack(grim_object scale, int base, intmax_t exponent, bool exact);
bool grim_is_exact(grim_object num);

grim_object grim_eval_in_module(grim_object module, grim_object expr);
grim_object grim_build_module(grim_object name, grim_object code);
