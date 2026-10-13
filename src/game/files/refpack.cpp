// EA RefPack decompression: reimplementation of 0x641f10 and 0x642110.
#include "refpack.h"

namespace {

inline u32 be24(const u8 *p) { return (u32)p[0] << 16 | (u32)p[1] << 8 | p[2]; }

// Byte by byte: a match may overlap the bytes it is producing.
inline void copy(u8 *&dst, const u8 *src, u32 n) {
    while (n--) *dst++ = *src++;
}

}  // namespace

extern "C" s32 __stdcall RefPack_Decode(u8 *dst, const u8 *src, s32 /*unused*/) {
    if (!src) return 0;
    const u8 *in = src + ((src[0] & 0x01) ? 5 : 2);
    s32 size = (s32)be24(in);
    in += 3;
    for (;;) {
        u8 b0 = in[0];
        if (!(b0 & 0x80)) {  // 2 bytes: 0-3 literals, then 3-10 bytes from up to 1 KB back
            u8 b1 = in[1];
            in += 2;
            copy(dst, in, b0 & 3);
            in += b0 & 3;
            copy(dst, dst - (((b0 & 0x60) << 3) + b1 + 1), ((b0 >> 2) & 7) + 3);
        } else if (!(b0 & 0x40)) {  // 3 bytes: 0-3 literals, then 4-67 bytes from up to 16 KB back
            u8 b1 = in[1], b2 = in[2];
            in += 3;
            copy(dst, in, b1 >> 6);
            in += b1 >> 6;
            copy(dst, dst - (((b1 & 0x3f) << 8) + b2 + 1), (b0 & 0x3f) + 4);
        } else if (!(b0 & 0x20)) {  // 4 bytes: 0-3 literals, then 5-1028 bytes from up to 128 KB back
            u8 b1 = in[1], b2 = in[2], b3 = in[3];
            in += 4;
            copy(dst, in, b0 & 3);
            in += b0 & 3;
            copy(dst, dst - (((b0 & 0x10) << 12) + (b1 << 8) + b2 + 1), ((b0 & 0x0c) << 6) + b3 + 5);
        } else {
            u32 n = (b0 & 0x1f) * 4 + 4;
            ++in;
            if (n > 0x70) {  // 0xfc-0xff: 0-3 last literals, end
                copy(dst, in, b0 & 3);
                return size;
            }
            copy(dst, in, n);  // 4-112 literals
            in += n;
        }
    }
}

extern "C" u32 __stdcall RefPack_DecodedSize(const u8 *src) {
    return be24(src + (((u32)src[0] << 8 | src[1]) == 0x10fb ? 2 : 5));
}
