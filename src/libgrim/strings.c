#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistring/iconveh.h>

#include "unistr.h"
#include "uniconv.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


/* Table for converting ASCII code to canonical escape sequence. */
#define N_SESCAPE (sizeof(T_SESCAPE) / sizeof(T_SESCAPE[0]))
#define HAS_SESCAPE(c) ((c) < N_SESCAPE && T_SESCAPE[(c)] != 0)
static const char *T_SESCAPE[] = {
    [0] = "\\0",
    [1] = "\\^A",
    [2] = "\\^B",
    [3] = "\\^C",
    [4] = "\\^D",
    [5] = "\\^E",
    [6] = "\\^F",
    [7] = "\\a",
    [8] = "\\b",
    [9] = "\\t",
    [10] = "\\n",
    [11] = "\\v",
    [12] = "\\f",
    [13] = "\\r",
    [14] = "\\^N",
    [15] = "\\^O",
    [16] = "\\^P",
    [17] = "\\^Q",
    [18] = "\\^R",
    [19] = "\\^S",
    [20] = "\\^T",
    [21] = "\\^U",
    [22] = "\\^V",
    [23] = "\\^W",
    [24] = "\\^X",
    [25] = "\\^Y",
    [26] = "\\^Z",
    [27] = "\\e",
    [28] = "\\^\\",
    [29] = "\\^]",
    [30] = "\\^^",
    [31] = "\\^_",
    [34] = "\"",
    [92] = "\\\\",
    [127] = "\\^?",
};

/* Table for converting single-character escape sequences to
 * corresponding ASCII character.
 */
#define N_SUSCAPE (sizeof(T_SUSCAPE) / sizeof(T_SUSCAPE[0]))
#define HAS_SUSCAPE(c) ((c == '0') || ((c) < N_SUSCAPE && T_SUSCAPE[(c)] != 0))
static const uint8_t T_SUSCAPE[] = {
    ['0'] = 0,
    ['a'] = 7,
    ['b'] = 8,
    ['t'] = 9,
    ['n'] = 10,
    ['v'] = 11,
    ['f'] = 12,
    ['r'] = 13,
    ['e'] = 27,
    ['"'] = 34,
    ['\\'] = 92,
};

/* Table for converting control character escape codes.
 * The standard capital letters A-Z are not listed here.
 */
#define N_SUSCAPE_CTRL (sizeof(T_SUSCAPE_CTRL) / sizeof(T_SUSCAPE_CTRL[0]))
#define HAS_SUSCAPE_CTRL(c) ((c) < N_SUSCAPE_CTRL && T_SUSCAPE_CTRL[(c)] != 0)
static const uint8_t T_SUSCAPE_CTRL[] = {
    ['['] = 27,
    ['\\'] = 28,
    [']'] = 29,
    ['^'] = 30,
    ['_'] = 31,
    ['?'] = 127,
};


static void grim_unescape_unicode(uint8_t **srcptr, uint8_t **tgtptr, int nchars) {
    ucs4_t code = 0;
    uint8_t ch;
    for (int i = 0; i < nchars; i++) {
        assert((ch = *((*srcptr)++)));
        if ('0' <= ch && ch <= '9')
            code = 16 * code + (ch - '0');
        else if ('A' <= ch && ch <= 'F')
            code = 16 * code + 10 + ch - 'A';
        else if ('a' <= ch && ch <= 'f')
            code = 16 * code + 10 + ch - 'a';
        else
            assert(false);
    }
    *tgtptr += u8_uctomb(*tgtptr, code, nchars + 2);
}


static void grim_unescape(uint8_t **srcptr, uint8_t **tgtptr) {
    uint8_t ch = *((*srcptr)++);
    if (HAS_SUSCAPE(ch)) {
        *((*tgtptr)++) = T_SUSCAPE[ch];
        return;
    }

    if (ch == '^') {
        ch = *((*srcptr)++);
        assert(ch);
        if ('A' <= ch && ch <= 'Z')
            *((*tgtptr)++) = ch - 'A' + 1;
        assert(HAS_SUSCAPE_CTRL(ch));
        *((*tgtptr)++) = T_SUSCAPE_CTRL[ch];
    }
    else if (ch == 'u')
        grim_unescape_unicode(srcptr, tgtptr, 4);
    else if (ch == 'U')
        grim_unescape_unicode(srcptr, tgtptr, 8);
    else
        assert(false);
}


void grim_unescape_string(grim_object str) {
    uint8_t *srcptr = ((grim_indirect *)str)->str;
    uint8_t *endptr = srcptr + ((grim_indirect *)str)->strlen;
    uint8_t *tgtptr = srcptr;
    uint8_t ch;

    while (srcptr < endptr) {
        ch = *(srcptr++);
        if (ch != 0x5c)
            *(tgtptr++) = ch;
        else
            grim_unescape(&srcptr, &tgtptr);
    }

    *tgtptr = 0x00;
    ((grim_indirect *)str)->strlen = tgtptr - ((grim_indirect *)str)->str;
}


void grim_fprint_escape_string(FILE *stream, grim_object str, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    grim_indirect *ind = (grim_indirect *)str;
    uint8_t *buf = ind->str;
    size_t start = 0, end, length = ind->strlen, convlength;

    fprintf(stream, "\"");
    while (start < length) {
        end = start;

        while (end < length) {
            uint8_t ch = buf[end];
            if (HAS_SESCAPE(ch))
                break;
            end++;
        }

        char *printbuf = u8_conv_to_encoding(
            encoding, iconveh_escape_sequence, buf,
            end - start, NULL, NULL, &convlength
        );
        fprintf(stream, "%*s", (int) convlength, printbuf);
        free(printbuf);

        if (end >= length)
            break;

        uint8_t ch = buf[end];
        assert(HAS_SESCAPE(ch));
        fprintf(stream, "%s", T_SESCAPE[ch]);

        start = end + 1;
    }
    fprintf(stream, "\"");
}
