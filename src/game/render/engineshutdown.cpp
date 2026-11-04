#include "engineshutdown.h"

#include <cstdint>

namespace {

AddrCalls g_calls;

}  // namespace

void EngineShutdown_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

u32 __cdecl Engine_ShutdownAll() {
    g_calls.Cd(0x00564b60);   // Wavelet_Shutdown
    g_calls.Cd(0x0057c4a0);   // Global8bba48_DeleteVBase
    g_calls.Cd(0x0057d1a0);   // WeatherFx_Delete
    g_calls.Cd(0x0058a490);   // SpriteQueue_Free
    g_calls.Cd(0x00561a00);   // FreeGlobal879a20
    g_calls.Cd(0x0058b040);   // FreeGlobal8c47d4
    g_calls.Cd(0x00574b10);   // TexManager_Destroy
    g_calls.Cd(0x005761b0);   // Renderer_Destroy
    g_calls.Cd(0x0056de90);   // TerrainTex_Destroy
    g_calls.Cd(0x0058d090);   // TexCache_Destroy
    g_calls.Cd(0x00580500);   // FreeGlobal8bcbb8
    g_calls.Cd(0x005740e0);   // FreeGlobal87a248
    g_calls.Cd(0x00582280);   // DrawBuffer_Destroy
    const u32 mode = *reinterpret_cast<const u32 *>((uintptr_t)0x008bd524);
    if (mode == 1) {
        g_calls.Cd(0x005667a0);   // Global879ac0_Delete
        g_calls.Cd(0x00569ef0);   // Global879acc_Delete
        g_calls.Cd(0x00573b30);   // SoftSetup_Destroy
    } else if (mode == 4) {
        g_calls.Cd(0x00569ef0);
    }
    g_calls.Cd(0x00578390);   // Global87b1d8_Delete
    g_calls.Cd(0x0056b760);   // FreeGlobal879d54
    return 1;
}

}
