#include "load_timer.h"

#include <windows.h>

#include <cstring>

#include "log.h"
#include "profiler.h"
#include "../game/world/camera.h"

namespace tpw::testing {

namespace {

const BYTE *eye_now() { return reinterpret_cast<const BYTE *>(&gCamera_790990[12]); }   // the camera's eye: x, y, z (floats)
constexpr LONG kMaxOpensLogged = 4000;

volatile LONG g_opens;

DWORD WINAPI timer_thread(void *) {
    bool down = false, running = false;
    double started = 0;
    BYTE eye[12] = {};
    for (;;) {
        Sleep(10);
        const bool now = (GetAsyncKeyState(VK_F9) & 0x8000) != 0;
        const bool pressed = now && !down;
        down = now;
        if (!running) {
            if (!pressed) continue;
            running = true;
            started = log_seconds();
            std::memcpy(eye, eye_now(), sizeof eye);
            reset_profile();
            log_line("[%6.1fs] level load: F9, timer started", started);
        } else if (pressed || std::memcmp(eye, eye_now(), sizeof eye) != 0) {
            log_line("[%6.1fs] level load: %.2f s, ended by %s", log_seconds(), log_seconds() - started, pressed ? "F9" : "the camera's eye moving");
            log_profile();
            reset_profile();
            running = false;
        }
    }
}

}  // namespace

void start_load_timer() {
    if (const HANDLE thread = CreateThread(nullptr, 0, timer_thread, nullptr, 0, nullptr)) CloseHandle(thread);
}

void load_timer_file_opened(const char *name) {
    if (InterlockedIncrement(&g_opens) <= kMaxOpensLogged) log_line("[%6.1fs] open %s", log_seconds(), name ? name : "?");
}

}  // namespace tpw::testing
