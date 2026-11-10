#include "levelfree.h"

#include <cstdint>

#include "../core/addrcalls.h"
#include "rides.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

AddrCalls g_calls;

}  // namespace

void LevelFree_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

void __cdecl Level_FreeAll(u32 keep) {
    const bool kept = (keep & 0xff) != 0;
    g_calls.Cd(0x0053c810);   // DynFaces_ReleaseModels
    g_calls.Cd(0x004628a0);   // WorldVerts_Free
    g_calls.Cd(0x00598ad0);   // Advisor_ClearSpeechText
    g_calls.Cd(0x00429e20, 1u);   // ModelSlot_Free
    g_calls.Cd(0x00429e20, 0u);
    g_calls.Cd(0x00454540);
    g_calls.Cd(0x00437150);   // CoasterLinks_DestroyAll
    g_calls.Cd(0x00430ad0);   // CoasterTypes_FreeAll
    g_calls.Cd(0x00458580);   // LightSource_DestroyMain
    if (!kept) g_calls.Cd(0x00450e80);   // Model_FreeVertexScratch
    g_calls.Cd(0x0052f190);   // AllPieces_Destroy
    if (!kept) g_calls.Cd(0x00541ed0);   // Sprites_FreeAll
    for (u32 i = 0; i < global<u32>(0x007a3ebc) + global<u32>(0x007a4084); i++)
        if ((u32)(uintptr_t)gRides_7a4610[i + 1]) g_calls.Cd(0x00463c80, i + 1);   // Ride_Destroy
    if (!kept) {
        g_calls.Cd(0x00450a80);   // Terrain_FreeLoaded
        g_calls.Cd(0x00463ed0);   // InstanceTypes_FreeAll
        g_calls.Cd(0x00469550);   // Highlight_FreeBuffers
    }
    g_calls.Cd(0x0046ea80);   // TexDirs_Reset
}
