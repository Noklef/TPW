#include "wavelet.h"

#include <cstdint>
#include <cstring>
#include "../core/vcall.h"
#include "../core/gameheap.h"
#include "../core/buffers.h"
#include "../core/seqs.h"
#include "../files/wrapsources.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The encoders' square side for a width or height: the power of two above it less one (the shift count's low 5 bits), at least 8.
u32 SideFor(u32 extent) {
    u32 bits = 1;
    for (u32 rest = (extent - 1) >> 1; rest != 0; rest >>= 1)
        ++bits;
    if (bits < 3)
        bits = 3;
    return 1u << (bits & 31);
}

// The planes' coefficients go to the stream buffer at 0xa067f8.
u8 *const kStream = reinterpret_cast<u8 *>(0x00a067f8);

// __ftol of a float, the low half.
u32 FtolLow(float f) { return (u32)(long long)f; }

// The dword count of a {data, size, ...} array zeroed as the original's REP STOSD / STOSB does (size * 4 bytes).
void ZeroArray(const u8 *array) { std::memset(at<u8 *>(array, 0), 0, at<u32>(array, 4) << 2); }

// The D4 filter, read from 0xc27208..0xc27214 for each product.
float H0() { return global<float>(0x00c27208); }
float H1() { return global<float>(0x00c2720c); }
float H2() { return global<float>(0x00c27210); }
float H3() { return global<float>(0x00c27214); }

// The inverse D4's software-pipelined pair loop (0x592c50's for n >= 6, 0x5933a0's along a row): the low values at p and p + next (bytes),
// the high ones at p + high and p + high + next; the even value goes to out, the odd one to out + outNext; then p moves by step and out by
// outStep, `loops` times (a do-while: 0 runs 2^32 times), and a last pair whose odd value is summed in another order. Each load is placed
// where the original makes it relative to the two stores (the next pair's four before the odd store).
void InverseD4Pairs(const u8 *p, u32 next, u32 high, u32 step, u8 *out, u32 outNext, u32 outStep, u32 loops) {
    X87Reg a = X87Reg(at<float>(p, 0)) * H2();
    X87Reg b = X87Reg(at<float>(p, next)) * H0();
    X87Reg c = X87Reg(at<float>(p, high)) * H1();
    X87Reg d = X87Reg(at<float>(p, high + next)) * H3();
    do {
        const X87Reg ca = c + a, bd = b + d;
        const X87Reg e = X87Reg(at<float>(p, 0)) * H3();
        const X87Reg even = bd + ca;
        const X87Reg f = X87Reg(at<float>(p, next)) * H1();
        const X87Reg g = X87Reg(at<float>(p, high)) * H0();
        at<float>(out, 0) = (float)even;
        const X87Reg h = X87Reg(at<float>(p, high + next)) * H2();
        a = X87Reg(at<float>(p, step)) * H2();
        const X87Reg fe = f + e;
        b = X87Reg(at<float>(p, step + next)) * H0();
        const X87Reg gh = g + h;
        c = X87Reg(at<float>(p, step + high)) * H1();
        const X87Reg odd = fe - gh;
        d = X87Reg(at<float>(p, step + high + next)) * H3();
        at<float>(out, outNext) = (float)odd;
        p += step;
        out += outStep;
    } while (--loops != 0);
    const X87Reg ca = c + a, bd = b + d;
    const X87Reg e = X87Reg(at<float>(p, 0)) * H3();
    const X87Reg even = bd + ca;
    const X87Reg f = X87Reg(at<float>(p, next)) * H1();
    at<float>(out, 0) = (float)even;
    const X87Reg g = X87Reg(at<float>(p, high)) * H0();
    const X87Reg h = X87Reg(at<float>(p, high + next)) * H2();
    at<float>(out, outNext) = (float)(e + (f - (g + h)));
}

// The inverse D4 down the columns of n x n planes (rows width floats apart), all but the wrapped first two rows: n / 2 - 1 row pairs
// (a do-while: 0 runs 2^32 times, sic) from src into dst's rows 2.., each over n columns with InverseD4Pairs.
void InverseD4RowPairs(const u8 *src, u8 *dst, u32 width, u32 n) {
    const u32 half = n >> 1, row = width * 4, high = width * half * 4;
    u8 *d = dst + 2 * row;
    u32 rows = half - 1;
    do {
        InverseD4Pairs(src, row, high, 4, d, row, 4, n - 1);
        src += row;
        d += 2 * row;
    } while (--rows != 0);
}

}  // namespace

extern "C" {

u32 __cdecl Wavelet_Shutdown() {
    const u32 block = global<u32>(0x00879a68);
    if (!block)
        return 0;
    TrackedGlobal_Free(block);
    u8 *codec = global<u8 *>(0x00879a64);
    global<u32>(0x00879a68) = 0;
    if (codec)
        vcall<void>(codec, 0x18);
    return 1;
}

void __stdcall WaveletCodec_PutCoefficient(u32 value, u8 **cursor) {
    s16 v = (s16)value;
    if (v >= -0x7f && v <= 0x7f) {
        **cursor = (u8)v;
        ++*cursor;
        return;
    }
    **cursor = 0x80;
    u8 *word = *cursor + 1;
    *cursor = word;
    *reinterpret_cast<u16 *>(word) = (u16)v;
    *cursor += 2;
}

u32 __cdecl Wavelet_ReadBits(u32 count) {
    u32 held = global<u32>(0x00aa67f8);
    if (held < count) {
        const u32 position = global<u32>(0x00be6800);
        const u32 word = at<u32>(reinterpret_cast<const void *>(position), global<u32>(0x00e74a40)) & 0xffffu;
        global<u32>(0x00be6800) = position + 2;
        global<u32>(0x00f74a48) |= word << (held & 31);
        held += 0x10;
    }
    held -= count;
    global<u32>(0x00aa67f8) = held;
    const u32 accumulator = global<u32>(0x00f74a48);
    const u32 result = ((1u << (count & 31)) - 1u) & accumulator;
    global<u32>(0x00f74a48) = accumulator >> (count & 31);
    return result;
}

u32 __fastcall WaveletCodec_Decode(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j) {
    u8 *const base = codec - 0x54;
    if (version == 0x12)
        return vcall<u32>(base, 0, a, b, c, d, e, f, g, h, i, j);
    if (version == 0x13)
        return vcall<u32>(base, 8, a, b, c, d, e, f, g, h, i, j);
    return (u32)(version - 0x13);  // (what the dispatch left in EAX)
}

u32 __fastcall WaveletCodec_DecodeScaled(u8 *codec, void *, s32 version, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l, u32 m) {
    if (version > 0x12)
        return vcall<u32>(codec - 0x54, 4, a, b, c, d, e, f, g, h, i, j, k, l, m);
    return 0;
}

void __fastcall WaveletCodec_VBaseDtor(u8 *base) {
    // The SEH frame is left out.
    at<u32>(base, -0x54) = 0x7026d8;
    const u32 offset = at<u32>(at<u8 *>(base, -0x50), 4);
    at<u32>(base, offset - 0x50) = 0x7026b8;
    at<u32>(base, offset - 0x54) = offset - 0x50;
    for (u32 i = 1; i <= 5; i++) {
        if (void *const block = at<void *>(base, -0x10 * i + 8))
            GameHeap_Free(block);
    }
}

void __fastcall WaveletCodec_UnpackBgr(u8 *codec, void *, u32 width, u32 height, u32 stride, const u8 *src) {
    if (height == 0)
        return;
    u32 row = 0;  // byte offset of the row in the planes (wraps as the original's does)
    do {
        u32 offset = row;
        for (u32 x = width; x != 0; --x) {
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x2c) + offset) = (float)*src++;
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x1c) + offset) = (float)*src++;
            *reinterpret_cast<float *>(at<u8 *>(codec, 0x0c) + offset) = (float)*src++;
            offset += 4;
        }
        row += stride * 4;
    } while (--height != 0);
}

void __fastcall WaveletCodec_RgbToYuv(u8 *codec, void *, u32 count) {
    for (u32 i = 0; i < count; ++i) {
        float *const bp = at<float *>(codec, 0x2c) + i;
        const X87Reg g = at<float *>(codec, 0x1c)[i];
        const X87Reg r = at<float *>(codec, 0x0c)[i];
        const X87Reg b = *bp;
        *bp = (float)(((g * 0.587f) - (b * -0.114f)) - (r * -0.299f));                // 0x702664, 0x702668, 0x70266c
        at<float *>(codec, 0x1c)[i] = (float)((b * 0.5f) - ((r * 0.168736f) - (g * -0.331264f)));  // 0x702670, 0x702674, 0x702678
        at<float *>(codec, 0x0c)[i] = (float)(((r * 0.5f) - (g * 0.418688f)) - (b * 0.081312f));   // 0x70267c, 0x702680
    }
}

void __fastcall WaveletCodec_ForwardD4Row(u8 *, void *, const float *src, float *dst, s32 n) {
    if (n <= 0)
        return;
    const u32 mask = (u32)(n - 1);
    float *high = dst + (n >> 1);
    const float *pair = src;  // s[2k], s[2k + 1]; s[2k + 2] and s[2k + 3] wrap around the row (n a power of two)
    s32 c = 3;
    do {
        const X87Reg h1s1 = X87Reg(global<float>(0x00c2720c)) * pair[1];
        const X87Reg h2s2 = X87Reg(global<float>(0x00c27210)) * src[(u32)(c - 1) & mask];
        const X87Reg h3s3 = X87Reg(global<float>(0x00c27214)) * src[(u32)c & mask];
        const X87Reg h0s0 = X87Reg(global<float>(0x00c27208)) * pair[0];
        *dst++ = (float)(((h1s1 + h2s2) + h3s3) + h0s0);
        const X87Reg h3s0 = X87Reg(global<float>(0x00c27214)) * pair[0];
        const X87Reg h2s1 = X87Reg(global<float>(0x00c27210)) * pair[1];
        const X87Reg h1s2 = X87Reg(global<float>(0x00c2720c)) * src[(u32)(c - 1) & mask];
        const X87Reg h0s3 = X87Reg(global<float>(0x00c27208)) * src[(u32)c & mask];
        *high++ = (float)(((h3s0 - h2s1) + h1s2) - h0s3);
        pair += 2;
        c += 2;
    } while (c - 3 < n);
}

void __fastcall WaveletCodec_HalvePlane(u8 *, void *, float *const *plane, u32 n) {
    const u32 half = n >> 1;
    if (half == 0)
        return;
    u32 top = 0, bottom = n * 4, out = 0;  // byte offsets (wrapping as the original's)
    for (u32 y = half; y != 0; --y) {
        for (u32 x = 0; x < half; ++x) {
            const u8 *const b = reinterpret_cast<const u8 *>(*plane);
            const X87Reg sum = ((X87Reg(at<float>(b, bottom + 8 * x)) + at<float>(b, bottom + 8 * x + 4)) + at<float>(b, top + 8 * x + 4)) +
                               at<float>(b, top + 8 * x);
            at<float>(b, out + 4 * x) = (float)(sum * 0.25f);  // 0x702684
        }
        top += n * 8;
        bottom += n * 8;
        out += half * 4;
    }
}

void __fastcall WaveletCodec_ForwardD4(u8 *codec, void *, float *const *plane, u32 stride, u32 n) {
    // The SEH frame is left out. Four {data, size, max, grow by} arrays on the stack: a row in and out, a column in and out.
    struct Array {
        u32 *data;
        u32 size, max, growBy;
    } rowIn = {}, rowOut = {}, colIn = {}, colOut = {};
    if (n != 0) {
        rowIn.data = static_cast<u32 *>(GameHeap_Alloc(n * 4));
        Dwords_Zero(rowIn.data, n);
        rowIn.max = n;
        rowIn.size = n;
    }
    IntArray_SetSize(reinterpret_cast<u8 *>(&rowOut), nullptr, (s32)n, -1);
    IntArray_SetSize(reinterpret_cast<u8 *>(&colIn), nullptr, (s32)n, -1);
    IntArray_SetSize(reinterpret_cast<u8 *>(&colOut), nullptr, (s32)n, -1);
    for (u32 col = 0; col < n; ++col) {
        u8 *p = reinterpret_cast<u8 *>(*plane) + col * 4;
        for (u32 i = 0; i < n; ++i, p += stride * 4)
            colIn.data[i] = *reinterpret_cast<u32 *>(p);
        // The column's D4 step, inline: as WaveletCodec_ForwardD4Row but the low-pass sum in filter order.
        if ((s32)n > 0) {
            const float *const src = reinterpret_cast<const float *>(colIn.data), *pair = src;
            float *low = reinterpret_cast<float *>(colOut.data), *high = low + ((s32)n >> 1);
            const u32 mask = n - 1;
            s32 c = 3;
            do {
                const X87Reg h0s0 = X87Reg(global<float>(0x00c27208)) * pair[0];
                const X87Reg h1s1 = X87Reg(global<float>(0x00c2720c)) * pair[1];
                const X87Reg h2s2 = X87Reg(global<float>(0x00c27210)) * src[(u32)(c - 1) & mask];
                const X87Reg h3s3 = X87Reg(global<float>(0x00c27214)) * src[(u32)c & mask];
                *low++ = (float)(((h0s0 + h1s1) + h2s2) + h3s3);
                const X87Reg h3s0 = X87Reg(global<float>(0x00c27214)) * pair[0];
                const X87Reg h2s1 = X87Reg(global<float>(0x00c27210)) * pair[1];
                const X87Reg h1s2 = X87Reg(global<float>(0x00c2720c)) * src[(u32)(c - 1) & mask];
                const X87Reg h0s3 = X87Reg(global<float>(0x00c27208)) * src[(u32)c & mask];
                *high++ = (float)(((h3s0 - h2s1) + h1s2) - h0s3);
                pair += 2;
                c += 2;
            } while (c - 3 < (s32)n);
        }
        p = reinterpret_cast<u8 *>(*plane) + col * 4;
        for (u32 i = 0; i < n; ++i, p += stride * 4)
            *reinterpret_cast<u32 *>(p) = colOut.data[i];
    }
    if (n != 0) {
        u32 *row = reinterpret_cast<u32 *>(*plane);
        for (u32 left = n; left != 0; --left) {
            for (u32 i = 0; i < n; ++i)
                rowIn.data[i] = row[i];
            WaveletCodec_ForwardD4Row(codec, nullptr, reinterpret_cast<const float *>(rowIn.data), reinterpret_cast<float *>(rowOut.data), (s32)n);
            for (u32 i = 0; i < n; ++i)
                row[i] = rowOut.data[i];
            row = reinterpret_cast<u32 *>(reinterpret_cast<u8 *>(row) + stride * 4);
        }
    }
    if (colOut.data)
        GameHeap_Free(colOut.data);
    if (rowOut.data)
        GameHeap_Free(rowOut.data);
    Ptr_FreeIfSet(reinterpret_cast<u8 **>(&colIn.data));
    Ptr_FreeIfSet(reinterpret_cast<u8 **>(&rowIn.data));
}

u32 __fastcall WaveletCodec_EncodeGrey(u8 *codec, void *, u32 width, u32 height, const u8 *pixels, void *dst, u32 *outSize, float q) {
    const u32 w = SideFor(width), h = SideFor(height);
    const u32 size = w > h ? w : h, count = size * size;
    u8 *const plane = codec + 0x2c;
    IntArray_SetSize(plane, nullptr, (s32)count, -1);  // inline in the original, the same steps
    ZeroArray(plane);
    u32 k = 0;
    u8 *row = at<u8 *>(plane, 0);
    for (u32 y = height; y != 0; --y, row += size * 4) {
        float *p = reinterpret_cast<float *>(row);
        for (u32 x = width; x != 0; --x)
            *p++ = (float)pixels[k++];
    }
    WaveletCodec_ForwardD4(codec, nullptr, reinterpret_cast<float *const *>(plane), size, size);
    const float inverse = (float)(X87Reg(1.0f) / q);  // 0x702660
    for (u32 i = 0; i < count; ++i) {
        float *const v = at<float *>(plane, 0) + i;
        *v = (float)(X87Reg(inverse) * *v);
    }
    u8 *out = kStream;
    for (u32 i = 0; i < count; ++i) {
        const s16 v = (s16)FtolLow(at<float *>(plane, 0)[i]);
        if (v >= -0x7f && v <= 0x7f) {
            *out++ = (u8)v;
        } else {
            *out++ = 0x80;
            *reinterpret_cast<s16 *>(out) = v;
            out += 2;
        }
    }
    const u32 packed = Buffer_Compress(kStream, dst, (u32)(out - kStream), at<void *>(codec, 0x4c));
    *outSize = packed;
    return packed;
}

u32 __fastcall WaveletCodec_Encode(u8 *iface, void *, u32 width, u32 height, u32 colour, const u8 *pixels, void *dst, u32 *outSize, float qY, float qCb,
                                   float qCr) {
    u8 *const codec = iface - 0x54;
    if ((u8)colour == 0)
        return WaveletCodec_EncodeGrey(codec, nullptr, width, height, pixels, dst, outSize, qY);
    u32 size = SideFor(width);
    const u32 h = SideFor(height);
    if (size <= h)
        size = h;
    const u32 count = size * size, quarter = count >> 2;
    IntArray_SetSize(codec + 0xc, nullptr, (s32)(count + 1), -1);
    IntArray_SetSize(codec + 0x1c, nullptr, (s32)(count + 1), -1);
    IntArray_SetSize(codec + 0x2c, nullptr, (s32)(count + 1), -1);
    ZeroArray(codec + 0xc);
    ZeroArray(codec + 0x1c);
    ZeroArray(codec + 0x2c);
    WaveletCodec_UnpackBgr(codec, nullptr, width, height, size, pixels);
    WaveletCodec_RgbToYuv(codec, nullptr, count);
    WaveletCodec_HalvePlane(codec, nullptr, reinterpret_cast<float *const *>(codec + 0x1c), size);
    WaveletCodec_HalvePlane(codec, nullptr, reinterpret_cast<float *const *>(codec + 0xc), size);
    WaveletCodec_ForwardD4(codec, nullptr, reinterpret_cast<float *const *>(codec + 0x2c), size, size);
    const u32 half = size >> 1;
    WaveletCodec_ForwardD4(codec, nullptr, reinterpret_cast<float *const *>(codec + 0x1c), half, half);
    WaveletCodec_ForwardD4(codec, nullptr, reinterpret_cast<float *const *>(codec + 0xc), half, half);
    const float inverseY = (float)(X87Reg(1.0f) / qY), inverseCb = (float)(X87Reg(1.0f) / qCb), inverseCr = (float)(X87Reg(1.0f) / qCr);  // 0x702660
    for (u32 i = 0; i < count; ++i) {
        float *const v = at<float *>(codec, 0x2c) + i;
        *v = (float)(X87Reg(inverseY) * *v);
    }
    for (u32 i = 0; i < quarter; ++i) {
        float *const cb = at<float *>(codec, 0x1c) + i;
        *cb = (float)(X87Reg(inverseCb) * *cb);
        float *const cr = at<float *>(codec, 0xc) + i;
        *cr = (float)(X87Reg(inverseCr) * *cr);
    }
    u8 *cursor = kStream;
    for (u32 i = 0; i < count; ++i)
        WaveletCodec_PutCoefficient(FtolLow(at<float *>(codec, 0x2c)[i]), &cursor);
    for (u32 i = 0; i < quarter; ++i)
        WaveletCodec_PutCoefficient(FtolLow(at<float *>(codec, 0x1c)[i]), &cursor);
    for (u32 i = 0; i < quarter; ++i)
        WaveletCodec_PutCoefficient(FtolLow(at<float *>(codec, 0xc)[i]), &cursor);
    const u32 packed = Buffer_Compress(kStream, dst, (u32)(cursor - kStream), at<void *>(codec, 0x4c));
    *outSize = packed;
    return packed;
}

void __fastcall WaveletCodec_ReadCoefficients(u8 *, void *, const u8 **cursor, float *dst, u32 count, float q) {
    if (global<u32>(0x00f75ad4) == 0) {
        for (u32 i = 0; i < 0x100; ++i) {
            const float s = (float)(s8)i;
            global<float>(0x00fb2be0 + 4 * i) = s;
            global<float>(0x00fb2fe0 + 4 * i) = (float)i;
            global<float>(0x00fb27e0 + 4 * i) = (float)(X87Reg(s) * 256.0f);  // 0x702688
        }
        global<u32>(0x00f75ad4) = 1;
    }
    const u8 *p = *cursor;
    do {
        const u8 b = *p++;
        if (b != 0x80) {
            *dst++ = (float)(X87Reg(global<float>(0x00fb2be0 + 4 * b)) * q);
        } else {
            const u8 low = p[0], high = p[1];
            p += 2;
            *dst++ = (float)((X87Reg(global<float>(0x00fb2fe0 + 4 * low)) + global<float>(0x00fb27e0 + 4 * high)) * q);
        }
    } while (--count != 0);
    *cursor = p;
}

void __fastcall WaveletCodec_InverseD4Row(u8 *, void *, const float *src, float *dst, s32 n) {
    const s32 half = n >> 1, m = half - 1;
    const float *const low = src, *const high = src + half;
    if (n >= 6) {
        InverseD4Pairs(reinterpret_cast<const u8 *>(src), 4, (u32)half * 4, 4, reinterpret_cast<u8 *>(dst + 2), 4, 8, (u32)(m - 1));
    } else {
        for (s32 k = 0; k < m; ++k) {
            dst[2 * k + 2] = (float)(((X87Reg(H3()) * high[k + 1] + X87Reg(H2()) * low[k]) + X87Reg(H0()) * low[k + 1]) + X87Reg(H1()) * high[k]);
            dst[2 * k + 3] = (float)(((X87Reg(H3()) * low[k] + X87Reg(H1()) * low[k + 1]) - X87Reg(H0()) * high[k]) - X87Reg(H2()) * high[k + 1]);
        }
    }
    // The first pair wraps around.
    dst[0] = (float)(((X87Reg(H2()) * low[m] + X87Reg(H1()) * high[m]) + X87Reg(H3()) * high[0]) + X87Reg(low[0]) * H0());
    dst[1] = (float)(((X87Reg(H3()) * low[m] + X87Reg(low[0]) * H1()) - X87Reg(H0()) * high[m]) - X87Reg(H2()) * high[0]);
}

void __fastcall WaveletCodec_InverseD4Columns(u8 *, void *, float *const *srcPlane, float *const *dstPlane, u32 width, u32 n) {
    const u8 *src = reinterpret_cast<const u8 *>(*srcPlane);
    u8 *dst = reinterpret_cast<u8 *>(*dstPlane);
    const u32 half = n >> 1, m = half - 1, row = width * 4, high = width * half * 4;
    InverseD4RowPairs(src, dst, width, n);
    // The first two rows wrap around; each value is read when its expression needs it.
    const u32 lastLow = m * width * 4, lastHigh = (m * width + width * half) * 4;
    for (u32 c = n; c != 0; --c) {
        at<float>(dst, 0) = (float)(((X87Reg(H3()) * at<float>(src, high) + X87Reg(H1()) * at<float>(src, lastHigh)) + X87Reg(H2()) * at<float>(src, lastLow)) +
                                    X87Reg(at<float>(src, 0)) * H0());
        at<float>(dst, row) = (float)(((X87Reg(H3()) * at<float>(src, lastLow) + X87Reg(at<float>(src, 0)) * H1()) - X87Reg(H0()) * at<float>(src, lastHigh)) -
                                      X87Reg(H2()) * at<float>(src, high));
        src += 4;
        dst += 4;
    }
}

void __fastcall WaveletCodec_DecodeGrey(u8 *codec, void *, u32 width, u32 height, const void *src, u8 *pixels, float q) {
    // The SEH frame is left out.
    const u32 w = SideFor(width), h = SideFor(height);
    const u32 side = w > h ? w : h, count = side * side;
    Buffer_Decompress(src, kStream, at<void *>(codec, 0x4c));
    IntArray_SetSize(codec + 0x2c, nullptr, (s32)count, -1);  // inline in the original, the same steps
    struct {
        u32 *data;
        u32 size, max, growBy;
    } row = {};
    IntArray_SetSize(reinterpret_cast<u8 *>(&row), nullptr, (s32)side, -1);
    // Each row: its coefficients (WaveletCodec_ReadCoefficients, inline there) and the inverse step along it into the plane at +0x3c.
    float *out = at<float *>(codec, 0x3c);
    const u8 *cursor = kStream;
    for (u32 y = side; y != 0; --y) {
        WaveletCodec_ReadCoefficients(codec, nullptr, &cursor, reinterpret_cast<float *>(row.data), side, q);
        WaveletCodec_InverseD4Row(codec, nullptr, reinterpret_cast<float *>(row.data), out, (s32)side);
        out += side;
    }
    // Then down the columns into the plane at +0x2c (WaveletCodec_InverseD4Columns inline, its first wrapped row summed in another order).
    const u8 *const from = at<const u8 *>(codec, 0x3c);
    u8 *const to = at<u8 *>(codec, 0x2c);
    InverseD4RowPairs(from, to, side, side);
    const u32 half = side >> 1, m = half - 1, stride = side * 4, high = side * half * 4;
    const u32 lastLow = m * side * 4, lastHigh = (m * side + side * half) * 4;
    const u8 *a = from;
    u8 *d = to;
    for (u32 c = side; c != 0; --c) {
        at<float>(d, 0) = (float)(((X87Reg(H1()) * at<float>(a, lastHigh) + X87Reg(H2()) * at<float>(a, lastLow)) + X87Reg(H3()) * at<float>(a, high)) +
                                  X87Reg(at<float>(a, 0)) * H0());
        at<float>(d, stride) = (float)(((X87Reg(H3()) * at<float>(a, lastLow) + X87Reg(at<float>(a, 0)) * H1()) - X87Reg(H0()) * at<float>(a, lastHigh)) -
                                       X87Reg(H2()) * at<float>(a, high));
        a += 4;
        d += 4;
    }
    // The pixels: each value below 0 (or a NaN) 0, above 255 0xff, else __ftol's low byte (0x70268c, 0x702690).
    const float *plane = at<const float *>(codec, 0x2c);
    u32 k = 0;
    for (u32 y = height; y != 0; --y) {
        const float *p = plane;
        for (u32 x = width; x != 0; --x, ++p) {
            u8 b;
            if (!(*p >= 0.0f))
                b = 0;
            else if (*p <= 255.0f)
                b = (u8)FtolLow(*p);
            else
                b = 0xff;
            pixels[k++] = b;
        }
        plane += side;
    }
    Ptr_FreeIfSet(reinterpret_cast<u8 **>(&row.data));
}

void __fastcall WaveletCodec_YuvToRgb(u8 *codec, void *, u32 width, u32 height, u32 stride, u8 *dst) {
    const auto bits = [](X87Reg r) {
        const float f = (float)r;
        u32 b;
        std::memcpy(&b, &f, 4);
        return b;
    };
    const auto k = [](u32 b) {
        float f;
        std::memcpy(&f, &b, 4);
        return X87Reg(f);
    };
    const u32 halfH = height >> 1, halfW = width >> 1;
    global<u32>(0x00fb2768) = halfH;
    global<u32>(0x00fb276c) = halfW;
    global<u32>(0x00fb2770) = stride * 2;
    global<u32>(0x00fb2774) = stride * 4;
    global<u32>(0x00fb2778) = stride * 8;
    global<u32>(0x00fb277c) = 0x3fe2d0e5;   // 1.772
    global<u32>(0x00fb2780) = 0x3fb374bc;   // 1.402
    global<u32>(0x00fb2784) = 0x3eb03298;   // 0.344136
    global<u32>(0x00fb2788) = 0x3f36d19e;   // 0.714136
    global<u32>(0x00fb278c) = 0x4b400080;   // 1.5 * 2^23 + 128
    global<u32>(0x00fb2790) = width * 3;
    global<u32>(0x00fb2794) = width * 6;
    global<u32>(0x00fb2798) = 8 - stride * 4;
    global<u32>(0x00fb279c) = 6 - width * 3;
    u8 *const y = at<u8 *>(codec, 0x2c);
    global<u8 *>(0x00fb27a0) = y;
    u8 *const cr = at<u8 *>(codec, 0xc);
    global<u8 *>(0x00fb27a4) = cr;
    u8 *const cb = at<u8 *>(codec, 0x1c);
    global<u8 *>(0x00fb27a8) = cb;
    const u32 diff = (u32)((s32)(cr - cb) >> 2) << 2;
    global<u32>(0x00fb27ac) = diff;
    const u8 *const table = reinterpret_cast<const u8 *>(0x00c27008);
    const X87Reg magic = k(0x4b400080);
    const auto put = [&](u8 *o, X87Reg b0, X87Reg r0, X87Reg g0, X87Reg b1, X87Reg r1, X87Reg g1) {
        global<u32>(0x00fb27b0) = bits(b0);
        global<u32>(0x00fb27b4) = bits(r0);
        global<u32>(0x00fb27b8) = bits(g0);
        global<u32>(0x00fb27bc) = bits(b1);
        global<u32>(0x00fb27c0) = bits(r1);
        global<u32>(0x00fb27c4) = bits(g1);
        const u8 c0 = table[global<u32>(0x00fb27b0) & 0x1ff], c1 = table[global<u32>(0x00fb27b8) & 0x1ff];
        const u8 c2 = table[global<u32>(0x00fb27b4) & 0x1ff], c3 = table[global<u32>(0x00fb27bc) & 0x1ff];
        const u32 i4 = global<u32>(0x00fb27c4) & 0x1ff, i5 = global<u32>(0x00fb27c0) & 0x1ff;
        o[0] = c0;
        o[1] = c1;
        o[2] = c2;
        o[3] = c3;
        o[4] = table[i4];
        o[5] = table[i5];
    };
    u8 *ys = y, *out = dst, *cbs = cb;
    u32 rows = halfH;
    do {   // (do-while: a height below 2 runs 2^32 rows, sic)
        u8 *yp = ys, *o = out, *c = cbs;
        u32 cols = global<u32>(0x00fb276c);
        do {
            const X87Reg a = X87Reg(at<float>(c, 0)) * k(0x3fe2d0e5) + magic;
            const X87Reg b = X87Reg(at<float>(c, diff)) * k(0x3fb374bc) + magic;
            const X87Reg g = X87_SubR(X87Reg(at<float>(c, diff)) * k(0x3f36d19e), X87_SubR(X87Reg(at<float>(c, 0)) * k(0x3eb03298), magic));
            X87Reg y0 = at<float>(yp, 0), y1 = at<float>(yp, 4);
            put(o, y0 + a, y0 + b, y0 + g, y1 + a, y1 + b, y1 + g);
            yp += global<u32>(0x00fb2774);
            o += global<u32>(0x00fb2790);
            y0 = at<float>(yp, 0);
            const X87Reg b2 = y0 + a, r2 = y0 + b, g2 = y0 + g;
            const X87Reg b3 = a + X87Reg(at<float>(yp, 4)), r3 = b + X87Reg(at<float>(yp, 4)), g3 = g + X87Reg(at<float>(yp, 4));
            put(o, b2, r2, g2, b3, r3, g3);
            yp += global<u32>(0x00fb2798);
            o += global<u32>(0x00fb279c);
            c += 4;
        } while (--cols != 0);
        ys += global<u32>(0x00fb2778);
        out += global<u32>(0x00fb2794);
        cbs += global<u32>(0x00fb2770);
    } while (--rows != 0);
    const u8 *const edge = reinterpret_cast<const u8 *>(0x00c27088);
    const X87Reg low = k(0xcb400000);   // -1.5 * 2^23
    const auto pixel = [&](u8 *o, const u8 *yp, const u8 *cbp, const u8 *crp) {
        const X87Reg yv = at<float>(yp, 0), e = at<float>(cbp, 0), c = at<float>(crp, 0);
        o[0] = edge[bits(X87_SubR(e * k(0xbfe2d0e5), yv) - low) & 0x1ff];
        const u32 red = bits(X87_SubR(c * k(0xbfb374bc), yv) - low);
        const u32 green = bits(X87_SubR(e * k(0x3eb03298), yv) - c * k(0x3f36d19e) - low);
        o[1] = edge[green & 0x1ff];
        o[2] = edge[red & 0x1ff];
    };
    if (width & 1) {
        const u8 *yp = y + width * 4 - 4, *crp = cr + halfW * 4 - 4, *cbp = cb + halfW * 4 - 4;
        u8 *o = dst + width * 3 - 3;
        const u32 step = stride >> 1;
        for (s32 i = 0; i < (s32)height; ++i) {
            pixel(o, yp, cbp, crp);
            yp += stride * 4;
            if (i & 1) {
                crp += step * 4;
                cbp += step * 4;
            }
            o += width * 3;
        }
    }
    if (height & 1) {
        const u32 last = height - 1;
        const u8 *yp = y + stride * last * 4;
        const u32 off = ((halfH - 1) * stride >> 1) << 2;
        const u8 *cbp = cb + off, *crp = cr + off;
        u8 *o = dst + width * last * 3;
        for (s32 j = 0; j < (s32)width; ++j) {
            pixel(o, yp, cbp, crp);
            yp += 4;
            if (j & 1) {
                crp += 4;
                cbp += 4;
            }
            o += 3;
        }
    }
}
}
