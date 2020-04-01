#pragma once

#include <stddef.h>
#include <stdint.h>

#include "zhash.h"
#include "gmp.h"

#include "grim.h"

typedef uint8_t grim_tag;

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

struct grim_hashnode_ {
    grim_object key;
    grim_object value;
    uint64_t hash;
    struct grim_hashnode_ *next;
};

typedef struct grim_hashnode_ grim_hashnode;

typedef struct {
    grim_tag tag;
    union {
        mpz_t bigint;
        struct {
            uint8_t *str;
            size_t strlen;
        };
        struct {
            grim_object *vector_data;
            size_t vectorlen;
        };
        struct {
            grim_object car;
            grim_object cdr;
        };
        struct {
            grim_object symbolname;
        };
        struct {
            char *buf;
            size_t buflen;
            size_t bufcap;
        };
        struct {
            grim_hashnode **hashnodes;
            size_t hashcap;
            size_t hashfill;
        };
    };
} grim_indirect;

extern grim_object grim_symbol_table;

grim_tag grim_get_direct_tag(grim_object obj);
grim_tag grim_get_indirect_tag(grim_object obj);

uint8_t *grim_get_strptr(grim_object obj);
size_t grim_set_strlen(grim_object obj, size_t length);

void grim_dump_buffer(FILE *stream, grim_object obj);
void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint);
void grim_buffer_copy(grim_object obj, const char *data, size_t length);
char *grim_get_bufptr(grim_object obj);
size_t grim_get_buflen(grim_object obj);

void grim_encode_display(grim_object buf, grim_object src, const char *encoding);
void grim_encode_print(grim_object buf, grim_object src, const char *encoding);

ucs4_t grim_unescape_character(uint8_t *str, size_t length);
void grim_unescape_string(grim_object str);

void grim_display_character(grim_object buf, grim_object src, const char *encoding);
void grim_display_string(grim_object buf, grim_object src, const char *encoding);
void grim_print_character(grim_object buf, grim_object src, const char *encoding);
void grim_print_string(grim_object buf, grim_object src, const char *encoding);

uint64_t grim_hash(grim_object obj, uint64_t h);
