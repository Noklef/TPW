#include "testing.h"

#include "fpu_watch.h"
#include "log.h"
#include "trace.h"

namespace tpw::testing {

void start(HINSTANCE dll, const char *game_dir, unsigned hook_count) {
    char log_path[MAX_PATH];
    lstrcpynA(log_path, game_dir, MAX_PATH);
    lstrcatA(log_path, "tpw.log");
    log_open(log_path);
    // The link time from our own PE header: unlike __DATE__, it changes with any rebuilt file.
    const auto *self = reinterpret_cast<const BYTE *>(dll);
    const auto *nt = reinterpret_cast<const IMAGE_NT_HEADERS *>(
        self + reinterpret_cast<const IMAGE_DOS_HEADER *>(self)->e_lfanew);
    log_line("tpw hook DLL (linked %08X, test tooling on): %u hooks registered",
             (unsigned)nt->FileHeader.TimeDateStamp, hook_count);
}

void after_hooks(const char *ini) {
    if (GetPrivateProfileIntA("tpw", "trace", 1, ini)) install_trace();
    // tp.exe imports dinput.dll, so DllMain runs on the game's main thread at startup.
    start_fpu_watch();
}

void stop() { log_line("[%6.1fs] process exiting", log_seconds()); }

}  // namespace tpw::testing
