#include "nodecells.h"

#include <cstdint>
#include <cstring>
#include <utility>

#include "../math/matrix.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 ftol(X87Reg v) { return (s32)(long long)v.v; }

}  // namespace

extern "C" {

void __cdecl Terrain_NodeCells(u8 *node, const u8 *grid, TerrainMatrix m) {
    Matrix_PreMul(reinterpret_cast<const float *>(node + 0x10), m.m);
    if (!(at<u32>(node, 0) & 0x200)) {
        float lo[3], hi[3];
        std::memcpy(lo, node + 0x78, sizeof lo);
        std::memcpy(hi, node + 0x84, sizeof hi);
        Matrix_TransformPoint(m.m, lo);
        Matrix_TransformPoint(m.m, hi);
        for (int i = 0; i < 3; ++i)
            if (lo[i] > hi[i]) std::swap(lo[i], hi[i]);
        const X87Reg ox = at<float>(grid, 8), oz = at<float>(grid, 0xc), sx = at<float>(grid, 0x10), sz = at<float>(grid, 0x14);
        const X87Reg round = -0.999000013f;   // 0x6feb70
        s32 x0 = ftol((X87Reg(lo[0]) - ox) / sx);
        if (!(lo[0] >= 0.0f)) --x0;   // sic: by the corner's sign (0x6feb6c), not the cell's
        s32 z0 = ftol((X87Reg(lo[2]) - oz) / sz);
        if (!(lo[2] >= 0.0f)) --z0;
        s32 x1 = ftol((X87Reg(hi[0]) - ox) / sx - round);
        s32 z1 = ftol((X87Reg(hi[2]) - oz) / sz - round);
        if (x0 < 0 || z0 < 0 || x1 >= at<s32>(grid, 0x18) || z1 >= at<s32>(grid, 0x1c)) at<u32>(node, 0) |= 0x4000000;
        if (x0 < -0x80) x0 = -0x80;
        if (x1 > 0x7f) x1 = 0x7f;
        if (z0 < -0x80) z0 = -0x80;
        if (z1 > 0x7f) z1 = 0x7f;
        at<u32>(node, 0) |= 0x2000;
        at<u32>(node, 0x90) = (u32)(u8)x0 << 24 | (u32)(u8)x1 << 16 | (u32)(u8)z0 << 8 | (u8)z1;
    }
    for (u8 *child = at<u8 *>(node, 0xc); child; child = at<u8 *>(child, 8))
        if (!(at<u32>(child, 0) & 0x40000000)) Terrain_NodeCells(child, grid, m);
}

void __cdecl Terrain_ModelCells(const u8 *map) {
    TerrainMatrix identity;
    Matrix_Identity(identity.m);
    Terrain_NodeCells(at<u8 *>(map, 0x78), at<const u8 *>(map, 0x6c), identity);
}
}
