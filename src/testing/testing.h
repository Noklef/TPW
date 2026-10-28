// TEST TOOLING, NOT PART OF THE DECOMPILATION.
//
// Everything under src/testing exists to diagnose in-game runs: tpw.log, the startup trace
// (trace.h), the FPU watcher (fpu_watch.h) and the sampling profiler (profiler.h). None of it reimplements or fixes game code, and
// game code (src/game) must never include it; CMake refuses to configure if it does. The hook
// DLL calls in through this header only, and only when built with TPW_TEST_TOOLING=ON, so a
// build with it OFF is the reimplementation alone. See .docs/06-in-game-testing.md.
#pragma once

#include <windows.h>

namespace tpw::testing {

// Opens tpw.log in `game_dir` (with trailing backslash) and logs the DLL's link time.
void start(HINSTANCE dll, const char *game_dir, unsigned hook_count);

// After the hooks are installed, on the game's main thread: the startup trace (unless tpw.ini
// says trace=0) and the FPU watcher.
void after_hooks(const char *ini);

void stop();

}  // namespace tpw::testing
