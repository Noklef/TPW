#include "baseload.h"

#include <cstdint>

#include "../coaster/helpers.h"
#include "../core/crt.h"
#include "../math/x87.h"
#include "land.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const char *text(u32 address) { return reinterpret_cast<const char *>((uintptr_t)address); }

// The two triangles of the square at h: their normals disagree. A NaN counts as disagreeing (FCOMP sets C0 when unordered).
bool Creased(const float *h, u32 row) {
    const float *const below = h + row;
    const X87Reg kOffset(-100.0);

    const X87Reg across = X87Reg(h[1]) - h[0];
    const float acrossStored = (float)across;
    const X87Reg down = X87Reg(below[0]) - h[0];
    const float downStored = (float)down;

    const float length1 = (float)X87_Sqrt(across * acrossStored - kOffset);
    const float second[3] = {(float)(X87Reg(10.0f) / length1), (float)(X87Reg(acrossStored) / length1), (float)(X87Reg(0.0f) / length1)};
    const float length2 = (float)X87_Sqrt(down * downStored - kOffset);
    const float first[3] = {(float)(X87Reg(0.0f) / length2), (float)(X87Reg(downStored) / length2), (float)(X87Reg(10.0f) / length2)};
    float normal1[3];
    Vec3_Cross(first, second, normal1);

    float v1[3] = {-10.0f, (float)(X87Reg(below[0]) - below[1]), 0.0f};
    float v2[3] = {0.0f, (float)(X87Reg(h[1]) - below[1]), -10.0f};
    Vec3_Normalize(v1);
    Vec3_Normalize(v2);
    float normal2[3];
    Vec3_Cross(v2, v1, normal2);

    return !(Vec3_Dot(normal1, normal2) >= 0.9999f);
}

}  // namespace

void TerrainBase_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Terrain_Load(u32 shared, u32 texture_arg) {
    const CrtEnv &crt = Crt();
    char folder[0x104], share[0x104], land[0x104];

    crt.sprintf(folder, "%s\\Terrain", shared ? global<const char *>(0xf81704) : text(g_calls.Cd(0x0040ef40)));   // GamePaths_Base
    g_calls.Cd(0x0046e650, share);   // GamePaths_ShareTexFolder
    g_calls.Cd(0x00461a60, "base.md2", folder, share);   // Quickload_LoadRide
    g_calls.Cd(0x00461f10, "Base", 8u, 0x0079fcb4u, 0u, 1u);   // RideModel_Load
    if (!global<u32>(0x79fcb4)) {
        g_calls.Cd(0x00461f10, "TestBase", 8u, 0x0079fcb4u, 0u, 1u);
        if (!global<u32>(0x79fcb4)) {
            global<u32>(0x7a0854) = 0;
            gTerrain_7a0884 = 0;
            return 0;
        }
    }

    const u8 *const model = at<const u8 *>(at<const u8 *>(global<const u8 *>(0x79fcb4), 8), 4);
    global<const u8 *>(0x7a0854) = model;
    gTerrain_7a0884 = at<u32>(at<const u8 *>(model, 0x6c), 0x2c);   // read before the test below: a null model faults here
    if (!model) return 1;

    g_calls.Cd(0x00427ad0, global<u32>(0x7a0854));   // TerrainSets_LoadLevel
    g_calls.Cd(0x0046f0c0, global<u32>(0x7a0854));   // Terrain_ModelCells

    const u8 *const grid = at<const u8 *>(global<const u8 *>(0x7a0854), 0x6c);
    for (u32 y = 0; y < at<u32>(grid, 0x1c); ++y) {
        for (u32 x = 0; x < at<u32>(grid, 0x18); ++x) {
            const u32 row = at<u32>(grid, 0x18) + 1;
            if (Creased(at<const float *>(grid, 0x28) + y * row + x, row))
                at<u8>(at<u8 *>(grid, 0x2c), (y * at<u32>(grid, 0x18) + x) * 4 + 1) |= 8;
        }
    }

    g_calls.Cd(0x00450950, global<u32>(0x7a0854), 0x0079fc54u);   // Terrain_CloneAttrs
    if ((global<u32>(0x7a1a8c) & 0x2000) && !shared) {
        const u32 directory = g_calls.Cd(0x0046e610);   // TexDirs_First
        if (directory) crt.sprintf(land, "%sbase.lnd", text(directory));
        else crt.sprintf(land, "base.lnd");
        g_calls.Cd(0x0056dfe0, land, texture_arg);   // TerrainTex_InitGlobal
        if (global<u32>(0x7a1a8c) & 0x4000) g_calls.Cd(0x0046ded0, 1u);   // Terrain_SetProceduralCells
        else g_calls.Cd(0x0056dfd0);   // TerrainTex_Disable
    }

    g_calls.Cd(0x0046dd50, at<u32>(global<const u8 *>(0x79fcb4), 8));   // Holder_CreateAt20
    g_calls.Cd(0x0045bbc0, at<u32>(global<const u8 *>(0x79fcb4), 8));   // Holder_Create
    const u8 *const holder = global<const u8 *>(0x79fcb4);
    if (at<u32>(holder, 4) & 0x4000) g_calls.Cd(0x0045bcb0, at<u32>(holder, 8), at<u32>(holder, 0xc0), at<u32>(holder, 0xc4));   // Holder_AddToGrid
    else g_calls.Cd(0x0045bd00, at<u32>(holder, 8));   // Holder_Wake
    return 1;
}

}
