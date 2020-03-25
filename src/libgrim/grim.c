#include <assert.h>
#include <stdalign.h>
#include <stdio.h>

#include "gc.h"
#include "gmp.h"
#include "unistdio.h"
#include "zhash.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


const grim_object grim_undefined = GRIM_UNDEFINED_TAG;
const grim_object grim_false = GRIM_FALSE_TAG;
const grim_object grim_true = GRIM_TRUE_TAG;
const grim_object grim_nil = GRIM_NIL_TAG;


void grim_init() {
    grim_symbol_table = zcreate_hash_table();
    assert(GRIM_ALIGN >= 16);
    GC_INIT();
}
