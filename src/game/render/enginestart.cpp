#include "enginestart.h"

#include <cstdint>

#include "engineerrors.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "screen.h"
#include "../core/gamewindow.h"

namespace {

AddrCalls g_calls;

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 &kind() { return global<u32>(0x008bd524); }

u32 address(const void *p) { return (u32)(uintptr_t)p; }

bool logging() { return (global<u32>(0x008bd508) & 0x2000000) != 0; }

void logStage(const char *text) {
    if (!logging())
        return;
    char line[0x64];
    Crt().sprintf(line, text);
    g_calls.Cd(0x005645e0, address(line));
}

void pushError(u32 code) { EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, code); }

bool createParts(u32 a, u32 c) {
    if (!g_calls.Cd(0x0056b6c0) || !g_calls.Cd(0x005781d0))
        return false;
    const u32 driver = kind();
    if (driver == 4) {
        if (!g_calls.Cd(0x00569e10))
            return false;
        global<u32>(0x00879ab0) = global<u32>(0x00879acc);
    } else if (driver == 1) {
        if (!g_calls.Cd(0x005666f0))
            return false;
        global<u32>(0x00879ab0) = global<u32>(0x00879ac0);
        if (global<u32>(0x008bcbc8) & 0x200000) {
            g_calls.Cd(0x00573a30);
            if (!g_calls.Cd(0x00569e10))
                return false;
        }
    }
    logStage(" (gamut,matrix,drawprimitive ok)");
    if (!g_calls.Cd(0x005821e0) || !g_calls.Cd(0x00573fd0) || !g_calls.Cd(0x005804b0))
        return false;
    logStage(" (rmem/lights/render)");
    if (!g_calls.Cd(0x0056ddf0))
        return false;
    if (c)
        g_calls.Th(0x0056e550, global<u32>(0x00879d60), c, 0u);
    if (!g_calls.Cd(0x005760a0) || !g_calls.Cd(0x0058aff0))
        return false;
    logStage(" (hfield,mapwho,switch)");
    if (!g_calls.Cd(0x00561840) || !g_calls.Cd(0x0058a430) || !g_calls.Cd(0x0057c360))
        return false;
    logStage(" (clipping/sprite/particle ok)");
    if (!g_calls.Cd(0x0058cff0, a))
        return false;
    logStage(" (tcache ok)");
    if (!g_calls.Cd(0x00574a20) || !g_calls.Cd(0x0057d100) || !g_calls.Th(0x00585720, 0x008c1828u))
        return false;
    logStage(" (tload/rain/sky ok)");
    return g_calls.Cd(0x00564b20) != 0;
}

void destroyParts() {
    if (logging()) {
        char line[0x64];
        Crt().sprintf(line, " --- FAILED, exiting (error code %x)", global<u32>(0x00879cd4));
        g_calls.Cd(0x005645e0, address(line));
    }
    g_calls.Cd(0x00564b60);
    g_calls.Cd(0x0057c4a0);
    g_calls.Cd(0x0057d1a0);
    g_calls.Cd(0x0058a490);
    g_calls.Cd(0x00561a00);
    g_calls.Cd(0x0058b040);
    g_calls.Cd(0x00574b10);
    g_calls.Cd(0x005761b0);
    g_calls.Cd(0x0056de90);
    g_calls.Cd(0x0058d090);
    g_calls.Cd(0x00580500);
    g_calls.Cd(0x005740e0);
    g_calls.Cd(0x00582280);
    const u32 driver = kind();
    if (driver == 4) {
        g_calls.Cd(0x00569ef0);
    } else if (driver == 1) {
        g_calls.Cd(0x005667a0);
        g_calls.Cd(0x00569ef0);
        g_calls.Cd(0x00573b30);
    }
    g_calls.Cd(0x00578390);
    g_calls.Cd(0x0056b760);
}

u32 openDirectDraw(u32 a, u32 b, u32 c) { return g_calls.Cd(0x005632b0, gGamewindow_7a130c, gScreen_7a1968, gScreen_7a1930, a, b, c); }

u32 init(u32 d) { return g_calls.Cd(0x00572780, d, 0x092f2266u, 0u); }

}  // namespace

void EngineStart_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

u32 __cdecl Engine_OpenDirectDraw(u32 x0, u32 x1, u32 x2, u32 x3, u32 x4, u32 x5) {
    if (global<u32>(0x00879a24))
        return 0;
    u8 *const object = static_cast<u8 *>(GameHeap_Alloc(0x790));
    if (object) {
        at<u32>(object, 0x780) = 0;
        for (u32 i = 0; i < 7; ++i)
            at<u32>(object, 4 * i) = 0;
        at<u32>(object, 0x788) = 0;
        at<u32>(object, 0x78c) = 0;
        at<u32>(object, 0x784) = 1;
    }
    global<u8 *>(0x00879a24) = object;
    if (!object) {
        pushError(0xb0754000);
        return 0;
    }
    global<u32>(0x008bd518) = x0;
    if (g_calls.Th(0x00563460, address(object), x1, x2, x3, x4, x5, (u32)(kind() == 1)))
        return 1;
    u8 *const held = global<u8 *>(0x00879a24);
    if (held) {
        g_calls.Th(0x005640a0, address(held));
        GameHeap_Free(held);
    }
    global<u32>(0x00879a24) = 0;
    pushError(0xb0754004);
    return 0;
}

u32 __cdecl Engine_CreateDevice(u32 x) {
    if (kind() == 1)
        return g_calls.Cd(0x00572c40, x);
    if (kind() == 4)
        return g_calls.Cd(0x00573960, x);
    return 0;
}

u32 __cdecl Engine_DestroyDevice() {
    if (kind() == 1)
        return g_calls.Cd(0x00572d50);
    if (kind() == 4)
        g_calls.Cd(0x00573b30);
    return 0;
}

u32 __cdecl Engine_Init(u32 a, u32, u32 c) {
    if (logging())
        g_calls.Cd(0x005645e0, address("Initializing engine internals"));
    global<u32>(0x008bcbc8) &= ~2u;
    g_calls.Cd(0x005913f0);
    g_calls.Th(0x005839f0, 0x008bd550u);
    if (kind() == 4)
        g_calls.Th(0x005839a0, 0x008bd550u);
    if (!createParts(a, c)) {
        destroyParts();
        return 0;
    }
    g_calls.Th(0x0057d920, global<u32>(0x008bcbac), 0u, 0x3e99999au);   // (0.3)
    logStage(" (wavelet/printf ok)");
    g_calls.Th(0x00583cc0, 0x008bd550u);
    logStage(" (uv correction ok)");
    if (global<u32>(0x008bd508) & 0x1000000) {
        g_calls.Th(0x005835c0, 0x008bd508u);
        g_calls.Th(0x005833c0, 0x008bcbc8u);
        g_calls.Th(0x00584790, 0x008bd550u);
    }
    if (logging())
        g_calls.Cd(0x005645e0, address("Engine internals initialized succesfully"));   // (sic)
    return 1;
}

u32 __cdecl Engine_Start(u32 a, u32 b, u32 c, u32 d) {
    if (kind() != 4 && openDirectDraw(a, b, c)) {
        if (!g_calls.Cd(0x005836d0, 1u)) {
            if (kind() != 1) {
                g_calls.Cd(0x005633a0);
                return 0;
            }
            kind() = 4;
            if (!g_calls.Cd(0x005836d0, 1u)) {
                g_calls.Cd(0x005633a0);
                return 0;
            }
        }
        if (init(d))
            return 1;
        if (kind() != 1) {
            g_calls.Cd(0x00583700);
            g_calls.Cd(0x005633a0);
            return 0;
        }
        g_calls.Cd(0x00583700);
        kind() = 4;
        if (!g_calls.Cd(0x005836d0, 1u)) {
            g_calls.Cd(0x005633a0);
            return 0;
        }
    } else {
        kind() = 4;
        if (!openDirectDraw(a, b, c))
            return 0;
        if (!g_calls.Cd(0x005836d0, 1u)) {
            g_calls.Cd(0x005633a0);
            return 0;
        }
    }
    if (init(d))
        return 1;
    g_calls.Cd(0x00583700);
    g_calls.Cd(0x005633a0);
    return 0;
}
