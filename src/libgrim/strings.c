#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistring/iconveh.h>
#include <unitypes.h>

#include "unistr.h"
#include "uniconv.h"

#include "grim.h"
#include "internal.h"


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
    [34] = "\\\"",
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

/* Table for converting ASCII code to canonical escape sequence for
 * characters.
 */
#define N_CESCAPE (sizeof(T_CESCAPE) / sizeof(T_CESCAPE[0]))
#define HAS_CESCAPE(c) ((c) < N_CESCAPE && T_CESCAPE[(c)] != 0)
static const char * T_CESCAPE[] = {
    [0] = "\\null",
    [1] = "\\^A",
    [2] = "\\^B",
    [3] = "\\^C",
    [4] = "\\^D",
    [5] = "\\^E",
    [6] = "\\^F",
    [7] = "\\bell",
    [8] = "\\backspace",
    [9] = "\\tab",
    [10] = "\\linefeed",
    [11] = "\\verticaltab",
    [12] = "\\formfeed",
    [13] = "\\return",
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
    [27] = "\\escape",
    [28] = "\\^\\",
    [29] = "\\^]",
    [30] = "\\^^",
    [31] = "\\^_",
    [32] = "\\space",
    [92] = "\\backslash",
    [94] = "\\caret",
    [127] = "\\^?",
};

/* Table for converting word-like escape sequences to characters. */
#define N_CESCAPE_WORD (sizeof(T_CESCAPE_WORD) / sizeof(T_CESCAPE_WORD[0]))
static const struct {const char *word; ucs4_t character;} T_CESCAPE_WORD[] = {
    {"null", 0},
    {"bell", 7},
    {"backspace", 8},
    {"tab", 9},
    {"linefeed", 10},
    {"verticaltab", 11},
    {"formfeed", 12},
    {"return", 13},
    {"escape", 27},
    {"space", 32},
    {"backslash", 92},
    {"caret", 94},
};


static ucs4_t count_codepoint(uint8_t **srcptr, int nchars) {
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
    return code;
}


static void grim_unescape_unicode(uint8_t **srcptr, uint8_t **tgtptr, int nchars) {
    ucs4_t code = count_codepoint(srcptr, nchars);
    *tgtptr += u8_uctomb(*tgtptr, code, nchars + 2);
}


static void grim_unescape(uint8_t **srcptr, uint8_t **tgtptr) {
    uint8_t ch = *((*srcptr)++);
    if (HAS_SUSCAPE(ch))
        *((*tgtptr)++) = T_SUSCAPE[ch];
    else if (ch == '^') {
        ch = *((*srcptr)++);
        assert(ch);
        if ('@' <= ch && ch <= '_')
            *((*tgtptr)++) = ch - 'A' + 1;
        else {
            assert(ch == '?');
            *((*tgtptr)++) = 127;
        }
    }
    else if (ch == 'u')
        grim_unescape_unicode(srcptr, tgtptr, 4);
    else if (ch == 'U')
        grim_unescape_unicode(srcptr, tgtptr, 8);
    else
        assert(false);
}


ucs4_t grim_unescape_character(uint8_t *str, size_t length) {
    assert(length > 0);
    uint8_t ch = str[0];
    if (ch == '^') {
        if (length == 1)
            return '^';
        ch = str[1];
        if ('@' <= ch && ch <= '_')
            return ch - 'A' + 1;
        assert(ch == '?');
        return 127;
    }
    if (ch == '\\')
        return 92;
    for (size_t i = 0; i < N_CESCAPE_WORD; i++) {
        if (length < strlen(T_CESCAPE_WORD[i].word))
            continue;
        if (!u8_strncmp(str, (const uint8_t *)T_CESCAPE_WORD[i].word, length))
            return T_CESCAPE_WORD[i].character;
    }
    if (ch == 'u' || ch == 'U') {
        assert(length > ((ch == 'u') ? 4 : 8));
        uint8_t *srcptr = str + 1;
        return count_codepoint(&srcptr, (ch == 'u') ? 4 : 8);
    }
    assert(u8_mblen(str, length) == (int) length);
    ucs4_t retval;
    u8_mbtouc(&retval, str, 1);
    return retval;
}


void grim_unescape_string(grim_object str) {
    uint8_t *srcptr = I_str(str);
    uint8_t *endptr = srcptr + I_strlen(str);
    uint8_t *tgtptr = srcptr;
    uint8_t ch;

    while (srcptr < endptr) {
        ch = *(srcptr++);
        if (ch != '\\')
            *(tgtptr++) = ch;
        else
            grim_unescape(&srcptr, &tgtptr);
    }

    I_strlen(str) = tgtptr - I_str(str);
}


void grim_display_character(grim_object buf, grim_object src, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    uint8_t workspace[6];
    int len = u8_uctomb(workspace, grim_character_extract(src), 6);
    size_t convlength;
    char *encoded = u8_conv_to_encoding(encoding, iconveh_question_mark, workspace, len, NULL, NULL, &convlength);
    grim_buffer_copy(buf, encoded, convlength);
    free(encoded);
}


void grim_display_string(grim_object buf, grim_object src, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    size_t convlength;
    char *encoded = u8_conv_to_encoding(
        encoding,
        iconveh_question_mark,
        I(src)->str,
        I(src)->strlen,
        NULL, NULL,
        &convlength
    );
    grim_buffer_copy(buf, encoded, convlength);
    free(encoded);
}


void grim_print_character(grim_object buf, grim_object src, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    ucs4_t ch = grim_character_extract(src);
    if (HAS_CESCAPE(ch)) {
        grim_buffer_copy(buf, "#", 1);
        grim_buffer_copy(buf, T_CESCAPE[ch], strlen(T_CESCAPE[ch]));
        return;
    }

    grim_buffer_copy(buf, "#\\", 2);
    uint8_t workspace[6];
    int len = u8_uctomb(workspace, ch, 6);
    size_t convlength;
    char *encoded = u8_conv_to_encoding(encoding, iconveh_escape_sequence, workspace, len, NULL, NULL, &convlength);
    grim_buffer_copy(buf, encoded, convlength);
    free(encoded);
}


void grim_print_string(grim_object buf, grim_object src, const char *encoding) {
    if (!encoding)
        encoding = locale_charset();
    uint8_t *buffer = I_str(src);
    size_t start = 0, end, length = I_strlen(src), convlength;

    grim_buffer_copy(buf, "\"", 1);
    while (start < length) {
        end = start;

        while (end < length) {
            uint8_t ch = buffer[end];
            if (HAS_SESCAPE(ch))
                break;
            end++;
        }

        char *printbuf = u8_conv_to_encoding(
            encoding, iconveh_escape_sequence, buffer + start,
            end - start, NULL, NULL, &convlength
        );
        grim_buffer_copy(buf, printbuf, convlength);
        free(printbuf);

        if (end >= length)
            break;

        uint8_t ch = buffer[end];
        assert(HAS_SESCAPE(ch));
        grim_buffer_copy(buf, T_SESCAPE[ch], strlen(T_SESCAPE[ch]));

        start = end + 1;
    }
    grim_buffer_copy(buf, "\"", 1);
}

int grim_peek_char(ucs4_t *retval, grim_object str, size_t offset) {
    size_t len = I_strlen(str);
    if (offset >= len)
        return -1;
    return u8_mbtouc(retval, I_str(str) + offset, len - offset);
}
