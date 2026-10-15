#include "fpu_watch.h"

#include <windows.h>

#include "log.h"

namespace tpw::testing {

namespace {

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

void start_fpu_watch() {
    g_main_thread = GetCurrentThreadId();
    if (HANDLE watcher = CreateThread(nullptr, 0, watch_fpu, nullptr, 0, nullptr)) CloseHandle(watcher);
}

}  // namespace tpw::testing
