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

// The texel at the top bytes of u and v, masked by [0xfbe008] (or not).
template <bool kMasked> u32 Texel(const u32 *uv) {
    const u32 index = (uv[0] >> 24) | (uv[1] >> 24) << 8;
    return global<const u32 *>(0x00fbe038)[kMasked ? index & global<u32>(0x00fbe008) : index];
}

// Each pixel's source: four words from u and v, and texel the texel read (its top 6 bits the alpha).
typedef void (*Source)(const u32 *, u16 *, u32 &);

// The texel's four bytes.
template <bool kMasked> void Point(const u32 *uv, u16 *source, u32 &texel) {
    texel = Texel<kMasked>(uv);
    for (int k = 0; k < 4; ++k) source[k] = Byte(texel, k);
}

// The four texels at (u, v), (u + 1, v), (u, v + 1), (u + 1, v + 1) weighted by u's and v's bits 10-25 masked by [0xfbe1b8] (its
// two dwords); kWrapped: the index stepped by 1, 0x100, -1 and masked each time, else the masked index plus 1, 0x100, 0x101.
template <bool kWrapped> void Filter(const u32 *uv, u16 *source, u32 &texel) {
    const u32 mask = global<u32>(0x00fbe008);
    const u32 *const texels = global<const u32 *>(0x00fbe038);
    const u32 index = ((uv[0] >> 24) | (uv[1] >> 24) << 8) & mask;
    const u32 i10 = kWrapped ? (index + 1) & mask : index + 1;
    const u32 i11 = kWrapped ? (i10 + 0x100) & mask : index + 0x101;
    const u32 i01 = kWrapped ? (i11 - 1) & mask : index + 0x100;
    const u32 t00 = texels[index], t10 = texels[i10], t11 = texels[i11], t01 = texels[i01];
    texel = t00;
    const u16 fu = (u16)((uv[0] >> 10) & global<u32>(0x00fbe1b8)), fv = (u16)((uv[1] >> 10) & global<u32>(0x00fbe1bc));
    for (int k = 0; k < 4; ++k) {
        const u16 top = (u16)(Byte(t00, k) + MulHigh((u16)((Byte(t10, k) - Byte(t00, k)) * 4), fu));
        const u16 bottom = (u16)(Byte(t01, k) + MulHigh((u16)((Byte(t11, k) - Byte(t01, k)) * 4), fu));
        source[k] = (u16)(top + MulHigh((u16)((u16)(bottom - top) * 4), fv));
    }
}

// Each pixel's colour: from the source, the shade words and the pixel under it.

// The source's words times the shade words (PMULHW of 2 * word).
template <Source kSource> u32 Shade(const u32 *uv, const u16 *shade, const u32 *, u32 &texel) {
    u16 source[4];
    kSource(uv, source, texel);
    u32 out = 0;
    for (int k = 0; k < 4; ++k) out |= Saturate(MulHigh((u16)(source[k] * 2), shade[k]), k);
    return out;
}

// The shaded source (8 * word) and the pixel under it (4 * byte), each times the row of its table (0xfbe2e0, 0xfbe4e0) for the
// fourth word's top bits (word >> 2), summed.
template <Source kSource> u32 Blend(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    u16 source[4];
    kSource(uv, source, texel);
    const u32 alpha = source[3] >> 2, under = *dest;
    const u16 *const mine = at<const u16>(0x00fbe2e0 + alpha * 8), *const theirs = at<const u16>(0x00fbe4e0 + alpha * 8);
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const u16 lit = (u16)MulHigh((u16)(source[k] << 3), shade[k]);
        out |= Saturate((s16)(MulHigh(lit, mine[k]) + MulHigh((u16)(Byte(under, k) << 2), theirs[k])), k);
    }
    return out;
}

// The source shaded as Shade's, then 4 * each word and 4 * the pixel under it times their tables' rows, for the top bits of the
// fourth shaded word (word >> 2) or kSecond of the second (word >> 10, sic: the first two words' top 6 bits), summed.
template <Source kSource, bool kSecond> u32 BlendLit(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    u16 source[4], lit[4];
    kSource(uv, source, texel);
    for (int k = 0; k < 4; ++k) lit[k] = (u16)MulHigh((u16)(source[k] * 2), shade[k]);
    const u32 alpha = kSecond ? lit[1] >> 10 : lit[3] >> 2, under = *dest;
    const u16 *const mine = at<const u16>(0x00fbe2e0 + alpha * 8), *const theirs = at<const u16>(0x00fbe4e0 + alpha * 8);
    u32 out = 0;
    for (int k = 0; k < 4; ++k)
        out |= Saturate((s16)(MulHigh((u16)(lit[k] << 2), mine[k]) + MulHigh((u16)(Byte(under, k) << 2), theirs[k])), k);
    return out;
}

// Shade's pixel added to the pixel under it, each byte saturated (PADDUSB).
template <Source kSource> u32 Add(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    const u32 lit = Shade<kSource>(uv, shade, dest, texel), under = *dest;
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const u32 sum = (u32)Byte(lit, k) + Byte(under, k);
        out |= (sum > 0xff ? 0xffu : sum) << (8 * k);
    }
    return out;
}

// What a drawn pixel writes: the pixel and the next pixel's depth (sic); the pixel and its own depth; the pixel only; both (the
// next pixel's depth) only above the alpha [0xfbe010] (signed).
enum Depth { kWriteNext, kWrite, kKeep, kAlphaTest };

// One routine's row loop (two each, by the depth test: atLeast where the buffer's is at least the pixel's, else at most, or below
// with kBelow): rows of the part from the edges left and right (later rows from [0xfbe208] and [0xfbe20c], which it steps); returns
// the row pointer after them. kHalf: u and v less [0xfbe0e4] (two dwords) at each row's start too. kRunTexel: the texel of each run
// of drawn pixels is its first pixel's (sic: the index is not reloaded in the loop).
template <u32 (*Colour)(const u32 *, const u16 *, const u32 *, u32 &), Depth kDepth, bool kBelow, bool kHalf, bool kRunTexel = false>
u32 TexRows(bool atLeast, Edge *left, const Edge *right, s32 rows, u32 row) {
    const u32 uvStep[2] = {global<u32>(0x00fbe138), global<u32>(0x00fbe13c)};
    const u16 *const shadeStep = at<const u16>(0x00fbe150);
    const u16 *const prestep = at<const u16>(0x00fbe160);
    const u32 depthStep = global<u32>(0x00fbe148);
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
            if (kHalf)
                for (int k = 0; k < 2; ++k) uv[k] -= global<u32>(0x00fbe0e4 + 4 * k);
            for (int k = 0; k < 4; ++k) shade[k] = (u16)(left->shade[k] - 2 * MulHigh(fraction, shadeStep[k]));
            u32 held[2] = {0, 0};
            bool run = false;
            for (; i < 0; ++i) {
                u16 &depth = *at<u16>(depthRow + (u32)i * 2);
                const bool drawn = atLeast ? depth >= (u16)z : kBelow ? depth < (u16)z : depth <= (u16)z;
                const u32 here = z;
                z = StepDepth(z, depthStep);
                if (drawn) {
                    u32 &out = *at<u32>(pixels + (u32)i * 4);
                    if (!run) held[0] = uv[0], held[1] = uv[1];
                    u32 texel;
                    const u32 colour = Colour(kRunTexel ? held : uv, shade, &out, texel);
                    if (kDepth == kKeep) {
                        out = colour;
                    } else if (kDepth != kAlphaTest || (s32)(texel >> 26) > global<s32>(0x00fbe010)) {
                        depth = (u16)(kDepth == kWrite ? here : z);
                        out = colour;
                    }
                }
                run = drawn;
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

void __cdecl RasterSpan_TexShadeZ(u32 row) { Parts<TexRows<Shade<Point<true>>, kWriteNext, false, false>>(row); }

void __cdecl RasterSpan_TexBlend(u32 row) { Parts<TexRows<Blend<Point<true>>, kKeep, false, false>>(row); }

void __cdecl RasterSpan_TexBlendAlphaZ(u32 row) { Parts<TexRows<Blend<Point<true>>, kAlphaTest, false, false>>(row); }

void __cdecl RasterSpan_TexBlendLit(u32 row) { Parts<TexRows<BlendLit<Point<false>, true>, kKeep, true, false, true>>(row); }

void __cdecl RasterSpan_TexAdd(u32 row) { Parts<TexRows<Add<Point<true>>, kKeep, false, false>>(row); }

void __cdecl RasterSpan_BilinearShadeZ(u32 row) { Parts<TexRows<Shade<Filter<true>>, kWrite, false, true>>(row); }

void __cdecl RasterSpan_BilinearAdd(u32 row) { Parts<TexRows<Add<Filter<true>>, kKeep, false, true>>(row); }

void __cdecl RasterSpan_BilinearBlend(u32 row) { Parts<TexRows<Blend<Filter<true>>, kKeep, false, true>>(row); }

void __cdecl RasterSpan_BilinearBlendLitZ(u32 row) { Parts<TexRows<BlendLit<Filter<false>, false>, kWrite, false, true>>(row); }
}
