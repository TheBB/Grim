#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "gc.h"

#include "grim.h"


typedef uint8_t grim_tag;


#define GRIM_ALIGN alignof(max_align_t)

enum {
    GRIM_INDIRECT_TAG  = 0b0000,
    GRIM_FIXNUM_TAG    = 0b0001,
    GRIM_CHARACTER_TAG = 0b0010,
    GRIM_SYMBOL_TAG    = 0b0100,
    GRIM_UNDEFINED_TAG = 0b0110,
    GRIM_FALSE_TAG     = 0b1000,
    GRIM_TRUE_TAG      = 0b1010,
};

const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;


static grim_tag grim_get_tag(grim_object obj) {
    if ((obj & GRIM_FIXNUM_TAG) != 0)
        return GRIM_FIXNUM_TAG;
    return obj & 0x0f;
}

grim_type grim_get_type(grim_object obj) {
    switch (grim_get_tag(obj)) {
    case GRIM_FIXNUM_TAG: return GRIM_INTEGER;
    case GRIM_CHARACTER_TAG: return GRIM_CHARACTER;
    case GRIM_SYMBOL_TAG: return GRIM_SYMBOL;
    case GRIM_FALSE_TAG: case GRIM_TRUE_TAG: return GRIM_BOOLEAN;
    default: return GRIM_UNDEFINED;
    }
}

intptr_t grim_extract_fixnum(grim_object obj) {
    bool signbit = ((intptr_t) obj) < 0;
    intptr_t ret = obj >> 1;
    return signbit ? (INTPTR_MIN | ret) : ret;
}

grim_object grim_pack_fixnum(intptr_t num) {
    return (grim_object) ((uintptr_t) num << 1) | GRIM_FIXNUM_TAG;
}

void grim_init() {
    assert(GRIM_ALIGN >= 16);
    GC_INIT();
}

void grim_fprint(grim_object obj, FILE *stream) {
    switch(grim_get_tag(obj)) {
    case GRIM_FALSE_TAG:
        fprintf(stream, "#f");
        break;
    case GRIM_TRUE_TAG:
        fprintf(stream, "#t");
        break;
    case GRIM_UNDEFINED_TAG:
    default:
        fprintf(stream, "#undefined");
        break;
    case GRIM_FIXNUM_TAG:
        fprintf(stream, "%ld", grim_extract_fixnum(obj));
        break;
    }
}

void grim_print(grim_object obj) {
    grim_fprint(obj, stdout);
}
