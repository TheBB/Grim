#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "unitypes.h"

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
    GRIM_BUFFER,
} grim_type;

grim_type grim_get_type(grim_object obj);

bool grim_can_extract_integer(grim_object obj);
intmax_t grim_extract_integer(grim_object obj);
grim_object grim_pack_integer(intmax_t num);

grim_object grim_pack_string(const char *input, const char *encoding);
grim_object grim_pack_string_escape(const char *input, const char *encoding);
size_t grim_get_strlen(grim_object obj);

grim_object grim_create_vector(size_t nelems);
size_t grim_vector_size(grim_object vec);
void grim_vector_set(grim_object vec, size_t index, grim_object elt);
grim_object grim_vector_get(grim_object vec, size_t index);

grim_object grim_create_cons(grim_object car, grim_object cdr);
grim_object grim_get_car(grim_object obj);
grim_object grim_get_cdr(grim_object obj);

grim_object grim_intern(const char *name, const char *encoding);
uint8_t *grim_get_symbol_name(grim_object obj);

grim_object grim_pack_character(ucs4_t ch);
grim_object grim_pack_character_name(const char *name, const char *encoding);
ucs4_t grim_extract_character(grim_object obj);

grim_object grim_create_buffer(size_t sizehint);

void grim_init();
void grim_display(grim_object obj, const char *encoding);
void grim_print(grim_object obj, const char *encoding);
bool grim_eq(grim_object a, grim_object b);
