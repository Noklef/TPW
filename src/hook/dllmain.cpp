// tpw hook DLL, loaded into tp.exe as a proxy dinput.dll (see dinput_proxy.cpp).
//
// On load it patches every function in the hook registry with a jump to our
// reimplementation. tpw.ini next to the exe can switch hooks off:
//   [tpw]   enabled=0          disable everything (pure passthrough)
//   [hooks] RSSE_FetchWord=0   disable one hook (names from re/symbols.csv)
// Results are written to tpw.log, followed by the FPU precision as the game runs.
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
    if (log) std::fprintf(log, "tpw hook DLL (built " __DATE__ " " __TIME__ "): %u hooks registered\n",
                          (unsigned)tpw::hook::kHookCount);

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

// The float ports assume the x87 runs at 24-bit precision, which Direct3D sets when the game
// creates its device. A wrapper (dgVoodoo) or software rendering may leave it at 53 or 64 bits,
// so this samples the main thread's FPU control word every second and logs each change.
DWORD g_main_thread;

const char *precision_name(DWORD control) {
    switch ((control >> 8) & 3) {
    case 0:  return "24-bit (single)";
    case 2:  return "53-bit (double)";
    case 3:  return "64-bit (extended)";
    default: return "reserved";
    }
}

DWORD WINAPI watch_fpu(LPVOID) {
    HANDLE thread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, g_main_thread);
    if (!thread) return 0;
    char log_path[MAX_PATH];
    path_in_game_dir(log_path, "tpw.log");
    const DWORD start = GetTickCount();
    DWORD last = 0xffffffff;
    for (;;) {
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FLOATING_POINT;
        if (SuspendThread(thread) == (DWORD)-1) break;  // the main thread has exited
        const BOOL ok = GetThreadContext(thread, &context);
        ResumeThread(thread);
        if (!ok) break;
        const DWORD control = context.FloatSave.ControlWord & 0xffff;
        if (control != last) {
            last = control;
            FILE *log = nullptr;
            fopen_s(&log, log_path, "a");
            if (log) {
                std::fprintf(log, "[%6.1fs] FPU control word %04X: precision %s\n",
                             (GetTickCount() - start) / 1000.0, (unsigned)control, precision_name(control));
                std::fclose(log);
            }
        }
        Sleep(1000);
    }
    CloseHandle(thread);
    return 0;
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        GetModuleFileNameA(nullptr, g_dir, MAX_PATH);
        if (char *slash = strrchr(g_dir, '\\')) slash[1] = '\0';

        // Only patch the game itself, and only the 0x400000-based image we mapped.
        if (reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr)) == 0x400000) {
            install_hooks();
            // tp.exe imports dinput.dll, so this runs on the game's main thread at startup.
            g_main_thread = GetCurrentThreadId();
            if (HANDLE watcher = CreateThread(nullptr, 0, watch_fpu, nullptr, 0, nullptr)) CloseHandle(watcher);
        }
    }
    return TRUE;
}
