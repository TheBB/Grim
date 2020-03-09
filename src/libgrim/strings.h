#pragma once

#include <stdio.h>

#include "grim.h"

void grim_unescape_string(grim_object str);
void grim_fprint_escape_string(FILE *stream, grim_object str, const char *encoding);
