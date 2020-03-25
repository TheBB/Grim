#pragma once

#include <stdio.h>

#include "grim.h"

ucs4_t grim_unescape_character(uint8_t *str);
void grim_unescape_string(grim_object str);

void grim_display_character(grim_object buf, grim_object src, const char *encoding);
void grim_display_string(grim_object buf, grim_object src, const char *encoding);
void grim_print_character(grim_object buf, grim_object src, const char *encoding);
void grim_print_string(grim_object buf, grim_object src, const char *encoding);
