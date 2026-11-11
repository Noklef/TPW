#include "levelsetup.h"

#include <cstdint>

#include "../core/crt.h"
#include "../math/x87.h"
#include "../render/graphicssettings.h"
#include "levelsetup.h"
#include "../core/staticobjects.h"

namespace {

AddrCalls g_calls;

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u32 rectObject() { return *global<const u32 *>(0x7cf6ec); }

const char *GamePath() { return reinterpret_cast<const char *>((uintptr_t)g_calls.Cd(0x0040ef40)); }   // GamePaths_Base

}  // namespace

void LevelSetup_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Level_LoadTerrainAndAdvisor() {
    char first[0x104], second[0x104], language[0x104];   // the original's frame, 0x30c
    const CrtEnv &crt = Crt();
    const char *const advisorName = reinterpret_cast<const char *>((uintptr_t)0x007af650);
    gLevelsetup_7b0958 = crt.rand();
    g_calls.Cd(0x004740d0, 0u);   // Motion_SetFlags
    g_calls.Cd(0x0042a990);   // Camera_Reset
    GamePath();   // the original looks for its last backslash and does not use the result
    const u32 flags = global<u32>(0x79d86c);
    const u32 texture = flags & 4 ? 0x10u : flags & 2 ? 0x20u : flags & 1 ? 0x40u : 0x20u;
    g_calls.Cd(0x00462870, 1u);   // WorldVerts_Alloc
    g_calls.Cd(0x004504c0, 0u, texture);   // Terrain_Load
    crt.sprintf(first, "%s\\Global\\Advisor", global<const char *>((u32)(uintptr_t)&gStrings5_f816c0 + 0x30));   // MemorySource_GetLock inlined: 0xf816c0 + 0x28, its +8
    crt.sprintf(second, "%s\\Advisor\\%s", GamePath(), advisorName);
    g_calls.Cd(0x00429ba0, first, second, 0u, 0u);   // ModelSlot_LoadAdvisor
    language[0] = '_';
    language[1] = 0;
    CRT_strncat(language, global<const char *>((u32)(uintptr_t)&gNamedObjects_786b68 + 0x8), 3);   // Obj_Self inlined: 0x786b68, its +8
    crt.sprintf(first, "%s\\Advisor", GamePath());
    crt.sprintf(second, "%s\\Global\\Advisor\\%s", global<const char *>((u32)(uintptr_t)&gStrings5_f816c0 + 0x30), advisorName);
    g_calls.Cd(0x00429ba0, first, second, language, 1u);
    g_calls.Cd(0x005989c0);   // Advisor_Init
    g_calls.Cd(0x004584d0);   // LightSource_CreateMain
    g_calls.Cd(0x00469350);   // Highlight_ResetTints
    return 1;
}

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
    if (gGraphicssettings_7a1a8c & 0x2000) g_calls.Cd(0x0056df70, a, b, global<u32>(0x7a006c) + c + a, global<u32>(0x7a01c0) + h + b);   // TerrainTex_SetRect
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
