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

u32 __fastcall VertexBlock_AddQuad(u8 *block, void *, const u32 *positions, u32, const u32 *uvs, const u8 *normals) {
    // the floats go over as their bits, as the original pushes them (no x87 load to quieten a signalling NaN)
    using AddBits = void(__fastcall *)(u8 *, void *, u32, u32, u32, u32, u32, u32, const u8 *);
    const AddBits add = reinterpret_cast<AddBits>(&VertexBlock_Add);
    const u32 base = at<u32>(block, 0x9c);
    for (u32 i = 0; i < 4; i++)
        add(block, nullptr, positions[3 * i], positions[3 * i + 1], positions[3 * i + 2], uvs[3 * i], uvs[3 * i + 1], 0, normals + 0xc * i);
    u8 *list = at<u8 *>(block, 0x94);
    at<u16>(list, 2) = (u16)(base + 2);
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)(base + 1);
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)base;
    list = at<u8 *>(block, 0x94) + 8;
    u32 triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    at<u16>(list, 2) = (u16)(base + 1);
    at<u16>(at<u8 *>(block, 0x94), 4) = (u16)(base + 2);
    at<u16>(at<u8 *>(block, 0x94), 6) = (u16)(base + 3);
    list = at<u8 *>(block, 0x94) + 8;
    triangles = at<u32>(block, 0x98) + 1;
    at<u8 *>(block, 0x94) = list;
    at<u32>(block, 0x98) = triangles;
    return triangles;
}
}
