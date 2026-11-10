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

u32 Saturate(s16 w, int k) { return (u32)(w < 0 ? 0 : w > 0xff ? 0xff : w) << (8 * k); }  // PACKUSWB, byte k

u8 Byte(u32 v, int k) { return (u8)(v >> (8 * k)); }

// The depth with its halves swapped (the fraction high), stepped with the carry out of the fraction added in (ADD, ADC 0).
u32 StepDepth(u32 z, u32 step) {
    const u32 sum = z + step;
    return sum + (sum < z ? 1u : 0u);
}

// The texel's four bytes times the shade words (PMULHW of 2 * byte).
u32 Shade(u32 texel, const u16 *shade, const u32 *) {
    u32 out = 0;
    for (int k = 0; k < 4; ++k) out |= Saturate(MulHigh((u16)(Byte(texel, k) * 2), shade[k]), k);
    return out;
}

// The shaded texel (8 * byte) and the pixel under it (4 * byte), each times the row of its table (0xfbe2e0, 0xfbe4e0) for the
// texel's top 6 bits, summed.
u32 Blend(u32 texel, const u16 *shade, const u32 *dest) {
    const u32 alpha = texel >> 26, under = *dest;
    const u16 *const mine = at<const u16>(0x00fbe2e0 + alpha * 8), *const theirs = at<const u16>(0x00fbe4e0 + alpha * 8);
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const u16 lit = (u16)MulHigh((u16)(Byte(texel, k) << 3), shade[k]);
        out |= Saturate((s16)(MulHigh(lit, mine[k]) + MulHigh((u16)(Byte(under, k) << 2), theirs[k])), k);
    }
    return out;
}

enum Depth { kWrite, kKeep, kAlphaTest };  // the depth written; none; both it and the pixel only above the alpha [0xfbe010]

// One routine's row loop (two each, by the depth test: atLeast where the buffer's is at least the pixel's): rows of the part from the
// edges left and right (later rows from [0xfbe208] and [0xfbe20c], which it steps); returns the row pointer after them.
template <u32 (*Colour)(u32, const u16 *, const u32 *), Depth kDepth>
u32 TexRows(bool atLeast, Edge *left, const Edge *right, s32 rows, u32 row) {
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
                    u32 &out = *at<u32>(pixels + (u32)i * 4);
                    const u32 colour = Colour(texel, shade, &out);
                    if (kDepth == kKeep) {
                        out = colour;
                    } else if (kDepth == kWrite || (s32)(texel >> 26) > global<s32>(0x00fbe010)) {
                        depth = (u16)z;  // the next pixel's (sic)
                        out = colour;
                    }
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

// The routine: the triangle's two parts.
template <u32 (*Rows)(bool, Edge *, const Edge *, s32, u32)>
void Parts(u32 row) {
    const bool atLeast = global<u32>(0x00fbe034) == 0;
    if (global<s32>(0x00fbe204) > 0)
        row = Rows(atLeast, global<Edge *>(0x00fbe208), global<Edge *>(0x00fbe20c), global<s32>(0x00fbe204), row);
    if (global<s32>(0x00fbe200) > 0) {
        global<u32>(0x00fbe208) = global<u32>(0x00fbe210);
        global<u32>(0x00fbe20c) = global<u32>(0x00fbe214);
        row = Rows(atLeast, global<Edge *>(0x00fbe208), global<Edge *>(0x00fbe20c), global<s32>(0x00fbe200), row);
    }
}

}  // namespace

extern "C" {

void __cdecl RasterSpan_TexShadeZ(u32 row) { Parts<TexRows<Shade, kWrite>>(row); }

void __cdecl RasterSpan_TexBlend(u32 row) { Parts<TexRows<Blend, kKeep>>(row); }

void __cdecl RasterSpan_TexBlendAlphaZ(u32 row) { Parts<TexRows<Blend, kAlphaTest>>(row); }
}
