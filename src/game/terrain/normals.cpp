#include "normals.h"

#include <cstdint>

#include "../math/matrix.h"
#include "../math/x87.h"
#include "../model/worldverts.h"
#include "../render/state.h"
#include "../world/mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u8 *grid() { return at<const u8 *>(global<const u8 *>(0x007a0854), 0x6c); }

}  // namespace

extern "C" {

void __cdecl Terrain_ScaleNormals(u8 *node, TerrainMatrix m) {
    Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
    if (!(at<u32>(node, 0) & 0x200)) {
        const u32 width = at<u32>(grid(), 0x18), height = at<u32>(grid(), 0x1c);
        const u16 *index = at<const u16 *>(node, 0x94);
        WorldVerts_TransformNode(node, m.m);
        float *n = at<float *>(node, 0x64);
        for (u32 i = at<u16>(node, 0x5e); i; --i, n += 3, ++index) {
            const float *const v = global<const float *>(0x007aecd4) + 3 * (u32)*index;
            const u32 cx = (u32)(long long)(X87Reg(v[0]) * X87Reg(0.100000001f)).v;  // 0x6fe3a0, __ftol
            const u32 cz = (u32)(long long)(X87Reg(v[2]) * X87Reg(0.100000001f)).v;
            if (cx >= width || cz >= height) continue;
            const bool shaded = global<const u32 *>(0x007a0884)[cz * width + cx] & 0x400;
            const float length = shaded ? global<const float>(0x0074cf30) : 1.0f;  // 0x6fe380
            const X87Reg n0 = n[0];
            const X87Reg s = (n0 * n0 + X87Reg(n[1]) * X87Reg(n[1])) + X87Reg(n[2]) * X87Reg(n[2]);
            const float t = (float)X87_Sqrt(s);
            const X87Reg k = X87Reg(length) / X87Reg(t);
            const float kf = (float)k;
            n[0] = (float)(n0 * X87Reg(kf));
            n[1] = (float)(k * X87Reg(n[1]));
            n[2] = (float)(X87Reg(kf) * X87Reg(n[2]));
        }
    }
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8)) Terrain_ScaleNormals(child, m);
}

void __cdecl Terrain_ShadeNormals() {
    u8 *flags = at<u8 *>(grid(), 0x2c);
    for (u32 row = 0; row < at<u32>(grid(), 0x1c); ++row) {
        for (u32 column = 0; column < at<u32>(grid(), 0x18); ++column, flags += 4) {
            s32 x = (s32)column, z = (s32)row;
            if (x < 0) x = 0;
            if (z < 0) z = 0;
            if (x >= global<s32>(0x007a0314)) x = global<s32>(0x007a0314) - 1;
            if (z >= global<s32>(0x007a0374)) z = global<s32>(0x007a0374) - 1;
            const u32 id = ((u32)(z << 7) + (u32)x + 1) & 0xffff;
            const u8 *const cell = *global<const u8 *const *>(0x007cf6ec) + 0x294 + id * 0x44;
            if (MapCell_HasFlags(cell, nullptr, 0x40) && !MapCell_HasFlags(cell, nullptr, 0x80))
                flags[1] |= 4;
            else
                at<u16>(flags, 0) &= 0xfbff;
        }
    }
    TerrainMatrix identity;
    Matrix_Identity(identity.m);
    Terrain_ScaleNormals(at<u8 *>(global<const u8 *>(0x007a0854), 0x78), identity);
    SceneFlags_Touch(reinterpret_cast<u32 *>(0x008bd508));
    global<u32>(0x0079f7cc) = 0;
}
}
