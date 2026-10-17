#include "heightgrid.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

template <typename T> const T &at(const void *p, u32 offset) { return *reinterpret_cast<const T *>((uintptr_t)p + offset); }

// __ftol: truncated to 64 bits, the low half.
s32 Ftol(X87Reg x) { return (s32)(long long)x.v; }

s32 Clamp(s32 v, s32 hi) { return v > hi ? hi : v < 0 ? 0 : v; }

}  // namespace

extern "C" {

double __fastcall HeightGrid_HeightAt(const u8 *sampler, void *, float x, float z) {
    const u8 *grid = at<const u8 *>(sampler, 0);
    const float ox = at<float>(grid, 0x08), oz = at<float>(grid, 0x0c), width = at<float>(grid, 0x10), depth = at<float>(grid, 0x14);
    const float perx = at<float>(sampler, 0x10), perz = at<float>(sampler, 0x14);
    const s32 i = Ftol((X87Reg(x) - ox) * perx);
    const s32 j = Ftol((X87Reg(z) - oz) * perz);
    const s32 nx = at<s32>(sampler, 0x08), nz = at<s32>(sampler, 0x0c);
    const u32 i1 = (u32)Clamp((s32)((u32)i + 1), nx), j1 = (u32)Clamp((s32)((u32)j + 1), nz);
    const X87Reg cellz = X87Reg(X87_MulIntDouble(j, depth)) + oz;
    const X87Reg cellx = X87Reg(X87_MulIntDouble(i, width)) + ox;
    const X87Reg dz = X87Reg(z) - cellz, dx = X87Reg(x) - cellx;
    const u32 heights = at<u32>(grid, 0x28), row = (u32)j * (u32)nx;
    const float h10 = *reinterpret_cast<const float *>((uintptr_t)(heights + (i1 + row) * 4));
    const float h01 = *reinterpret_cast<const float *>((uintptr_t)(heights + (j1 * (u32)nx + (u32)i) * 4));
    const float h00 = *reinterpret_cast<const float *>((uintptr_t)(heights + ((u32)i + row) * 4));
    const X87Reg up = X87Reg(h01) - h00, across = X87Reg(h10) - h00;
    return ((dz * perz) * up + (dx * perx) * across + h00).v;
}
}
