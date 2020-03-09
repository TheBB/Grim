#pragma once

#include <stdio.h>

#include "grim.h"

ucs4_t grim_unescape_character(uint8_t *str);
void grim_unescape_string(grim_object str);
void grim_fprint_escape_character(FILE *stream, ucs4_t ch, const char *encoding);
void grim_fprint_escape_string(FILE *stream, grim_object str, const char *encoding);
