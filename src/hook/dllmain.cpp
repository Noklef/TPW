// tpw hook DLL, loaded into tp.exe as a proxy dinput.dll (see dinput_proxy.cpp).
//
// On load it patches every function in the hook registry with a jump to our
// reimplementation. tpw.ini next to the exe can switch hooks off:
//   [tpw]   enabled=0          disable everything (pure passthrough)
//   [hooks] RSSE_FetchWord=0   disable one hook (names from re/symbols.csv)
// Results are written to tpw.log.
#include <cstdio>
#include <windows.h>

#include "hooks.h"

namespace {

char g_dir[MAX_PATH];  // folder of the game exe, with trailing backslash

void path_in_game_dir(char *out, const char *file) {
    lstrcpynA(out, g_dir, MAX_PATH);
    lstrcatA(out, file);
}

void install_hooks() {
    char ini[MAX_PATH], log_path[MAX_PATH];
    path_in_game_dir(ini, "tpw.ini");
    path_in_game_dir(log_path, "tpw.log");

    FILE *log = nullptr;
    fopen_s(&log, log_path, "w");
    if (log) std::fprintf(log, "tpw hook DLL: %u hooks registered\n", (unsigned)tpw::hook::kHookCount);

    if (!GetPrivateProfileIntA("tpw", "enabled", 1, ini)) {
        if (log) std::fprintf(log, "disabled by tpw.ini\n");
    } else {
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
            if (log) std::fprintf(log, "  %08X %-32s %s\n", h.address, h.name, status);
        }
        if (log) std::fprintf(log, "%d/%u hooks active\n", ok, (unsigned)tpw::hook::kHookCount);
    }
    if (log) std::fclose(log);
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        GetModuleFileNameA(nullptr, g_dir, MAX_PATH);
        if (char *slash = strrchr(g_dir, '\\')) slash[1] = '\0';

        // Only patch the game itself, and only the 0x400000-based image we mapped.
        if (reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr)) == 0x400000)
            install_hooks();
    }
    return TRUE;
}
