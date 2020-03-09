#include <assert.h>

#include "unistr.h"

#include "grim.h"
#include "internal.h"
#include "strings.h"


void grim_unescape_string(grim_object str) {
    uint8_t *srcptr = ((grim_indirect *)str)->str;
    uint8_t *tgtptr = srcptr;
    uint8_t ch;
    int nchars = 8;
    ucs4_t code = 0;

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
        case 0x75: nchars = 4;                // \uXXXX
        case 0x55:                            // \UXXXXXXXX
            code = 0;
            for (int i = 0; i < nchars; i++) {
                assert((ch = *(srcptr++)));
                if ('0' <= ch && ch <= '9')
                    code = 16 * code + (ch - '0');
                else if ('a' <= ch && ch <= 'f')
                    code = 16 * code + 10 + ch - 'a';
                else
                    assert(false);
            }
            tgtptr += u8_uctomb(tgtptr, code, nchars + 2);
            break;
        }

        nchars = 8;
    }

    *tgtptr = 0x00;
    ((grim_indirect *)str)->strlen = tgtptr - ((grim_indirect *)str)->str;
}
