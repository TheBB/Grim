#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef uintptr_t grim_object;

extern const grim_object grim_undefined;
extern const grim_object grim_false;
extern const grim_object grim_true;
extern const grim_object grim_nil;

typedef enum {
    GRIM_INTEGER,
    GRIM_CHARACTER,
    GRIM_SYMBOL,
    GRIM_UNDEFINED,
    GRIM_BOOLEAN,
    GRIM_NIL,
    GRIM_STRING,
    GRIM_VECTOR,
    GRIM_CONS,
} grim_type;

grim_type grim_get_type(grim_object obj);

bool grim_can_extract_integer(grim_object obj);
intmax_t grim_extract_integer(grim_object obj);
grim_object grim_pack_integer(intmax_t num);

grim_object grim_pack_string(const char *input, const char *encoding);

grim_object grim_create_vector(size_t nelems);
size_t grim_vector_size(grim_object vec);
void grim_vector_set(grim_object vec, size_t index, grim_object elt);
grim_object grim_vector_get(grim_object vec, size_t index);

void grim_init();
void grim_fprint(FILE *stream, grim_object obj);
void grim_print(grim_object obj);
