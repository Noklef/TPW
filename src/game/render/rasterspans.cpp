#include "rasterspans.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T *at(u32 address) { return reinterpret_cast<T *>((uintptr_t)address); }

// A triangle edge as the set-ups leave it (the right edge's x and dx only are read): x 17.15 fixed, u and v, four shade words, depth.
struct Edge {
    u32 x, dx;
    u32 uv[2], duv[2];
    u16 shade[4], dshade[4];
    u32 z, dz;
};

s16 MulHigh(u16 a, u16 b) { return (s16)(((s32)(s16)a * (s16)b) >> 16); }  // PMULHW

// The depth with its halves swapped (the fraction high), stepped with the carry out of the fraction added in (ADD, ADC 0).
u32 StepDepth(u32 z, u32 step) {
    const u32 sum = z + step;
    return sum + (sum < z ? 1u : 0u);
}

// The texel's four bytes times the shade words (PMULHW of 2 * byte), each saturated to a byte (PACKUSWB).
u32 Shade(u32 texel, const u16 *shade) {
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const s16 w = MulHigh((u16)((texel >> (8 * k) & 0xff) * 2), shade[k]);
        out |= (u32)(w < 0 ? 0 : w > 0xff ? 0xff : w) << (8 * k);
    }
    return out;
}

// 0x6f1de0 (atLeast) / 0x6f1fc0: rows of the part from the edges left and right (later rows from [0xfbe208] and [0xfbe20c], which
// it steps); returns the row pointer after them.
u32 TexShadeZRows(bool atLeast, Edge *left, const Edge *right, s32 rows, u32 row) {
    const u32 uvStep[2] = {global<u32>(0x00fbe138), global<u32>(0x00fbe13c)};
    const u16 *const shadeStep = at<const u16>(0x00fbe150);
    const u16 *const prestep = at<const u16>(0x00fbe160);
    const u32 *const texels = global<const u32 *>(0x00fbe038);
    const u32 mask = global<u32>(0x00fbe008), depthStep = global<u32>(0x00fbe148);
    do {
        const u32 start = (left->x + 0x7fff) & 0xffff8000u, end = (right->x + 0x7fff) >> 15;
        const u16 fraction = (u16)(left->x - start);
        const u32 depthRow = global<u32>(0x00fbe228) + end * 2;
        global<s32>(0x00fbe1f8) = rows;
        global<u32>(0x00fbe1fc) = row;
        s32 i = (s32)((start >> 15) - end);
        if (i < 0) {
            const u32 pixels = row + end * 4;
            u32 uv[2], z = left->z << 16 | left->z >> 16;
            u16 shade[4];
            for (int k = 0; k < 2; ++k) uv[k] = left->uv[k] - ((u32)(u16)MulHigh(prestep[2 * k], fraction) << 18);
            for (int k = 0; k < 4; ++k) shade[k] = (u16)(left->shade[k] - 2 * MulHigh(fraction, shadeStep[k]));
            for (; i < 0; ++i) {
                u16 &depth = *at<u16>(depthRow + (u32)i * 2);
                const bool drawn = atLeast ? depth >= (u16)z : depth <= (u16)z;
                z = StepDepth(z, depthStep);
                if (drawn) {
                    const u32 texel = texels[((uv[0] >> 24) | (uv[1] >> 24) << 8) & mask];
                    depth = (u16)z;  // the next pixel's (sic)
                    *at<u32>(pixels + (u32)i * 4) = Shade(texel, shade);
                }
                for (int k = 0; k < 2; ++k) uv[k] += uvStep[k];
                for (int k = 0; k < 4; ++k) shade[k] = (u16)(shade[k] + shadeStep[k]);
            }
        }
        left = global<Edge *>(0x00fbe208);
        Edge *const next = global<Edge *>(0x00fbe20c);
        const u32 rightX = next->x + next->dx, leftX = left->x + left->dx;
        next->x = rightX;
        left->x = leftX;
        right = next;
        row -= global<u32>(0x00fbe02c);
        for (int k = 0; k < 2; ++k) left->uv[k] += left->duv[k];
        for (int k = 0; k < 4; ++k) left->shade[k] = (u16)(left->shade[k] + left->dshade[k]);
        left->z += left->dz;
        global<u32>(0x00fbe228) -= global<u32>(0x00fbe030);
    } while (--rows > 0);
    return row;
}

}  // namespace

extern "C" {

void __cdecl RasterSpan_TexShadeZ(u32 row) {
    const bool atLeast = global<u32>(0x00fbe034) == 0;
    if (global<s32>(0x00fbe204) > 0)
        row = TexShadeZRows(atLeast, global<Edge *>(0x00fbe208), global<Edge *>(0x00fbe20c), global<s32>(0x00fbe204), row);
    if (global<s32>(0x00fbe200) > 0) {
        global<u32>(0x00fbe208) = global<u32>(0x00fbe210);
        global<u32>(0x00fbe20c) = global<u32>(0x00fbe214);
        row = TexShadeZRows(atLeast, global<Edge *>(0x00fbe208), global<Edge *>(0x00fbe20c), global<s32>(0x00fbe200), row);
    }
}
}
