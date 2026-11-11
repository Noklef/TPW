#include "gamewindow.h"

#include <cstdint>
#include "../render/screen.h"
#include "../render/graphicssettings.h"
#include "../gfx/texcache.h"
#include "staticobjects.h"
#include "../render/d3dsetup.h"
#include "imports.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kWindowName = 0x0074c350, kOptions = (u32)(uintptr_t)&gOptions_78d8d8;   // (the class's name and the window's, "Theme Park W..."; the options object)

u32 Fail(u32 code) {
    g_calls.Th(0x00405710, 0x00785de8u, code, 0u, 0u);   // GameError_Set
    return 0;
}

u32 MakeWindow() {
    using CreateWindowExA = u32(__stdcall *)(u32, u32, u32, u32, u32, u32, s32, s32, u32, u32, u32, u32);
    const auto create = [](u32 style, s32 width, s32 height) {
        return global<CreateWindowExA>(IMPORT(User32_CreateWindowExA))(0, kWindowName, kWindowName, style, 0x80000000u, 0x80000000u, width, height, 0, 0, global<u32>(0x007a1a44), 0);
    };
    if (reinterpret_cast<u8 &>(gGraphicssettings_7a1a8c) & 1)
        return create(0x90000000u, reinterpret_cast<s32 &>(gScreen_7a1968), reinterpret_cast<s32 &>(gScreen_7a1930));
    s32 rect[4] = {0, 0, reinterpret_cast<s32 &>(gScreen_7a1968), reinterpret_cast<s32 &>(gScreen_7a1930)};
    const u32 overlapped = 0xc00000 | 0x80000 | 0x40000 | 0x20000 | 0x10000;   // WS_OVERLAPPEDWINDOW in its parts (one literal lies in a GAME_DATA range)
    global<u32(__stdcall *)(s32 *, u32, u32)>(IMPORT(User32_AdjustWindowRect))(rect, overlapped, 0);   // AdjustWindowRect
    return create(overlapped, rect[2] - rect[0], rect[3] - rect[1]);
}

}  // namespace

void GameWindow_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" u32 __cdecl GameWindow_Create() {
    const u32 instance = global<u32>(0x007a1a44);
    const u32 cursor = global<u32(__stdcall *)(u32, u32)>(IMPORT(User32_LoadIconA))(0, 0x7f00);   // LoadCursorA
    const u32 windowClass[10] = {0, 0x0046bcb0, 0, 0, instance, cursor, 0, 0, 0, kWindowName};   // (the cursor in hIcon, none in hCursor: sic)
    if ((u16)global<u32(__stdcall *)(const u32 *)>(IMPORT(User32_RegisterClassA))(windowClass) == 0)   // RegisterClassA
        return Fail(4);
    const u32 window = MakeWindow();
    gGamewindow_7a130c = window;
    if (window == 0) return Fail(4);
    global<u32(__stdcall *)(u32, u32)>(IMPORT(User32_ShowWindow))(window, 5);   // ShowWindow
    global<u32(__stdcall *)(u32)>(IMPORT(User32_UpdateWindow))(gGamewindow_7a130c);   // UpdateWindow

    if (global<u32>((u32)(uintptr_t)&gOptions_78d8d8 + 0x90) == 1) {
        const u32 settings = global<u32>(0x0079d86c);
        if (!(settings & 0x4000) && g_calls.Cd(0x0044dc60, settings) == 0)   // GraphicsSettings_Load
            return 0;
        g_calls.Cd(0x0044dd50);   // Options_Apply
    } else {
        g_calls.Cd(0x0044de20);   // GraphicsSettings_LoadForOptions
    }
    const u32 flags = gGraphicssettings_7a1a8c;
    if (g_calls.Cd(0x0044df20, flags & 0x200000 ? 0x20u : 0x10u, flags >> 12 & 1, flags & 1, ((flags & 0x400000) | 0x800000) >> 22) == 0)   // Engine_Start
        return Fail(0xc);

    u32 *const matrix = global<u32 *>(0x0087afe8);
    for (u32 i = 0; i < 0x10; ++i) matrix[i] = i % 5 == 0 ? 0x3f800000u : 0;
    g_calls.Th(0x00578be0, 0x0087b110u, 0x3dcccccdu, 0x447a0000u, 0x42b40000u);   // Mat4_Perspective (0.1, 1000, 90)

    const u32 detail = global<u32>((u32)(uintptr_t)&gBalanceGraphics + 0xc);
    if (detail <= 3) {
        u8 *const engine = reinterpret_cast<u8 * &>(gTexcache_8c4b44);
        *reinterpret_cast<u32 *>(engine + 0x1920) = 3 - detail;
        *reinterpret_cast<u32 *>(engine + 0x18f4) = 3 - detail;
    }
    const bool on = global<u32>(0x008bd524) == 1;
    g_calls.Th(0x00423b50, kOptions, on ? 1u : 0u);   // Obj_SetFirst
    u32 &settings = global<u32>(0x0079d86c);
    settings = (settings & ~(on ? 0x40u : 0x80u)) | (on ? 0x80u : 0x40u);
    if (!on) gGraphicssettings_7a1a8c &= ~0x6000u;
    if (global<u32>((u32)(uintptr_t)&gOptions_78d8d8 + 0xc) == 1 && *reinterpret_cast<s32 *>(global<u8 *>((u32)(uintptr_t)&gD3dsetup_879a20 + 0x4) + 0x788) <= 1)
        g_calls.Th(0x00423b80, kOptions, 0u);   // Options_SetC
    return 1;
}
