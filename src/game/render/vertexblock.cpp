#include "vertexblock.h"

#include <cstdint>

#include "../math/sintable.h"
#include "../math/x87.h"
#include "../terrain/land.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

// the sine table's entry for (the float at 0x874fc0 + phase) * its scale (stored as a float, then fistp)
float ripple(float phase) {
    const SinTable sines = SinTable_Game();
    return SinTable_At(sines, (float)((X87Reg(*reinterpret_cast<const float *>(0x00874fc0)) + phase) * sines.scale));
}

}  // namespace

extern "C" {

void __fastcall VertexBlock_Add(u8 *block, void *, float x, float y, float z, float u, float v, u32 wave, const float *normal) {
    if (wave) {
        y = (float)(X87Reg(Terrain_HeightAt(x, z)) + y);
        const float phase = (float)(X87Reg(z) + x);
        y = (float)(X87Reg(ripple(phase)) + y);
        const float lift = ripple(phase);  // (looked up again)
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = 0.0f;
        n[1] = (float)((X87Reg(lift) - -1.0f) * 0.5f);  // 0x700e28, 0x700de4
        n[2] = 0.0f;
    } else if (normal) {
        const float nz = X87_Through(normal[2]), ny = X87_Through(normal[1]), nx = X87_Through(normal[0]);
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = nx;
        n[1] = ny;
        n[2] = nz;
    } else {
        float *const n = at<float *>(block, 0x68) + at<u32>(block, 0x9c) * 3;
        n[0] = 0.0f;
        n[1] = 1.0f;
        n[2] = 0.0f;
    }
    u32 count = at<u32>(block, 0x9c);
    float *const p = at<float *>(block, 0x64) + (count >> 2) * 12 + (count & 3);
    p[0] = X87_Through(x);
    p[4] = X87_Through(y);
    p[8] = X87_Through(z);
    count = at<u32>(block, 0x9c);
    float *const t = at<float *>(block, 0x6c) + (count >> 2) * 8 + (count & 3);
    t[0] = X87_Through(u);
    t[4] = X87_Through(v);
    at<u32>(block, 0x9c) = at<u32>(block, 0x9c) + 1;
}
}
