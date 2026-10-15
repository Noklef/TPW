// tpw hook DLL, loaded into tp.exe as a proxy dinput.dll (see dinput_proxy.cpp).
//
// On load it patches every function in the hook registry with a jump to our
// reimplementation. tpw.ini next to the exe can switch hooks off:
//   [tpw]   enabled=0          disable everything (pure passthrough)
//   [hooks] RSSE_FetchWord=0   disable one hook (names from re/symbols.csv)
// Built with TPW_TEST_TOOLING=ON, the DLL also writes tpw.log through src/testing (test tooling,
// not part of the decompilation; [tpw] trace=0 turns its startup trace off).
#include <windows.h>

#include "hooks.h"

#ifdef TPW_TEST_TOOLING
#include "../testing/log.h"
#include "../testing/testing.h"
#define TEST_LOG(...) tpw::testing::log_line(__VA_ARGS__)
#else
#define TEST_LOG(...) ((void)0)
#endif

namespace {

char g_dir[MAX_PATH];  // folder of the game exe, with trailing backslash

void install_hooks(const char *ini) {
    if (!GetPrivateProfileIntA("tpw", "enabled", 1, ini)) {
        TEST_LOG("hooks disabled by tpw.ini");
        return;
    }
    int ok = 0;
    for (std::size_t i = 0; i < tpw::hook::kHookCount; ++i) {
        const tpw::hook::Hook &h = tpw::hook::kHooks[i];
        const char *status;
        if (!GetPrivateProfileIntA("hooks", h.name, 1, ini)) {
            status = "off (tpw.ini)";
        } else {
            switch (tpw::hook::patch_jump(reinterpret_cast<void *>(h.address), h.replacement, h.expect)) {
            case tpw::hook::PatchResult::Ok:            status = "hooked"; ++ok; break;
            case tpw::hook::PatchResult::BytesMismatch: status = "SKIPPED: original bytes differ (unknown exe build?)"; break;
            default:                                    status = "FAILED: VirtualProtect"; break;
            }
        }
        TEST_LOG("  %08X %-32s %s", h.address, h.name, status);
        (void)status;
    }
    TEST_LOG("%d/%u hooks active", ok, (unsigned)tpw::hook::kHookCount);
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    // Only the game itself, and only the 0x400000-based image the hooks were made for.
    if (reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr)) != 0x400000) return TRUE;
    (void)instance;
    if (reason == DLL_PROCESS_ATTACH) {
        GetModuleFileNameA(nullptr, g_dir, MAX_PATH);
        if (char *slash = strrchr(g_dir, '\\')) slash[1] = '\0';
        char ini[MAX_PATH];
        lstrcpynA(ini, g_dir, MAX_PATH);
        lstrcatA(ini, "tpw.ini");
#ifdef TPW_TEST_TOOLING
        tpw::testing::start(instance, g_dir, (unsigned)tpw::hook::kHookCount);
#endif
        install_hooks(ini);
#ifdef TPW_TEST_TOOLING
        tpw::testing::after_hooks(ini);
#endif
    } else if (reason == DLL_PROCESS_DETACH) {
#ifdef TPW_TEST_TOOLING
        tpw::testing::stop();
#endif
    }
    return TRUE;
}
