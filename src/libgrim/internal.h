#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#include "grim.h"

typedef uint8_t grim_tag_t;

#define GRIM_ALIGN alignof(max_align_t)
#define GRIM_FIXNUM_MAX (INTPTR_MAX / 2)
#define GRIM_FIXNUM_MIN (INTPTR_MIN / 2)

enum {
    // Immediate types
    GRIM_INDIRECT_TAG  = 0b0000,
    GRIM_FIXNUM_TAG    = 0b0001,
    GRIM_CHARACTER_TAG = 0b0010,
    GRIM_SYMBOL_TAG    = 0b0100,
    GRIM_UNDEFINED_TAG = 0b0110,
    GRIM_FALSE_TAG     = 0b1000,
    GRIM_TRUE_TAG      = 0b1010,
    GRIM_NIL_TAG       = 0b1100,

    // Indirect types
    GRIM_BIGINT_TAG    = 0x0f,
    GRIM_STRING_TAG    = 0x1f,
    GRIM_VECTOR_TAG    = 0x2f,
    GRIM_CONS_TAG      = 0x3f,
    GRIM_BUFFER_TAG    = 0x4f,
    GRIM_HASHTABLE_TAG = 0x5f,
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
        // GRIM_BIGINT_TAG
        struct {
            mpz_t bigint;
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
    };
} grim_indirect;

extern grim_object grim_symbol_table;

grim_tag_t grim_direct_tag(grim_object obj);
grim_tag_t grim_indirect_tag(grim_object obj);

uint8_t *grim_strptr(grim_object obj);
size_t grim_set_strlen(grim_object obj, size_t length);

void grim_buffer_dump(FILE *stream, grim_object obj);
void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint);
void grim_buffer_copy(grim_object obj, const char *data, size_t length);
char *grim_bufptr(grim_object obj);
size_t grim_buflen(grim_object obj);

void grim_encode_display(grim_object buf, grim_object src, const char *encoding);
void grim_encode_print(grim_object buf, grim_object src, const char *encoding);

ucs4_t grim_unescape_character(uint8_t *str, size_t length);
void grim_unescape_string(grim_object str);

void grim_display_character(grim_object buf, grim_object src, const char *encoding);
void grim_display_string(grim_object buf, grim_object src, const char *encoding);
void grim_print_character(grim_object buf, grim_object src, const char *encoding);
void grim_print_string(grim_object buf, grim_object src, const char *encoding);

uint64_t grim_hash(grim_object obj, uint64_t h);
