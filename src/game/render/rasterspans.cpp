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
// two dwords); kWrapped: the index stepped by 1, 0x100, -1 and masked each time, else the index (masked, or not) plus 1, 0x100, 0x101.
template <bool kWrapped, bool kMasked = true> void Filter(const u32 *uv, u16 *source, u32 &texel) {
    const u32 mask = global<u32>(0x00fbe008);
    const u32 *const texels = global<const u32 *>(0x00fbe038);
    const u32 index = ((uv[0] >> 24) | (uv[1] >> 24) << 8) & (kMasked ? mask : ~0u);
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

// Where BlendLit's alpha comes from: the fourth shaded word >> 2; the second >> 10 (sic: the first two words' top 6 bits); the
// fourth source word >> 2.
enum Alpha { kLitFourth, kLitSecond, kSourceFourth };

// The source shaded as Shade's, then 4 * each word and 4 * the pixel under it times their tables' rows for the alpha, summed.
template <Source kSource, Alpha kAlpha> u32 BlendLit(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    u16 source[4], lit[4];
    kSource(uv, source, texel);
    for (int k = 0; k < 4; ++k) lit[k] = (u16)MulHigh((u16)(source[k] * 2), shade[k]);
    const u32 alpha = kAlpha == kLitFourth ? lit[3] >> 2 : kAlpha == kLitSecond ? lit[1] >> 10 : source[3] >> 2, under = *dest;
    const u16 *const mine = at<const u16>(0x00fbe2e0 + alpha * 8), *const theirs = at<const u16>(0x00fbe4e0 + alpha * 8);
    u32 out = 0;
    for (int k = 0; k < 4; ++k)
        out |= Saturate((s16)(MulHigh((u16)(lit[k] << 2), mine[k]) + MulHigh((u16)(Byte(under, k) << 2), theirs[k])), k);
    return out;
}

// The texel as it is.
u32 Copy(const u32 *uv, const u16 *, const u32 *, u32 &texel) { return texel = Texel<true>(uv); }

// The texel shaded as Shade's by the four words at 0xfbe000 (one shade for the triangle).
u32 Tint(const u32 *uv, const u16 *, const u32 *dest, u32 &texel) { return Shade<Point<true>>(uv, at<const u16>(0x00fbe000), dest, texel); }

// The texel (unmasked) shaded as Shade's and blended as BlendLit's by its own top 6 bits: at 0 the pixel under it kept (not
// written), at 63 the shaded texel unblended.
u32 BlendKeyed(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    texel = Texel<false>(uv);
    const u32 alpha = texel >> 26, under = *dest;
    if (alpha == 0) return under;
    const u16 *const mine = at<const u16>(0x00fbe2e0 + alpha * 8), *const theirs = at<const u16>(0x00fbe4e0 + alpha * 8);
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const u16 lit = (u16)MulHigh((u16)(Byte(texel, k) * 2), shade[k]);
        out |= Saturate(alpha == 63 ? (s16)lit : (s16)(MulHigh((u16)(lit << 2), mine[k]) + MulHigh((u16)(Byte(under, k) << 2), theirs[k])), k);
    }
    return out;
}

// a + b, each byte saturated (PADDUSB).
u32 AddBytes(u32 a, u32 b) {
    u32 out = 0;
    for (int k = 0; k < 4; ++k) {
        const u32 sum = (u32)Byte(a, k) + Byte(b, k);
        out |= (sum > 0xff ? 0xffu : sum) << (8 * k);
    }
    return out;
}

// The texel doubled.
u32 Bright(const u32 *uv, const u16 *, const u32 *, u32 &texel) {
    texel = Texel<true>(uv);
    return AddBytes(texel, texel);
}

// Shade's pixel added to the pixel under it.
template <Source kSource> u32 Add(const u32 *uv, const u16 *shade, const u32 *dest, u32 &texel) {
    return AddBytes(Shade<kSource>(uv, shade, dest, texel), *dest);
}

// A row's end: the edges [0xfbe208] (now left) and [0xfbe20c] (now right) step x (both read first), the row pointer -[0xfbe02c],
// the left edge's u and v (kUv), shade (kShade) and depth their steps, and (kZ) the depth row [0xfbe228] -[0xfbe030]; returns the
// row pointer.
template <bool kUv, bool kShade = true, bool kZ = true> u32 NextRow(Edge *&left, const Edge *&right, u32 row) {
    left = global<Edge *>(0x00fbe208);
    Edge *const next = global<Edge *>(0x00fbe20c);
    const u32 rightX = next->x + next->dx, leftX = left->x + left->dx;
    next->x = rightX;
    left->x = leftX;
    right = next;
    if (kUv)
        for (int k = 0; k < 2; ++k) left->uv[k] += left->duv[k];
    if (kShade)
        for (int k = 0; k < 4; ++k) left->shade[k] = (u16)(left->shade[k] + left->dshade[k]);
    if (kZ) {
        left->z += left->dz;
        global<u32>(0x00fbe228) -= global<u32>(0x00fbe030);
    }
    return row - global<u32>(0x00fbe02c);
}

// What a drawn pixel writes: the pixel and the next pixel's depth (sic); the pixel and its own depth; the pixel only; both (the
// next pixel's depth) only above the alpha [0xfbe010] (signed); every pixel, no depth read or stepped ([0xfbe1f8] and [0xfbe1fc]
// not written).
enum Depth { kWriteNext, kWrite, kKeep, kAlphaTest, kNone };

// One routine's row loop (two each, by the depth test: atLeast where the buffer's is at least the pixel's, else at most, or below
// with kBelow): rows of the part from the edges left and right (later rows from [0xfbe208] and [0xfbe20c], which it steps); returns
// the row pointer after them. kHalf: u and v less [0xfbe0e4] (two dwords) at each row's start too. kRunTexel: the texel of each run
// of drawn pixels is its first pixel's (sic: the index is not reloaded in the loop). kShaded: the left edge's shade stepped per row.
template <u32 (*Colour)(const u32 *, const u16 *, const u32 *, u32 &), Depth kDepth, bool kBelow, bool kHalf, bool kRunTexel = false,
          bool kShaded = true>
u32 TexRows(bool atLeast, Edge *left, const Edge *right, s32 rows, u32 row) {
    const u32 uvStep[2] = {global<u32>(0x00fbe138), global<u32>(0x00fbe13c)};
    const u16 *const shadeStep = at<const u16>(0x00fbe150);
    const u16 *const prestep = at<const u16>(0x00fbe160);
    const u32 depthStep = global<u32>(0x00fbe148);
    do {
        const u32 start = (left->x + 0x7fff) & 0xffff8000u, end = (right->x + 0x7fff) >> 15;
        const u16 fraction = (u16)(left->x - start);
        const u32 depthRow = global<u32>(0x00fbe228) + end * 2;
        if (kDepth != kNone) {
            global<s32>(0x00fbe1f8) = rows;
            global<u32>(0x00fbe1fc) = row;
        }
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
                const bool drawn = kDepth == kNone || (atLeast ? depth >= (u16)z : kBelow ? depth < (u16)z : depth <= (u16)z);
                const u32 here = z;
                z = StepDepth(z, depthStep);
                if (drawn) {
                    u32 &out = *at<u32>(pixels + (u32)i * 4);
                    if (!run) held[0] = uv[0], held[1] = uv[1];
                    u32 texel;
                    const u32 colour = Colour(kRunTexel ? held : uv, shade, &out, texel);
                    if (kDepth == kKeep || kDepth == kNone) {
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
        row = NextRow<true, kShaded, kDepth != kNone>(left, right, row);
    } while (--rows > 0);
    return row;
}

// 0x6f4690's row loop: the shade words >> 7, saturated, kept with the next pixel's depth (sic); the shade not prestepped (sic: the
// prestep is computed, not applied); after a drawn pixel the next one is tested against the drawn one's depth (sic); u and v unused.
// Without kZ (0x6f6090's): every pixel, no depth read, written or stepped, [0xfbe1f8] and [0xfbe1fc] not written.
template <bool kZ> u32 ShadeRows(bool atLeast, Edge *left, const Edge *right, s32 rows, u32 row) {
    const u16 *const shadeStep = at<const u16>(0x00fbe150);
    const u32 depthStep = global<u32>(0x00fbe148);
    do {
        const u32 start = (left->x + 0x7fff) & 0xffff8000u, end = (right->x + 0x7fff) >> 15;
        const u32 depthRow = global<u32>(0x00fbe228) + end * 2;
        if (kZ) {
            global<s32>(0x00fbe1f8) = rows;
            global<u32>(0x00fbe1fc) = row;
        }
        s32 i = (s32)((start >> 15) - end);
        if (i < 0) {
            const u32 pixels = row + end * 4;
            u32 z = left->z << 16 | left->z >> 16, test = z;
            u16 shade[4];
            for (int k = 0; k < 4; ++k) shade[k] = left->shade[k];
            for (; i < 0; ++i) {
                u16 &depth = *at<u16>(depthRow + (u32)i * 2);
                const bool drawn = !kZ || (atLeast ? depth >= (u16)test : depth <= (u16)test);
                const u32 here = z;
                z = StepDepth(z, depthStep);
                test = drawn ? here : z;
                if (drawn) {
                    if (kZ) depth = (u16)z;
                    u32 out = 0;
                    for (int k = 0; k < 4; ++k) out |= Saturate((s16)(shade[k] >> 7), k);
                    *at<u32>(pixels + (u32)i * 4) = out;
                }
                for (int k = 0; k < 4; ++k) shade[k] = (u16)(shade[k] + shadeStep[k]);
            }
        }
        row = NextRow<false, true, kZ>(left, right, row);
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

void __cdecl RasterSpan_TexBlendLit(u32 row) { Parts<TexRows<BlendLit<Point<false>, kLitSecond>, kKeep, true, false, true>>(row); }

void __cdecl RasterSpan_TexAdd(u32 row) { Parts<TexRows<Add<Point<true>>, kKeep, false, false>>(row); }

void __cdecl RasterSpan_BilinearShadeZ(u32 row) { Parts<TexRows<Shade<Filter<true>>, kWrite, false, true>>(row); }

void __cdecl RasterSpan_BilinearAdd(u32 row) { Parts<TexRows<Add<Filter<true>>, kKeep, false, true>>(row); }

void __cdecl RasterSpan_BilinearBlend(u32 row) { Parts<TexRows<Blend<Filter<true>>, kKeep, false, true>>(row); }

void __cdecl RasterSpan_BilinearBlendLitZ(u32 row) { Parts<TexRows<BlendLit<Filter<false>, kLitFourth>, kWrite, false, true>>(row); }

void __cdecl RasterSpan_ShadeZ(u32 row) { Parts<ShadeRows<true>>(row); }

void __cdecl RasterSpan_TexZ(u32 row) { Parts<TexRows<Copy, kWriteNext, false, false, false, false>>(row); }

void __cdecl RasterSpan_TexTintZ(u32 row) { Parts<TexRows<Tint, kWriteNext, false, false, false, false>>(row); }

void __cdecl RasterSpan_TexBrightZ(u32 row) { Parts<TexRows<Bright, kWriteNext, false, false, false, false>>(row); }

void __cdecl RasterSpan_TexShadeNoZ(u32 row) { Parts<TexRows<Shade<Point<false>>, kNone, false, false>>(row); }

void __cdecl RasterSpan_TexAddNoZ(u32 row) { Parts<TexRows<Add<Point<false>>, kNone, false, false>>(row); }

void __cdecl RasterSpan_TexBlendKeyedNoZ(u32 row) { Parts<TexRows<BlendKeyed, kNone, false, false>>(row); }

void __cdecl RasterSpan_TexBlendLitNoZ(u32 row) { Parts<TexRows<BlendLit<Point<false>, kLitFourth>, kNone, false, false>>(row); }

void __cdecl RasterSpan_BilinearAddNoZ(u32 row) { Parts<TexRows<Add<Filter<false, false>>, kNone, false, true>>(row); }

void __cdecl RasterSpan_BilinearShadeNoZ(u32 row) { Parts<TexRows<Shade<Filter<false, false>>, kNone, false, true>>(row); }

void __cdecl RasterSpan_BilinearBlendLitNoZ(u32 row) {
    Parts<TexRows<BlendLit<Filter<false, false>, kSourceFourth>, kNone, false, true>>(row);
}

void __cdecl RasterSpan_ShadeNoZ(u32 row) { Parts<ShadeRows<false>>(row); }
}
