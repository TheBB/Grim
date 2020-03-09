#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistring/iconveh.h>

#include "unistr.h"
#include "uniconv.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


static void grim_decode_unicode(uint8_t **srcptr, uint8_t **tgtptr, int nchars) {
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


void grim_unescape_string(grim_object str) {
    uint8_t *srcptr = ((grim_indirect *)str)->str;
    uint8_t *tgtptr = srcptr;
    uint8_t ch;

    while ((ch = *(srcptr++))) {
        if (ch != 0x5c) {
            *(tgtptr++) = ch;
            continue;
        }

        assert((ch = *(srcptr++)));
        switch (ch) {
        case 0x0a: break;                     // \\n -> ignore
        case 0x22: *(tgtptr++) = 0x22; break; // \" -> quote
        case 0x5c: *(tgtptr++) = 0x5c; break; // \\ -> backslash
        case 0x62: *(tgtptr++) = 0x08; break; // \b -> backspace
        case 0x6e: *(tgtptr++) = 0x0a; break; // \n -> newline
        case 0x74: *(tgtptr++) = 0x09; break; // \t -> tab
        case 0x75:                            // \uXXXX
            grim_decode_unicode(&srcptr, &tgtptr, 4); break;
        case 0x55:                            // \UXXXXXXXX
            grim_decode_unicode(&srcptr, &tgtptr, 8); break;
        }
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
    while (start <= length) {
        end = start;

        while (end <= length) {
            uint8_t ch = buf[end];
            if (ch == 0x22 || ch == 0x5c || ch == 0x08 || ch == 0x0a || ch == 0x09)
                break;
            end++;
        }

        char *printbuf = u8_conv_to_encoding(
            encoding, iconveh_escape_sequence, buf,
            end <= length ? end - start : end - start - 1,
            NULL, NULL, &convlength
        );
        fprintf(stream, "%*s", (int) convlength, printbuf);
        free(printbuf);

        if (end > length)
            break;

        uint8_t ch = buf[end];
        switch (ch) {
        case 0x22: fprintf(stream, "\\\""); break;
        case 0x5c: fprintf(stream, "\\\\"); break;
        case 0x08: fprintf(stream, "\\b"); break;
        case 0x0a: fprintf(stream, "\\n"); break;
        case 0x09: fprintf(stream, "\\t"); break;
        default: assert(false);
        }

        start = end + 1;

        break;
    }
    fprintf(stream, "\"");
}
