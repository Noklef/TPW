#include "startcheck.h"

#include <cstdint>

#include "gameerror.h"
#include "regkey.h"

namespace {

template <typename T> T &global(std::uintptr_t address) { return *reinterpret_cast<T *>(address); }

template <typename F> F import(std::uintptr_t slot) { return *reinterpret_cast<const F *>(slot); }

}  // namespace

extern "C" {

u8 __cdecl Startup_CheckCanRun() {
    u8 *const report = reinterpret_cast<u8 *>(static_cast<std::uintptr_t>(0x00785de8));
    if (!Registry_GameInstalled()) {
        GameError_Set(report, nullptr, 0, 0, nullptr);
        return 0;
    }
    const u32 mutex = import<u32(__stdcall *)(void *, u32, const char *)>(0x006fd134)(nullptr, 1, global<const char *>(0x007a2508));
    global<u32>(0x007a267c) = mutex;
    if (mutex && import<u32(__stdcall *)()>(0x006fd138)() == 0xb7) {   // (ERROR_ALREADY_EXISTS)
        GameError_Set(report, nullptr, 1, 0, nullptr);
        import<s32(__stdcall *)(u32)>(0x006fd13c)(global<u32>(0x007a267c));
        return 0;
    }
    return 1;
}
}
