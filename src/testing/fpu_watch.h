// FPU watcher (test tooling): the float ports assume the x87 runs at 24-bit precision, which
// Direct3D sets when the game creates its device. A wrapper (dgVoodoo), software rendering or
// the game itself (0x591220/0x591250 set 24 and 64 bits) can change it, so this samples the main
// thread's FPU control word every second and logs each change. Read-only: the thread is
// suspended for the read and resumed at once.
#pragma once

namespace tpw::testing {

// Call on the thread to watch.
void start_fpu_watch();

}  // namespace tpw::testing
