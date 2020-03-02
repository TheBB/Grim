#pragma once

#include <stdint.h>

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
};


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
    };
} grim_indirect;


grim_tag grim_get_direct_tag(grim_object obj);
grim_tag grim_get_indirect_tag(grim_object obj);
