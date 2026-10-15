// tpw hook DLL, loaded into tp.exe as a proxy dinput.dll (see dinput_proxy.cpp).
//
// On load it patches every function in the hook registry with a jump to our
// reimplementation. tpw.ini next to the exe can switch hooks off:
//   [tpw]   enabled=0          disable everything (pure passthrough)
//   [tpw]   trace=0            no startup trace (see trace.h; on by default)
//   [hooks] RSSE_FetchWord=0   disable one hook (names from re/symbols.csv)
// Results are written to tpw.log, followed by the trace and the FPU precision as the game runs.
#include <windows.h>

#include "hooks.h"
#include "log.h"
#include "trace.h"

namespace {

using tpw::hook::log_line;
using tpw::hook::log_seconds;

char g_dir[MAX_PATH];  // folder of the game exe, with trailing backslash

void path_in_game_dir(char *out, const char *file) {
    lstrcpynA(out, g_dir, MAX_PATH);
    lstrcatA(out, file);
}

void install_hooks(const char *ini) {
    if (!GetPrivateProfileIntA("tpw", "enabled", 1, ini)) {
        log_line("hooks disabled by tpw.ini");
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
        log_line("  %08X %-32s %s", h.address, h.name, status);
    }
    log_line("%d/%u hooks active", ok, (unsigned)tpw::hook::kHookCount);
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
            log_line("[%6.1fs] FPU control word %04X: precision %s", log_seconds(), (unsigned)control,
                     precision_name(control));
        }
        Sleep(1000);
    }
    CloseHandle(thread);
    return 0;
}

}  // namespace

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    // Only the game itself, and only the 0x400000-based image the hooks were made for.
    if (reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr)) != 0x400000) return TRUE;
    if (reason == DLL_PROCESS_ATTACH) {
        GetModuleFileNameA(nullptr, g_dir, MAX_PATH);
        if (char *slash = strrchr(g_dir, '\\')) slash[1] = '\0';
        char ini[MAX_PATH], log_path[MAX_PATH];
        path_in_game_dir(ini, "tpw.ini");
        path_in_game_dir(log_path, "tpw.log");

        tpw::hook::log_open(log_path);
        // The link time from our own PE header: unlike __DATE__, it changes with any rebuilt file.
        const auto *self = reinterpret_cast<const BYTE *>(instance);
        const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(
            self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
        log_line("tpw hook DLL (linked %08X): %u hooks registered", (unsigned)nt->FileHeader.TimeDateStamp,
                 (unsigned)tpw::hook::kHookCount);
        install_hooks(ini);
        if (GetPrivateProfileIntA("tpw", "trace", 1, ini)) tpw::hook::install_trace();
        // tp.exe imports dinput.dll, so this runs on the game's main thread at startup.
        g_main_thread = GetCurrentThreadId();
        if (HANDLE watcher = CreateThread(nullptr, 0, watch_fpu, nullptr, 0, nullptr)) CloseHandle(watcher);
    } else if (reason == DLL_PROCESS_DETACH) {
        log_line("[%6.1fs] process exiting", log_seconds());
    }
    return TRUE;
}
