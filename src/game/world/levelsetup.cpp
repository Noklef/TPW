#include "levelsetup.h"

#include <cstdint>

#include "../math/x87.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 rectObject() { return *global<const u32 *>(0x7cf6ec); }

}  // namespace

void LevelSetup_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Level_SetupAfterTerrain() {
    g_calls.Cd(0x00450900);   // TerrainAttrMap_Load
    const u32 h = g_calls.Th(0x00519440, rectObject());
    const u32 c = g_calls.Th(0x00519430, rectObject());
    const u32 b = g_calls.Th(0x00519420, rectObject());
    const u32 a = g_calls.Th(0x00519410, rectObject());
    global<u32>(0x7a01bc) = a;
    global<u32>(0x7a029c) = b;
    global<u32>(0x7a0314) = c;
    global<u32>(0x7a0374) = h;
    const u8 *const model = global<const u8 *>(0x7a0854);
    if (model && at<const u8 *>(model, 0x6c)) {
        global<float>(0x79fe94) = (float)X87_MulInt(a, at<float>(at<const u8 *>(model, 0x6c), 0x10));
        global<float>(0x7a003c) = (float)X87_MulInt(b, at<float>(at<const u8 *>(model, 0x6c), 0x14));
    }
    if (global<u32>(0x7a1a8c) & 0x2000) g_calls.Cd(0x0056df70, a, b, global<u32>(0x7a006c) + c + a, global<u32>(0x7a01c0) + h + b);   // TerrainTex_SetRect
    g_calls.Cd(0x0053bfe0);   // DynFaces_InitModels
    g_calls.Cd(0x00450e60);   // Model_AllocVertexScratch
    g_calls.Cd(0x00469550);   // Highlight_FreeBuffers
    g_calls.Cd(0x004628a0);   // WorldVerts_Free
    g_calls.Cd(0x00462870, 0u);   // WorldVerts_Alloc
    g_calls.Cd(0x00469510);   // Highlight_AllocBuffers
    g_calls.Cd(0x00452e10);   // Model_InitUnusedTolerances
    g_calls.Cd(0x00435500);   // Coasters_InitTables
    g_calls.Cd(0x0052f050, 0x80u, 0x80u);   // Track_ResetMap
    g_calls.Cd(0x00475290);   // SpriteScripts_Init
    return 1;
}

}
