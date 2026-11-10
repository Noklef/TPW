#include "rastertriangle.h"

#include "rasterspans.h"

#include <cstdint>
#include <cstring>
#include "rasteriser.h"
#include "devicecreate.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T at(const u8 *p, u32 offset) { return *reinterpret_cast<const T *>(p + offset); }

s16 MulHigh(u16 a, u16 b) { return (s16)(((s32)(s16)a * (s16)b) >> 16); }  // PMULHW

u16 Saturate(u32 v) { return (u16)((s32)v < -0x8000 ? -0x8000 : (s32)v > 0x7fff ? 0x7fff : (s32)v); }  // PACKSSDW

// PMADDWD of one dword lane: the low words' product plus the high words'.
u32 MaddWords(u32 a, u32 b) { return (u32)((s32)(s16)a * (s16)b) + (u32)((s32)(s16)(a >> 16) * (s16)(b >> 16)); }

// The words w times the 16.16 s, as the set-ups' MMX does it: w with s's words halved (PMADDWD) >> shift, plus w's low word times s's
// high word << (15 - shift).
u32 MulFixed(u32 w, u32 s, int shift) {
    return (u32)((s32)MaddWords(w, (s >> 1) & 0x7fff7fffu) >> shift) + ((u32)((s32)(s16)w * (s16)(s >> 16)) << (15 - shift));
}

// IMUL; bits 15-46 of the 64-bit product.
u32 MulShift15(u32 a, u32 b) { return (u32)((uint64_t)((int64_t)(s32)a * (s32)b) >> 15); }

// CDQ; IDIV: faults (sic) on INT_MIN / -1, as the original.
s32 Idiv(s32 a, s32 divisor) {
    s32 q;
    __asm {
        mov eax, a
        cdq
        idiv divisor
        mov q, eax
    }
    return q;
}

// Each word of a less b's.
u32 SubWords(u32 a, u32 b) { return (u16)(a - b) | (u32)(u16)((a >> 16) - (b >> 16)) << 16; }

// An edge from vertex `from` to `to` (record 0xfbe240, 0xfbe274 or 0xfbe2a8): the slope (x 17.15 per row, the x difference << 15 left
// as it is with no rows between), x at the first row (prestep the row fraction << 11, the prestep's product at +0x30).
void SetEdge(u32 edge, const u8 *from, const u8 *to, u32 prestep) {
    const u32 dx = (at<u32>(to, 0) - at<u32>(from, 0)) << 15;
    const s32 dy = (s32)(at<u32>(from, 4) - at<u32>(to, 4));
    const u32 slope = dy ? (u32)Idiv((s32)dx, dy) : dx;
    global<u32>(edge + 4) = slope;
    const u32 step = MulShift15(slope, prestep);
    global<u32>(edge) = (at<u32>(from, 0) << 11) + step - 0x4000;
    global<u32>(edge + 0x30) = step;
}

// A left edge's start at `from` and steps: shade (kShade) from the per-pixel and per-row steps, u and v, depth (kDepth).
template <bool kShade, bool kDepth> void StartEdge(u32 edge, const u8 *from, u32 prestep) {
    const u32 slope = global<u32>(edge + 4), step = global<u32>(edge + 0x30);
    if (kShade) {
        const u16 *const perPixel = &global<u16>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x18), *const perRow = &global<u16>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x20);
        for (int k = 0; k < 4; ++k) {
            global<u16>(edge + 0x20 + 2 * k) = (u16)(Saturate(MulFixed(perPixel[k], slope, 14)) - perRow[k]);
            global<u16>(edge + 0x18 + 2 * k) = (u16)((at<u8>(from, 0xc + k) << 7) - 2 * MulHigh((u16)prestep, perRow[k]) +
                                                     ((u16)MulHigh((u16)((s32)step >> 8), perPixel[k]) << 7));
        }
    }
    for (int k = 0; k < 2; ++k) {
        const u32 perPixel = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 4 * k), perRow = global<u32>(((u32)(uintptr_t)&gRasterspans_fbe138 + 0x8) + 4 * k);
        global<u32>(edge + 0x10 + 4 * k) = MulShift15(slope, perPixel) - perRow;
        global<u32>(edge + 8 + 4 * k) = (at<u32>(from, 0x10 + 4 * k) << 20) - MulShift15(perRow, prestep) + MulShift15(perPixel, step);
    }
    if (kDepth) {
        const u32 perPixel = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x10), perRow = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x14);
        global<u32>(edge + 0x2c) = MulShift15(slope, perPixel) - perRow;
        global<u32>(edge + 0x28) = ((u32)at<u16>(from, 8) << 16) - MulShift15(perRow, prestep) + MulShift15(perPixel, step);
    }
}

// The span routine [0xfbe044] (one of the 21 its tables hold).
void Span(u32 row) {
    switch (gRasteriser_fbe044) {
    case 0x006f1d70: RasterSpan_TexShadeZ(row); break;
    case 0x006f2140: RasterSpan_TexBlend(row); break;
    case 0x006f2570: RasterSpan_TexBlendAlphaZ(row); break;
    case 0x006f29d0: RasterSpan_TexBlendLit(row); break;
    case 0x006f2de0: RasterSpan_TexAdd(row); break;
    case 0x006f31c0: RasterSpan_BilinearShadeZ(row); break;
    case 0x006f36d0: RasterSpan_BilinearAdd(row); break;
    case 0x006f3be0: RasterSpan_BilinearBlend(row); break;
    case 0x006f4140: RasterSpan_BilinearBlendLitZ(row); break;
    case 0x006f4690: RasterSpan_ShadeZ(row); break;
    case 0x006f49c0: RasterSpan_TexZ(row); break;
    case 0x006f4d40: RasterSpan_TexTintZ(row); break;
    case 0x006f50d0: RasterSpan_TexBrightZ(row); break;
    case 0x006f5590: RasterSpan_TexShadeNoZ(row); break;
    case 0x006f5840: RasterSpan_TexAddNoZ(row); break;
    case 0x006f56d0: RasterSpan_TexBlendKeyedNoZ(row); break;
    case 0x006f5980: RasterSpan_TexBlendLitNoZ(row); break;
    case 0x006f5af0: RasterSpan_BilinearAddNoZ(row); break;
    case 0x006f5cc0: RasterSpan_BilinearShadeNoZ(row); break;
    case 0x006f5e90: RasterSpan_BilinearBlendLitNoZ(row); break;
    case 0x006f6090: RasterSpan_ShadeNoZ(row); break;
    default: break;  // shortcut: the original jumps to any address; the callers set only these (tables 0x6f19b0-0x6f1a20).
    }
}

template <bool kShade, bool kDepth> void Setup(const u8 *v0, const u8 *v1, const u8 *v2) {
    const u8 *const v[3] = {v0, v1, v2};
    // 1 / area (the cross product of the low words), then the x and y differences (saturated to words) scaled by it.
    const u32 dx21 = at<u32>(v2, 0) - at<u32>(v1, 0), dy21 = at<u32>(v2, 4) - at<u32>(v1, 4);
    const u32 dx20 = at<u32>(v2, 0) - at<u32>(v0, 0), dy20 = at<u32>(v2, 4) - at<u32>(v0, 4);
    const u32 area = (u32)((s32)(s16)dy21 * (s16)dx20) - (u32)((s32)(s16)dx21 * (s16)dy20);
    const u32 inverse = (s32)area > 0x80 ? (u32)(0x8000000000ull / area) : 0;
    const u32 sdx21 = MulFixed(Saturate(dx21), inverse, 14), sdy21 = MulFixed(Saturate(dy21), inverse, 14);
    const u32 sdx20 = MulFixed(Saturate(dx20), inverse, 14), sdy20 = MulFixed(Saturate(dy20), inverse, 14);
    s32 rows[3];
    for (int k = 0; k < 3; ++k) global<s32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 4 * k) = rows[k] = (s32)(at<u32>(v[k], 4) - 8) >> 4;
    // The top (most rows), bottom and middle (sic: on ties both top and bottom can be v2).
    int top = 0, bottom = 1, middle = 2;
    if (rows[0] < rows[1]) top = 1, bottom = 0;
    if (rows[top] <= rows[2]) middle = top, top = 2;
    if (rows[bottom] >= rows[2]) middle = bottom, bottom = 2;
    const s32 topRow = rows[top], middleRow = rows[middle];
    global<s32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0xcc) = topRow - middleRow;
    global<s32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0xc8) = middleRow - rows[bottom];
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x10) = (at<u32>(v[top], 4) - 8 - ((u32)topRow << 4)) << 11;
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0xc) = (at<u32>(v[middle], 4) - 8 - ((u32)middleRow << 4)) << 11;
    const u32 order = global<u32>(((u32)(uintptr_t)&gRastertriangle_fbe6f0 + 0x20) + 4 * ((middle < top) << 2 | (bottom < middle) << 1 | (bottom < top)));
    u32 pair[2];
    memcpy(pair, &global<u32>((u32)(uintptr_t)&gRastertriangle_fbe6f0 + order), 8);
    memcpy(&global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0xd8), pair, 8);
    memcpy(pair, &global<u32>(((u32)(uintptr_t)&gRastertriangle_fbe6f0 + 0x8) + order), 8);
    memcpy(&global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0xd0), pair, 8);
    const u32 row = global<u32>((u32)(uintptr_t)&gDevicecreate_fbe024) + MaddWords((u32)topRow, global<u32>((u32)(uintptr_t)&gDevicecreate_fbe02c));
    const u32 depthRow = gDevicecreate_fbe028 + MaddWords((u32)topRow, gDevicecreate_fbe030);
    SetEdge((u32)(uintptr_t)&gRastertriangle_fbe240 + 0x68, v[top], v[middle], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x10));
    SetEdge((u32)(uintptr_t)&gRastertriangle_fbe240 + 0x34, v[middle], v[bottom], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0xc));
    SetEdge((u32)(uintptr_t)&gRastertriangle_fbe240, v[top], v[bottom], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x10));
    global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0xf0) = depthRow;
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x24) = global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x28) = sdx20;
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x1c) = global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x20) = sdy21;
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x2c) = global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x30) = sdx21;
    global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x14) = global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x18) = sdy20;
    // The gradients: each the differences from v2 (v1's and v0's) across the scaled ones.
    if (kShade)
        for (int k = 0; k < 4; ++k) {
            const u32 d1 = (u16)((at<u8>(v1, 0xc + k) - at<u8>(v2, 0xc + k)) * 8), d0 = (u16)((at<u8>(v0, 0xc + k) - at<u8>(v2, 0xc + k)) * 8);
            global<u16>(((u32)(uintptr_t)&gRasterspans_fbe138 + 0x18) + 2 * k) = Saturate(MulFixed(d1, sdy20, 15) - MulFixed(d0, sdy21, 15));
            global<u16>(((u32)(uintptr_t)&gRasterspans_fbe138 + 0x20) + 2 * k) = Saturate(MulFixed(d0, sdx21, 15) - MulFixed(d1, sdx20, 15));
        }
    for (int k = 0; k < 2; ++k) {
        const u32 d1 = at<u32>(v1, 0x10 + 4 * k) - at<u32>(v2, 0x10 + 4 * k), d0 = at<u32>(v0, 0x10 + 4 * k) - at<u32>(v2, 0x10 + 4 * k);
        const u32 perPixel = d1 * sdy20 - d0 * sdy21;
        global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 4 * k) = perPixel;
        global<u32>(((u32)(uintptr_t)&gRasterspans_fbe138 + 0x8) + 4 * k) = d0 * sdx21 - d1 * sdx20;
        global<s32>(((u32)(uintptr_t)&gRasterspans_fbe138 + 0x28) + 4 * k) = (s32)perPixel >> 17;
    }
    if (kDepth) {
        const u32 d1 = SubWords(at<u32>(v1, 8), at<u32>(v2, 8)), d0 = SubWords(at<u32>(v0, 8), at<u32>(v2, 8));
        global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x10) = (MulFixed(d1, sdy20 << 8, 14) - MulFixed(d0, sdy21 << 8, 14)) << 3;
        global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x14) = (MulFixed(d0, sdx21 << 8, 14) - MulFixed(d1, sdx20 << 8, 14)) << 3;
    }
    // The left edges: top to bottom for both parts, or top to middle then middle to bottom.
    if (order == 0) {
        StartEdge<kShade, kDepth>((u32)(uintptr_t)&gRastertriangle_fbe240, v[top], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x10));
    } else {
        StartEdge<kShade, kDepth>((u32)(uintptr_t)&gRastertriangle_fbe240 + 0x34, v[middle], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0xc));
        StartEdge<kShade, kDepth>((u32)(uintptr_t)&gRastertriangle_fbe240 + 0x68, v[top], global<u32>((u32)(uintptr_t)&gRastertriangle_fbe0b0 + 0x10));
    }
    if (kDepth) {
        const u32 perPixel = global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x10);
        global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x30) = perPixel;
        global<u32>((u32)(uintptr_t)&gRasterspans_fbe138 + 0x10) = perPixel << 16 | perPixel >> 16;
    }
    Span(row);
}

}  // namespace

extern "C" {

void __cdecl RasterTriangle_Draw(const u8 *v0, const u8 *v1, const u8 *v2) { Setup<true, true>(v0, v1, v2); }

void __cdecl RasterTriangle_DrawFlat(const u8 *v0, const u8 *v1, const u8 *v2) {
    for (int k = 0; k < 4; ++k) gRasterspans_fbe000[k] = (u16)(at<u8>(v0, 0xc + k) << 7);
    Setup<false, true>(v0, v1, v2);
}

void __cdecl RasterTriangle_DrawNoZ(const u8 *v0, const u8 *v1, const u8 *v2) { Setup<true, false>(v0, v1, v2); }
}
