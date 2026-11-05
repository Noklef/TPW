#include "load_timer.h"

#include <windows.h>

#include <cstring>

#include "log.h"
#include "profiler.h"

namespace tpw::testing {

namespace {

const BYTE *const kEye = reinterpret_cast<const BYTE *>(0x007909c0);   // the camera's eye: x, y, z (floats)

volatile LONG g_opened_ms = -1;
volatile LONG g_eye_logged;
BYTE g_eye[12];

double since_open() {
    return log_seconds() - g_opened_ms / 1000.0;
}

DWORD WINAPI timer_thread(void *) {
    bool down = false;
    for (;;) {
        Sleep(10);
        const bool now = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
        const bool pressed = now && !down;
        down = now;
        if (g_opened_ms < 0) continue;
        if (!g_eye_logged && std::memcmp(g_eye, kEye, sizeof g_eye) != 0) {
            g_eye_logged = 1;
            log_line("[%6.1fs] level load: the camera's eye first moved %.2f s after the open", log_seconds(), since_open());
        }
        if (pressed) {
            log_line("[%6.1fs] level load: F9 %.2f s after the open", log_seconds(), since_open());
            log_profile();
            reset_profile();
            g_opened_ms = -1;
        }
    }
}

}  // namespace

void start_load_timer() {
    if (const HANDLE thread = CreateThread(nullptr, 0, timer_thread, nullptr, 0, nullptr)) CloseHandle(thread);
}

void load_timer_file_opened(const char *name) {
    const char *const dot = name ? strrchr(name, '.') : nullptr;
    if (!dot || (lstrcmpiA(dot, ".TPWS") != 0 && lstrcmpiA(dot, ".TPWI") != 0 && lstrcmpiA(dot, ".INTS") != 0)) return;
    std::memcpy(g_eye, kEye, sizeof g_eye);
    g_eye_logged = 0;
    reset_profile();
    g_opened_ms = static_cast<LONG>(log_seconds() * 1000.0);
    log_line("[%6.1fs] level load: opened %s", log_seconds(), name);
}

}  // namespace tpw::testing
