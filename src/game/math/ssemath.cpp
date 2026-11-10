#include "ssemath.h"

#include <xmmintrin.h>

#include <cstdint>

#include "x87.h"

namespace {

void Expand(__m128 row, float *dest) {
    _mm_stream_ps(dest, _mm_shuffle_ps(row, row, 0x00));
    _mm_stream_ps(dest + 4, _mm_shuffle_ps(row, row, 0x55));
    _mm_stream_ps(dest + 8, _mm_shuffle_ps(row, row, 0xaa));
    _mm_stream_ps(dest + 12, _mm_shuffle_ps(row, row, 0xff));
}

__m128 Row(const float *table, u32 offset) { return _mm_load_ps(table + offset / 4); }

const float *Float(u32 address) { return reinterpret_cast<const float *>((uintptr_t)address); }

// RCPPS of the lanes, refined by one Newton step: 2 r - (w r) r.
__m128 Refined(__m128 w) {
    const __m128 r = _mm_rcp_ps(w);
    return _mm_sub_ps(_mm_add_ps(r, r), _mm_mul_ps(_mm_mul_ps(w, r), r));
}

// A scale (the low lane r, the others its second) for the last of a vertex's four lanes squared: MULSS, then SHUFPS 0x15.
__m128 SquaredLast(__m128 r) {
    r = _mm_mul_ss(r, r);
    return _mm_shuffle_ps(r, r, 0x15);
}

}  // namespace

extern "C" {

void __cdecl SseMath_ExpandMatrix(const float *source, float *dest) {
    __m128 first = _mm_loadu_ps(source), second = _mm_loadu_ps(source + 4);
    Expand(first, dest);
    first = _mm_loadu_ps(source + 8);
    Expand(second, dest + 16);
    second = _mm_loadu_ps(source + 12);
    Expand(first, dest + 32);
    Expand(second, dest + 48);
}

void __cdecl SseMath_TransformBatch(void *, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 start, u32 count) {
    const float *const table = reinterpret_cast<const float *>((uintptr_t)0x007753d0);
    SseMath_ExpandMatrix(matrix, reinterpret_cast<float *>((uintptr_t)0x007753d0));
    u32 batches = (count + 3) >> 2;
    if (!batches) return;
    source += (start & 0xfffffffc) << 2;
    do {
        const __m128 x = _mm_loadu_ps(reinterpret_cast<const float *>(source)), y = _mm_loadu_ps(reinterpret_cast<const float *>(source + 0x10)), z = _mm_loadu_ps(reinterpret_cast<const float *>(source + 0x20));
        const __m128 r0 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(y, Row(table, 0x40)), _mm_mul_ps(z, Row(table, 0x80))), _mm_add_ps(_mm_mul_ps(x, Row(table, 0x00)), Row(table, 0xc0)));
        const __m128 r1 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(z, Row(table, 0x90)), _mm_mul_ps(y, Row(table, 0x50))), _mm_add_ps(_mm_mul_ps(x, Row(table, 0x10)), Row(table, 0xd0)));
        const __m128 r2 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(y, Row(table, 0x60)), _mm_mul_ps(x, Row(table, 0x20))), _mm_add_ps(_mm_mul_ps(z, Row(table, 0xa0)), Row(table, 0xe0)));
        const __m128 r3 = _mm_add_ps(_mm_add_ps(_mm_mul_ps(y, Row(table, 0x70)), _mm_mul_ps(x, Row(table, 0x30))), _mm_add_ps(_mm_mul_ps(z, Row(table, 0xb0)), Row(table, 0xf0)));
        const __m128 low02 = _mm_unpacklo_ps(r0, r2), high02 = _mm_unpackhi_ps(r0, r2), low13 = _mm_unpacklo_ps(r1, r3), high13 = _mm_unpackhi_ps(r1, r3);
        float *const out = reinterpret_cast<float *>(dest);
        _mm_stream_ps(out, _mm_unpacklo_ps(low02, low13));
        _mm_stream_ps(out + 4, _mm_unpackhi_ps(low02, low13));
        _mm_stream_ps(out + 8, _mm_unpacklo_ps(high02, high13));
        _mm_stream_ps(out + 12, _mm_unpackhi_ps(high02, high13));
        dest += destStride << 2;
        source += 0x30;
    } while (--batches);
}

void __cdecl SseMath_TransformVectors(void *, const float *matrix, u8 *dest, const u8 *source, u32 destStride, u32 sourceStride, u32 count, u32 mode) {
    const __m128 m0 = _mm_loadu_ps(matrix), m1 = _mm_loadu_ps(matrix + 4), m2 = _mm_loadu_ps(matrix + 8);
    const __m128 m3 = mode == 3 ? m2 : _mm_loadu_ps(matrix + 12);   // (mode 3 reads no bottom row)
    do {
        const float *const in = reinterpret_cast<const float *>(source);
        float *const out = reinterpret_cast<float *>(dest);
        if (mode == 1) {
            const __m128 v = _mm_load_ps(in);
            const __m128 a = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v, v, 0x00), m0), _mm_mul_ps(_mm_shuffle_ps(v, v, 0x55), m1));
            const __m128 b = _mm_add_ps(_mm_mul_ps(_mm_shuffle_ps(v, v, 0xaa), m2), _mm_mul_ps(_mm_shuffle_ps(v, v, 0xff), m3));
            _mm_storeu_ps(out, _mm_add_ps(a, b));
        } else {
            const __m128 xy = _mm_loadl_pi(_mm_setzero_ps(), reinterpret_cast<const __m64 *>(in));
            const __m128 x = _mm_mul_ps(_mm_shuffle_ps(xy, xy, 0x00), m0), y = _mm_mul_ps(_mm_shuffle_ps(xy, xy, 0x55), m1), z = _mm_mul_ps(_mm_load1_ps(in + 2), m2);
            __m128 r;
            if (mode == 2)
                r = _mm_add_ps(z, _mm_add_ps(_mm_add_ps(x, m3), y));
            else if (mode == 3)
                r = _mm_add_ps(z, _mm_add_ps(x, y));
            else
                r = _mm_add_ps(_mm_add_ps(y, z), _mm_add_ps(x, m3));
            if (mode == 2 || mode == 3) {
                _mm_storel_pi(reinterpret_cast<__m64 *>(out), r);
                _mm_store_ss(out + 2, _mm_shuffle_ps(r, r, 0xaa));
            } else {
                _mm_storeu_ps(out, r);
            }
        }
        dest += destStride;
        source += sourceStride;
    } while (--count);
}

void __cdecl SseMath_Multiply(void *, const float *a, const float *b, float *out) {
    const __m128 b0 = _mm_loadu_ps(b), b1 = _mm_loadu_ps(b + 4), b2 = _mm_loadu_ps(b + 8), b3 = _mm_loadu_ps(b + 12);
    // Each of a's floats read where the original reads it, some ahead of the row stored before them: out may lie on a.
    const __m128 r0 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_load1_ps(a), b0), _mm_mul_ps(_mm_load1_ps(a + 1), b1)), _mm_mul_ps(_mm_load1_ps(a + 2), b2)), _mm_mul_ps(_mm_load1_ps(a + 3), b3));
    const __m128 a4 = _mm_load1_ps(a + 4), a5 = _mm_load1_ps(a + 5);
    _mm_storeu_ps(out, r0);
    const __m128 r1 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(a4, b0), _mm_mul_ps(a5, b1)), _mm_mul_ps(_mm_load1_ps(a + 6), b2)), _mm_mul_ps(_mm_load1_ps(a + 7), b3));
    const __m128 a8 = _mm_load1_ps(a + 8), a9 = _mm_load1_ps(a + 9);
    _mm_storeu_ps(out + 4, r1);
    const __m128 r2 = _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(a8, b0), _mm_mul_ps(a9, b1)), _mm_mul_ps(_mm_load1_ps(a + 10), b2)), _mm_mul_ps(_mm_load1_ps(a + 11), b3));
    const __m128 a13 = _mm_load1_ps(a + 13);
    _mm_storel_pi(reinterpret_cast<__m64 *>(out + 8), r2);
    const __m128 a12 = _mm_load1_ps(a + 12);
    _mm_storeh_pi(reinterpret_cast<__m64 *>(out + 10), r2);
    _mm_storeu_ps(out + 12, _mm_add_ps(_mm_add_ps(_mm_add_ps(_mm_mul_ps(a12, b0), _mm_mul_ps(a13, b1)), _mm_mul_ps(_mm_load1_ps(a + 14), b2)), _mm_mul_ps(_mm_load1_ps(a + 15), b3)));
}

double __cdecl SseMath_ProjectToScreen(void *, u8 *v, s32 stride, u32 count, u32 mode) {
    const u32 times = count;
    __m128 scale = _mm_loadl_pi(_mm_setzero_ps(), reinterpret_cast<const __m64 *>((uintptr_t)0x008bcc00));
    __m128 offset = _mm_loadl_pi(_mm_setzero_ps(), reinterpret_cast<const __m64 *>((uintptr_t)0x008bcbf0));
    if (mode == 1) {
        __m128 k = _mm_load_ss(Float(0x008bcc3c));
        offset = _mm_add_ps(offset, _mm_shuffle_ps(k, k, 0x45));
        do {
            const __m128 p = _mm_loadu_ps(reinterpret_cast<const float *>(v));
            const __m128 r = Refined(_mm_shuffle_ps(p, p, 0xff));
            const __m128 s = _mm_add_ps(_mm_mul_ps(_mm_mul_ps(p, scale), r), offset);
            _mm_storeu_ps(reinterpret_cast<float *>(v), _mm_shuffle_ps(s, _mm_shuffle_ps(r, offset, 0xaa), 0x64));
            v += stride;
        } while (--count);
        return X87_MulIntDouble(static_cast<s32>(times), *Float(0x008bcc3c));
    }
    __m128 sum = _mm_setzero_ps();
    if (mode == 2) {
        __m128 k = _mm_load_ss(Float(0x008bcc20));
        scale = _mm_add_ps(scale, _mm_shuffle_ps(k, k, 0x45));
        do {
            const __m128 p = _mm_loadu_ps(reinterpret_cast<const float *>(v));
            const __m128 s = _mm_mul_ps(p, scale);
            const __m128 r = _mm_rcp_ps(_mm_shuffle_ps(p, p, 0xff));
            const __m128 out = _mm_shuffle_ps(_mm_add_ps(_mm_mul_ps(s, r), offset), _mm_unpackhi_ps(r, s), 0x94);
            sum = _mm_add_ps(sum, out);
            _mm_storel_pi(reinterpret_cast<__m64 *>(v), out);
            _mm_storeh_pi(reinterpret_cast<__m64 *>(v + 8), out);
            v += stride;
        } while (--count);
    } else {
        const __m128 nearZ = _mm_add_ps(_mm_load_ss(Float(0x008bcc34)), _mm_load_ps(Float(0x007754d0)));
        const __m128 farZ = _mm_add_ps(_mm_load_ss(Float(0x008bcc2c)), _mm_load_ps(Float(0x007754d0)));
        const __m128 nearScale = _mm_shuffle_ps(scale, nearZ, 0x44), nearOffset = _mm_shuffle_ps(offset, _mm_load_ss(Float(0x008bcc38)), 0x44);
        const __m128 farScale = _mm_shuffle_ps(scale, farZ, 0x44), farOffset = _mm_shuffle_ps(offset, _mm_load_ss(Float(0x008bcc30)), 0x44);
        const u32 limit = *reinterpret_cast<const u32 *>((uintptr_t)0x008bcc28);
        do {
            const bool far = limit < *reinterpret_cast<const u32 *>(v + 8);
            const __m128 p = _mm_loadu_ps(reinterpret_cast<const float *>(v));
            const __m128 w = _mm_shuffle_ps(p, p, 0xff);
            const __m128 out = far ? _mm_add_ps(_mm_mul_ps(_mm_mul_ps(p, farScale), SquaredLast(Refined(w))), farOffset)
                                   : _mm_add_ps(_mm_mul_ps(_mm_mul_ps(p, nearScale), SquaredLast(_mm_rcp_ps(w))), nearOffset);
            _mm_storel_pi(reinterpret_cast<__m64 *>(v), out);
            _mm_storeh_pi(reinterpret_cast<__m64 *>(v + 8), out);
            sum = _mm_add_ps(sum, out);
            v += stride;
        } while (--count);
    }
    _mm_store_ss(reinterpret_cast<float *>((uintptr_t)0x007754f0), _mm_shuffle_ps(sum, sum, 0xaa));
    return *Float(0x007754f0);
}
}
