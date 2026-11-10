#include "base64.h"
#include "../online/postcardmail.h"

namespace {

const char *Table() { return gPostcardmail_773d80.base64; }

}  // namespace

extern "C" {

void __cdecl Base64_Encode(char *dst, const u8 *src, s32 length) {
    if (!dst || !src)
        return;
    const char *table = Table();
    u32 column = 0;
    while (length > 0) {
        u32 bits = (u32)*src++ << 8;
        if (length > 1)
            bits |= *src++;
        bits <<= 8;
        if (length > 2)
            bits |= *src++;
        *dst++ = table[bits >> 18];
        bits &= 0x3ffff;
        *dst++ = table[bits >> 12];
        bits &= 0xfff;
        if (length > 1) {
            *dst++ = table[bits >> 6];
            bits &= 0x3f;
            *dst++ = length > 2 ? table[bits] : '=';
        } else {
            *dst++ = '=';
            *dst++ = '=';
        }
        column += 4;
        length -= 3;
        if (column == 0x4c) {
            *dst++ = '\r';
            *dst++ = '\n';
            column = 0;
        }
    }
    *dst = 0;
}
}
