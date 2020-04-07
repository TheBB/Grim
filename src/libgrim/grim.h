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
    GRIM_FLOAT,
    GRIM_RATIONAL,
    GRIM_COMPLEX,
    GRIM_STRING,
    GRIM_VECTOR,
    GRIM_CONS,
    GRIM_BUFFER,
    GRIM_HASHTABLE,
} grim_type_t;

grim_type_t grim_type(grim_object obj);

grim_object grim_float_pack(double num);
double grim_float_extract(grim_object obj);
grim_object grim_float_read(const char *str);

bool grim_integer_extractable(grim_object obj);
intmax_t grim_integer_extract(grim_object obj);
grim_object grim_integer_pack(intmax_t num);
grim_object grim_integer_read(const char *str, int base);

grim_object grim_rational_pack(grim_object numerator, grim_object denominator);
grim_object grim_rational_num(grim_object obj);
grim_object grim_rational_den(grim_object obj);

grim_object grim_complex_pack(grim_object real, grim_object imag);

grim_object grim_string_pack(const char *input, const char *encoding, bool escape);
size_t grim_strlen(grim_object obj);

grim_object grim_vector_create(size_t nelems);
size_t grim_vector_size(grim_object vec);
void grim_vector_set(grim_object vec, size_t index, grim_object elt);
grim_object grim_vector_get(grim_object vec, size_t index);

grim_object grim_cons_pack(grim_object car, grim_object cdr);

grim_object grim_intern(const char *name, const char *encoding);
grim_object grim_symbol_name(grim_object obj);

grim_object grim_character_pack(ucs4_t ch);
grim_object grim_character_pack_name(const char *name, const char *encoding);
ucs4_t grim_character_extract(grim_object obj);

grim_object grim_buffer_create(size_t sizehint);

grim_object grim_hashtable_create(size_t sizehint);
bool grim_hashtable_has(grim_object table, grim_object key);
grim_object grim_hashtable_get(grim_object table, grim_object key);
void grim_hashtable_set(grim_object table, grim_object key, grim_object value);
void grim_hashtable_unset(grim_object table, grim_object key);

void grim_init();
void grim_display(grim_object obj, const char *encoding);
void grim_print(grim_object obj, const char *encoding);
bool grim_equal(grim_object a, grim_object b);
bool grim_nonnegative(grim_object obj);
