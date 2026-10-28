// Sampling profiler (test tooling): to find what makes a hooked run slower than the original, a
// thread suspends the game's main thread about every millisecond (the timer's resolution decides),
// reads its EIP and counts it: in the game's code by 16-byte bucket, in the hook DLL by 16-byte
// bucket of its RVA, elsewhere by module. At exit the busiest buckets go to tpw.log; the DLL's RVAs
// map to ported functions with its PDB. Read-only: the thread is resumed at once after each read.
#pragma once

#include <windows.h>

namespace tpw::testing {

// Call on the thread to profile; `dll` is the hook DLL.
void start_profiler(HINSTANCE dll);
// Logs the profile (at exit).
void log_profile();

}  // namespace tpw::testing
