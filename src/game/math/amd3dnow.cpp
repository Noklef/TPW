#include "amd3dnow.h"

extern "C" {

void __cdecl Amd3DNow_TransformBatch(void *, const float *m, u8 *dest, const u8 *source, u32 destStride, u32 start, u32 count) {
    u32 pairs = (count + 1) >> 1, offset = start << 2;
    const u8 *in = source + (start << 2) + ((start & 0xfffffffc) << 2);
    do {
        const float *const v = reinterpret_cast<const float *>(in);
        const float xa = v[0], xb = v[1], ya = v[4], yb = v[5], za = v[8], zb = v[9];
        const float a0 = ((xa * m[0] + ya * m[4]) + za * m[8]) + m[12], a1 = ((xa * m[1] + ya * m[5]) + za * m[9]) + m[13];
        const float a2 = ((xa * m[2] + ya * m[6]) + za * m[10]) + m[14], a3 = ((xa * m[3] + ya * m[7]) + za * m[11]) + m[15];
        float *out = reinterpret_cast<float *>(dest);
        out[0] = a0;
        out[1] = a1;
        const float m0 = m[0], m1 = m[1];
        out[2] = a2;
        out[3] = a3;
        const float b0 = (zb * m[8] + m[12]) + (m0 * xb + yb * m[4]), b1 = (zb * m[9] + m[13]) + (m1 * xb + yb * m[5]);
        const float b2 = ((xb * m[2] + yb * m[6]) + m[14]) + zb * m[10], b3 = ((xb * m[3] + yb * m[7]) + m[15]) + zb * m[11];
        dest += destStride;
        out = reinterpret_cast<float *>(dest);
        out[0] = b0;
        out[1] = b1;
        out[2] = b2;
        out[3] = b3;
        dest += destStride;
        const u32 next = offset + 0x10;
        in += 8 + (next & 0x20);
        offset = next & 0x10;
    } while (--pairs);
}

void __cdecl Amd3DNow_TransformVectors(void *, const float *m, u8 *dest, const u8 *source, u32 destStride, u32 sourceStride, u32 count, u32 mode) {
    const float k12 = mode != 1 && mode != 3 ? m[12] : 0, k13 = mode != 1 && mode != 3 ? m[13] : 0, k14 = mode != 1 && mode != 3 ? m[14] : 0,
                k15 = mode != 1 && mode != 3 ? m[15] : 0;
    const float k2 = mode == 3 ? m[2] : 0, k10 = mode == 3 ? m[10] : 0;   // (mode 3's once-read column; floats 3 and 11 go unused)
    do {
        const float *const v = reinterpret_cast<const float *>(source);
        float *const out = reinterpret_cast<float *>(dest);
        const float x = v[0], y = v[1], z = v[2];
        if (mode == 1) {
            const float w = v[3];
            const float r0 = ((x * m[0] + y * m[4]) + z * m[8]) + w * m[12], r1 = ((x * m[1] + y * m[5]) + z * m[9]) + w * m[13];
            const float r2 = ((x * m[2] + y * m[6]) + z * m[10]) + w * m[14], r3 = ((x * m[3] + y * m[7]) + z * m[11]) + w * m[15];
            out[0] = r0;
            out[1] = r1;
            out[2] = r2;
            out[3] = r3;
        } else if (mode == 2) {
            const float r0 = ((x * m[0] + y * m[4]) + z * m[8]) + k12, r1 = ((x * m[1] + y * m[5]) + z * m[9]) + k13;
            const float r2 = ((x * m[2] + y * m[6]) + k14) + z * m[10];
            out[0] = r0;
            out[1] = r1;
            out[2] = r2;
        } else if (mode == 3) {
            const float r0 = (x * m[0] + y * m[4]) + z * m[8], r1 = (x * m[1] + y * m[5]) + z * m[9];
            const float r2 = (x * k2 + y * m[6]) + z * k10;
            out[0] = r0;
            out[1] = r1;
            out[2] = r2;
        } else {
            const float r0 = ((x * m[0] + y * m[4]) + z * m[8]) + k12, r1 = ((x * m[1] + y * m[5]) + z * m[9]) + k13;
            const float r2 = ((x * m[2] + y * m[6]) + z * m[10]) + k14, r3 = ((x * m[3] + y * m[7]) + z * m[11]) + k15;
            out[0] = r0;
            out[1] = r1;
            out[2] = r2;
            out[3] = r3;
        }
        dest += destStride;
        source += sourceStride;
    } while (--count);
}

void __cdecl Amd3DNow_Multiply(void *, const float *a, const float *b, float *out) {
    for (int i = 0; i < 16; i += 4) {
        for (int c = 0; c < 4; c += 2) {
            const float first = (a[i] * b[c] + a[i + 2] * b[8 + c]) + (a[i + 1] * b[4 + c] + a[i + 3] * b[12 + c]);
            const float second = (a[i] * b[c + 1] + a[i + 2] * b[9 + c]) + (a[i + 1] * b[5 + c] + a[i + 3] * b[13 + c]);
            out[i + c + 1] = second;
            out[i + c] = first;
        }
    }
}
}
