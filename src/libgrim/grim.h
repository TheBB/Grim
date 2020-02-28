#include <stdint.h>
#include <stdio.h>

#pragma once

typedef uintptr_t grim_object;

extern const grim_object grim_undefined;
extern const grim_object grim_false;
extern const grim_object grim_true;

typedef enum {
    GRIM_INTEGER,
    GRIM_CHARACTER,
    GRIM_SYMBOL,
    GRIM_UNDEFINED,
    GRIM_BOOLEAN,
    GRIM_STRING,
    GRIM_VECTOR,
    GRIM_CONS,
} grim_type;

grim_type grim_get_type(grim_object obj);
intptr_t grim_extract_integer(grim_object obj);
grim_object grim_pack_integer(intptr_t num);

void grim_init();
void grim_fprint(grim_object obj, FILE *stream);
void grim_print(grim_object obj);
