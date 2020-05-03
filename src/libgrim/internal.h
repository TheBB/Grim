#pragma once

#include <stddef.h>
#include <stdint.h>

#include "gmp.h"

#include "grim.h"

typedef uint8_t grim_tag_t;

#define GRIM_ALIGN alignof(max_align_t)
#define GRIM_FIXNUM_MAX (INTPTR_MAX / 2)
#define GRIM_FIXNUM_MIN (INTPTR_MIN / 2)

// Immediate types
enum {
    GRIM_INDIRECT_TAG  = 0b0000,
    GRIM_FIXNUM_TAG    = 0b0001,
    GRIM_CHARACTER_TAG = 0b0010,
    GRIM_SYMBOL_TAG    = 0b0100,
    GRIM_UNDEFINED_TAG = 0b0110,
    GRIM_FALSE_TAG     = 0b1000,
    GRIM_TRUE_TAG      = 0b1010,
    GRIM_NIL_TAG       = 0b1100,
};

// Indirect types
enum {
    GRIM_FLOAT_TAG     = 0x00,
    GRIM_BIGINT_TAG    = 0x01,
    GRIM_RATIONAL_TAG  = 0x02,
    GRIM_COMPLEX_TAG   = 0x03,
    GRIM_STRING_TAG    = 0x04,
    GRIM_VECTOR_TAG    = 0x05,
    GRIM_CONS_TAG      = 0x06,
    GRIM_BUFFER_TAG    = 0x07,
    GRIM_HASHTABLE_TAG = 0x08,
    GRIM_CELL_TAG      = 0x09,
    GRIM_MODULE_TAG    = 0x10,
    GRIM_CFUNC_TAG     = 0x11,
    GRIM_LFUNC_TAG     = 0x12,
    GRIM_FRAME_TAG     = 0x13,
};

struct grim_hashnode_t {
    grim_object key;
    grim_object value;
    uint64_t hash;
    struct grim_hashnode_t *next;
};

typedef struct grim_hashnode_t grim_hashnode;

typedef struct {
    grim_tag_t tag;
    union {
        // GRIM_FLOAT_TAG
        struct {
            double floating;
        };

        // GRIM_BIGINT_TAG
        struct {
            mpz_t bigint;
        };

        // GRIM_RATIONAL_TAG
        struct {
            mpq_t rational;
        };

        // GRIM_COMPLEX_TAG
        struct {
            grim_object real;
            grim_object imag;
        };

        // GRIM_VECTOR_TAG
        struct {
            grim_object *vectordata;
            size_t vectorlen;
        };

        // GRIM_CONS_TAG
        struct {
            grim_object car;
            grim_object cdr;
        };

        // GRIM_SYMBOL_TAG (via direct)
        struct {
            grim_object symbolname;
        };

        // GRIM_BUFFER_TAG, GRIM_STRING_TAG
        struct {
            char *buf;
            size_t buflen;
            size_t bufcap;
        };

        // GRIM_HASHTABLE_TAG
        struct {
            grim_hashnode **hashnodes;
            size_t hashcap;
            size_t hashfill;
        };

        // GRIM_CELL_TAG
        struct {
            grim_object cellvalue;
        };

        // GRIM_MODULE_TAG
        struct {
            grim_object modulename;
            grim_object modulemembers;
        };

        // GRIM_CFUNC_TAG, GRIM_LFUNC_TAG
        struct {
            union {
                grim_cfunc *cfunc;
                struct {
                    grim_object bytecode;
                    grim_object cellvars;
                    int8_t nlocals;
                };
            };
            uint8_t minargs;
            uint8_t maxargs;
            bool varargs;
        };

        // GRIM_FRAME_TAG
        struct {
            grim_object framefunc;
            grim_object frameargs;
            grim_object parentframe;
        };
    };
} grim_indirect;

#define I(c) ((grim_indirect *) (c))
#define I_tag(c) (I(c)->tag)
#define I_floating(c) (I(c)->floating)
#define I_bigint(c) (I(c)->bigint)
#define I_rational(c) (I(c)->rational)
#define I_real(c) (I(c)->real)
#define I_imag(c) (I(c)->imag)
#define I_str(c) ((uint8_t *) (I(c)->buf))
#define I_setstr(c, v) do { I(c)->buf = (char *) (v); } while (0)
#define I_strlen(c) (I(c)->buflen)
#define I_vectordata(c) (I(c)->vectordata)
#define I_vectorlen(c) (I(c)->vectorlen)
#define I_vectorelt(c, i) (I(c)->vectordata[i])
#define I_car(c) (I(c)->car)
#define I_cdr(c) (I(c)->cdr)
#define I_symbolname(c) (I((c) - GRIM_SYMBOL_TAG)->symbolname)
#define I_buf(c) (I(c)->buf)
#define I_buflen(c) (I(c)->buflen)
#define I_bufcap(c) (I(c)->bufcap)
#define I_bufend(c) (I(c)->buf[I(c)->buflen])
#define I_hashnodes(c) (I(c)->hashnodes)
#define I_hashcap(c) (I(c)->hashcap)
#define I_hashfill(c) (I(c)->hashfill)
#define I_cellvalue(c) (I(c)->cellvalue)
#define I_modulename(c) (I(c)->modulename)
#define I_modulemembers(c) (I(c)->modulemembers)
#define I_cfunc(c) (I(c)->cfunc)
#define I_bytecode(c) (I(c)->bytecode)
#define I_cellvars(c) (I(c)->cellvars)
#define I_nlocals(c) (I(c)->nlocals)
#define I_minargs(c) (I(c)->minargs)
#define I_maxargs(c) (I(c)->maxargs)
#define I_varargs(c) (I(c)->varargs)
#define I_framefunc(c) (I(c)->framefunc)
#define I_frameargs(c) (I(c)->frameargs)
#define I_parentframe(c) (I(c)->parentframe)

// Hash table mapping strings to symbols
extern grim_object grim_symbol_table;

// Module with builtin functions and variables
extern grim_object grim_builtin_module;

extern grim_object grim_top_frame;
extern grim_object gs_i_moduleset;

extern size_t grim_fixnum_max_ndigits[];

grim_object grim_indirect_create(bool permanent);

grim_tag_t grim_direct_tag(grim_object obj);

void grim_buffer_dump(FILE *stream, grim_object obj);
void grim_buffer_ensure_free_capacity(grim_object obj, size_t sizehint);
void grim_buffer_copy(grim_object obj, const char *data, size_t length);

void grim_encode_display(grim_object buf, grim_object src, const char *encoding);
void grim_encode_print(grim_object buf, grim_object src, const char *encoding);

ucs4_t grim_unescape_character(uint8_t *str, size_t length);
void grim_unescape_string(grim_object str);
int grim_peek_char(ucs4_t *retval, grim_object str, size_t offset);

void grim_display_character(grim_object buf, grim_object src, const char *encoding);
void grim_display_string(grim_object buf, grim_object src, const char *encoding);
void grim_print_character(grim_object buf, grim_object src, const char *encoding);
void grim_print_string(grim_object buf, grim_object src, const char *encoding);

uint64_t grim_hash(grim_object obj, uint64_t h);

double grim_to_double(grim_object num);
grim_object grim_negate_i(grim_object obj);
grim_object grim_scinot_pack(grim_object scale, int base, intmax_t exponent, bool exact);
bool grim_is_exact(grim_object num);
grim_object grim_add(grim_object a, grim_object b);
grim_object grim_read_file(FILE *file);

grim_object grim_eval_in_module(grim_object module, grim_object expr);
grim_object grim_build_module(grim_object name, grim_object code);

grim_object grim_call(grim_object func, int nargs, const grim_object *args);
grim_object grim_call_0(grim_object func);
grim_object grim_call_1(grim_object func, grim_object arg);
grim_object grim_call_2(grim_object func, grim_object arg1, grim_object arg2);


// Builtin functions
// -----------------------------------------------------------------------------

grim_cfunc gf_add;
