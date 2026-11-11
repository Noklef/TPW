// The C++ runtime's terminate: see crtterminate.h.
#include "crtterminate.h"

#include <cstdint>
#include "addrcalls.h"

namespace {
AddrCalls g_calls;
}

void CrtTerminate_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" void __cdecl CRT_terminate() {
    __try {
        if (reinterpret_cast<const u32 *>((uintptr_t)g_calls.Cd(0x00682460))[0x60 / 4])   // _getptd
            reinterpret_cast<void(__cdecl *)()>((uintptr_t) reinterpret_cast<const u32 *>((uintptr_t)g_calls.Cd(0x00682460))[0x60 / 4])();
    } __except (1) {
    }
    g_calls.Cd(0x0068cc30);   // abort
}
