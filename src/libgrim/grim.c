#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdalign.h>
#include <stdio.h>
#include <string.h>

#include "gc.h"
#include "gmp.h"
#include "unistdio.h"

#include "grim.h"
#include "internal.h"


const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;
const grim_object grim_nil = GRIM_NIL_TAG;

grim_object gs_i_moduleset;


#define NBUF 300
static char buf[NBUF];

void grim_init() {
    assert(GRIM_ALIGN >= 16);
    grim_symbol_table = grim_hashtable_create(0);

    gs_i_moduleset = grim_intern("%module-set!", NULL);

    grim_fixnum_max_ndigits[2] = sizeof(intmax_t) * CHAR_BIT - 1;
    grim_fixnum_max_ndigits[8] = ceil((sizeof(intmax_t) * CHAR_BIT - 1) / 3.0) - 1;
    grim_fixnum_max_ndigits[16] = ceil((sizeof(intmax_t) * CHAR_BIT - 1) / 4.0) - 1;

    snprintf(buf, NBUF, "%ju", GRIM_FIXNUM_MAX);
    grim_fixnum_max_ndigits[10] = strlen(buf) - 1;

    GC_INIT();
}
