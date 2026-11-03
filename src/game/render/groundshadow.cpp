#include "groundshadow.h"

#include <cstdint>

#include "batchquad.h"
#include "heightgrid.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const GroundShadowEnv kPortEnv = {&Batch_DrawQuad};
const GroundShadowEnv *g_env = &kPortEnv;

float Height(const u8 *grid, float x, float z) { return (float)(X87Reg(HeightGrid_HeightAt(grid, nullptr, x, z)) + global<float>(0x008bd4ec)); }

}  // namespace

void GroundShadow_SetEnv(const GroundShadowEnv *env) { g_env = env ? env : &kPortEnv; }

u32 __fastcall GroundShadow_Draw(u8 *object, void *) {
    typedef void(__cdecl * TransformFn)(void *, const void *, void *, const void *, u32, u32, u32, u32);
    const u8 *const rect = at<const u8 *>(object, 0x13c);
    if (!rect)
        return 1;

    // The spec, its points 0x2c on as in the original's frame.
    // Sic, not kept: the original never sets the polygon flags at +0, so they start as what its frame held, and the batch reads them. 0 here.
    u8 spec[0x6c] = {};
    u8 *const points = spec + 0x2c;
    const u8 *const texture = at<const u8 *>(rect, 0x18);
    if (texture) {
        at<u32>(spec, 0x10) = at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0;
        at<float>(spec, 0x18) = (float)X87Reg(at<float>(texture, 0x28));
        at<float>(spec, 0x1c) = (float)X87Reg(at<float>(texture, 0x2c));
        at<float>(spec, 0x20) = (float)(X87Reg(at<float>(texture, 0x20)) + at<float>(spec, 0x18));
        at<float>(spec, 0x24) = (float)(X87Reg(at<float>(texture, 0x24)) + at<float>(spec, 0x1c));
        if (at<u32>(texture, 8) & 0x100000)
            at<u32>(spec, 0) |= 4;
    }
    at<u8 *>(spec, 4) = points;
    at<u32>(spec, 8) = 0x80404040;
    at<u32>(spec, 0xc) = 0xd56;

    const float x0 = at<float>(rect, 0), x1 = at<float>(rect, 0xc), z0 = at<float>(rect, 0x14), z1 = at<float>(rect, 8);
    const u8 *const grid = global<const u8 *>(0x00879d60);
    float corners[12];
    at<u32>(corners, 0) = at<u32>(rect, 0);
    at<u32>(corners, 0xc) = at<u32>(rect, 0);
    at<u32>(corners, 8) = at<u32>(rect, 0x14);
    at<u32>(corners, 0x2c) = at<u32>(rect, 0x14);
    at<u32>(corners, 0x14) = at<u32>(rect, 8);
    at<u32>(corners, 0x20) = at<u32>(rect, 8);
    corners[6] = (float)X87Reg(x1);
    corners[9] = corners[6];
    corners[1] = Height(grid, x0, z0);
    corners[4] = Height(grid, x0, z1);
    corners[7] = Height(grid, corners[6], z1);
    corners[10] = Height(grid, corners[6], z0);

    u8 *const transformer = global<u8 *>(0x0087b1d8);
    at<TransformFn>(at<u8 *>(transformer, 0), 4)(transformer, global<void *>(0x0087b088), points, corners, 0x10, 0xc, 4, 0);
    return g_env->drawQuad(global<u8 *>(0x00879ab0), nullptr, spec, 0xffffffff);
}
