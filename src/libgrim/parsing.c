#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "grim.h"
#include "internal.h"

#include "unitypes.h"


typedef struct {
    grim_object str;
    size_t offset;
    int next_size;
} str_iter;

typedef bool charpred(ucs4_t ch, int param);
typedef bool parsefunc(void *out, str_iter *iter, int *params);

#define BASE 0
#define EXACTNESS 1
#define INDETERMINATE -1
#define INEXACT 0
#define EXACT 1


// Source code inspection
// -----------------------------------------------------------------------------

static char *substring(str_iter *iter, size_t start) {
    size_t len = iter->offset - start;
    char *code = malloc((len + 1) * sizeof(char));
    memcpy(code, I(iter->str)->str + start, len);
    code[len] = 0;
    return code;
}

static inline bool done(str_iter *iter) {
    return iter->offset >= I(iter->str)->strlen;
}

static inline ucs4_t unsafe_peek(str_iter *iter) {
    ucs4_t retval;
    iter->next_size = grim_peek_char(&retval, iter->str, iter->offset);
    return retval;
}

static inline void unsafe_advance(str_iter *iter) {
    if (iter->next_size != 0)
        unsafe_peek(iter);
    iter->offset += iter->next_size;
    iter->next_size = 0;
}

static inline ucs4_t unsafe_next(str_iter *iter) {
    ucs4_t retval = unsafe_peek(iter);
    unsafe_advance(iter);
    return retval;
}

#define assert_not_done(iter) do { if (done(iter)) return false; } while (0)

// Warning! Statement expressions
#define safe_peek(iter) ({ assert_not_done(iter); unsafe_peek(iter); })
#define safe_next(iter) ({ assert_not_done(iter); unsafe_next(iter); })


// Utility functions
// -----------------------------------------------------------------------------

static int pr_10[] = {10};

static bool is_whitespace(ucs4_t ch, int _) {
    (void)_;
    return ch == 9 || ch == 10 || ch == 12 || ch == 13 || ch == 32;
}

static bool is_digit(ucs4_t ch, int base) {
    if (ch == '_')
        return true;
    switch (base) {
    case 2: return ch == '0' || ch == '1';
    case 8: return ch >= '0' && ch <= '7';
    case 16: return (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f');
    default: return ch >= '0' && ch <= '9';
    }
}

static bool is_hash(ucs4_t ch, int _) {
    (void)_;
    return ch == '#' || ch == '_';
}

static size_t consume_while(str_iter *iter, charpred pred, int param) {
    size_t retval = 0;
    while (!done(iter)) {
        ucs4_t ch = unsafe_peek(iter);
        if (!pred(ch, param))
            return retval;
        unsafe_advance(iter);
        retval++;
    }
    return retval;
}

static void read_uint(grim_object *out, str_iter *iter, size_t start, int base) {
    char *code = substring(iter, start);
    *out = grim_integer_read(code, base);
    free(code);
}

static void read_float(grim_object *out, str_iter *iter, size_t start) {
    char *code = substring(iter, start);
    *out = grim_float_read(code);
    free(code);
}


// Combinators
// -----------------------------------------------------------------------------

static bool try(void *out, str_iter *iter, parsefunc parser, int *params) {
    size_t offset = iter->offset;
    bool retval = parser(out, iter, params);
    if (!retval) {
        iter->offset = offset;
        iter->next_size = 0;
    }
    return retval;
}


// Parsers
// -----------------------------------------------------------------------------

static bool parse_uint(void *out, str_iter *iter, int *base) {
    size_t start = iter->offset;
    if (consume_while(iter, is_digit, *base) == 0)
        return false;
    consume_while(iter, is_hash, 0);
    read_uint(out, iter, start, *base);
    return true;
}

static bool parse_exponent(void *_out, str_iter *iter, int *_) {
    (void)_;
    intmax_t *out = (intmax_t *) _out;

    if (safe_next(iter) != 'e')
        return false;

    ucs4_t ch = safe_peek(iter);
    if (ch == '+' || ch == '-')
        unsafe_advance(iter);

    grim_object temp;
    int params[] = {10};
    if (!parse_uint(&temp, iter, params))
        return false;
    assert(grim_direct_tag(temp) == GRIM_FIXNUM_TAG);
    *out = grim_integer_extract(temp);

    if (ch == '-')
        *out = -(*out);
    return true;
}

static bool parse_decimal_1(void *_out, str_iter *iter, int *exact) {
    grim_object scale, *out = (grim_object *) _out;
    intmax_t exp;
    if (!try(&scale, iter, parse_uint, pr_10))
        return false;
    if (!try(&exp, iter, parse_exponent, NULL))
        return false;
    *out = grim_scinot_pack(scale, 10, exp, *exact);
    return true;
}

static bool parse_decimal_2(void *out, str_iter *iter, int *exact) {
    size_t start = iter->offset;
    bool digits_before = consume_while(iter, is_digit, 10) > 0;
    bool pounds_before = consume_while(iter, is_hash, 0) > 0;

    if (safe_next(iter) != '.')
        return false;

    size_t fract = iter->offset;
    size_t ndigits_after = consume_while(iter, is_digit, 10);
    size_t npounds_after = consume_while(iter, is_hash, 0);
    size_t end = iter->offset;

    if (!digits_before && ndigits_after == 0)
        return false;
    if (pounds_before && ndigits_after > 0)
        return false;

    char *code = substring(iter, start);
    grim_object scale = grim_integer_read(code, 10);
    free(code);

    intmax_t exp = 0;
    try(&exp, iter, parse_exponent, 0);

    if (*exact != EXACT) {
        read_float((grim_object *) out, iter, start);
        return true;
    }

    exp -= ndigits_after + npounds_after;
    for (size_t i = fract; i < end; i++)
        if (I(iter->str)->str[i] == '_')
            exp++;

    *((grim_object *) out) = grim_scinot_pack(scale, 10, exp, *exact == EXACT);
    return true;
}

static bool parse_fraction(void *out, str_iter *iter, int* base) {
    grim_object num, den;
    if (!try(&num, iter, parse_uint, base))
        return false;
    if (safe_next(iter) != '/')
        return false;
    if (!try(&den, iter, parse_uint, base))
        return false;
    *((grim_object *) out) = grim_rational_pack(num, den);
    return true;
}

// Params: base, exactness
static bool parse_ureal(void *out, str_iter *iter, int* params) {
    if (params[BASE] == 10) {
        if (try(out, iter, parse_decimal_1, &params[EXACTNESS]))
            return true;
        if (try(out, iter, parse_decimal_2, &params[EXACTNESS]))
            return true;
    }
    if (try(out, iter, parse_fraction, &params[BASE]))
        return true;
    if (try(out, iter, parse_uint, &params[BASE]))
        return true;
    return false;
}

// Params: base, exactness
static bool parse_real(void *_out, str_iter *iter, int *params) {
    ucs4_t ch = safe_peek(iter);
    if (ch == '-' || ch == '+')
        unsafe_advance(iter);
    if (!try(_out, iter, parse_ureal, params))
        return false;
    if (ch == '-') {
        grim_object *out = (grim_object *) _out;
        *out = grim_negate_i(*out);
    }
    return true;
}

// Params: base, exactness
static bool parse_uimag(void *_out, str_iter *iter, int *params) {
    grim_object *out = (grim_object *) _out;
    if (safe_peek(iter) == 'i') {
        unsafe_advance(iter);
        *out = params[EXACTNESS] == INEXACT ? grim_float_pack(1.0) : grim_integer_pack(1);
        return true;
    }
    grim_object real;
    if (!try (&real, iter, parse_ureal, params))
        return false;
    if (safe_next(iter) != 'i')
        return false;
    *((grim_object *)_out) = real;
    return true;
}

// Params: base, exactness
static bool parse_pure_imag_part(void *_out, str_iter *iter, int *params) {
    ucs4_t ch = safe_next(iter);
    if (ch != '+' && ch != '-')
        return false;
    grim_object *out = (grim_object *) _out;
    if (!try(out, iter, parse_uimag, params))
        return false;
    if (ch == '-') {
        grim_object *out = (grim_object *) _out;
        *out = grim_negate_i(*out);
    }
    return true;
}

// Params: base, exactness
static bool parse_complex(void *_out, str_iter *iter, int *params) {
    grim_object real, imag, *out = (grim_object *) _out;
    if (try(&imag, iter, parse_pure_imag_part, params)) {
        bool exact = (params[EXACTNESS] == EXACT) ||
                     (params[EXACTNESS] == INDETERMINATE && grim_is_exact(imag));
        *out = exact ? grim_complex_pack(grim_integer_pack(0), imag)
                     : grim_complex_pack(grim_float_pack(0.0), imag);
        return true;
    }
    if (!try (&real, iter, parse_real, params))
        return false;
    if (done(iter)) {
        *out = real;
        return true;
    }
    ucs4_t mode = safe_next(iter);
    if (mode != '@' && mode != '+' && mode != '-') {
        *out = real;
        return true;
    }
    if (mode == '@' && !try(&imag, iter, parse_real, params))
        return false;
    else if (mode != '@' && !try(&imag, iter, parse_uimag, params))
        return false;

    if (mode == '-')
        imag = grim_negate_i(imag);
    if (mode != '@') {
        *out = grim_complex_pack(real, imag);
        return true;
    }

    // TODO: Handle '@'
    return false;
}

static bool parse_number(void *out, str_iter *iter, int *_) {
    (void)_;
    int params[2] = { [BASE] = 10, [EXACTNESS] = INDETERMINATE };
    for (int i = 0; i < 2; i++) {
        ucs4_t ch = safe_peek(iter);
        if (ch != '#')
            break;
        ch = safe_next(iter);
        if (ch == 'i')
            params[EXACTNESS] = INEXACT;
        else if (ch == 'e')
            params[EXACTNESS] = EXACT;
        else if (ch == 'b')
            params[BASE] = 2;
        else if (ch == 'o')
            params[BASE] = 8;
        else if (ch == 'd')
            params[BASE] = 10;
        else if (ch == 'x')
            params[BASE] = 16;
        else
            return false;
    }
    return parse_complex(out, iter, params);
}


// Main parsing functions
// -----------------------------------------------------------------------------

static grim_object read_exp(str_iter *iter) {
    consume_while(iter, is_whitespace, 0);
    grim_object obj;
    int params[] = {10, 0};
    if (parse_number(&obj, iter, params))
        return obj;
    return grim_undefined;
}


grim_object grim_read(grim_object str) {
    str_iter iter = {str, 0, 0};
    return read_exp(&iter);
}
