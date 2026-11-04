// State dump (test tooling): F11 writes tp.exe's data section to tpwdump.bin beside the game, so that a run that draws and one
// that does not can be compared offline. Taken from a separate thread, so a few words may be mid-update. Each press overwrites.
#pragma once

namespace tpw::testing {

void start_data_dump(const char *ini);

}  // namespace tpw::testing
