// Level load timer (test tooling), to compare a hooked run's load with a run with the hooks off. The game opening a save or level file
// starts it and resets the profile. F9, pressed as the world appears, logs the time and the profile of the load alone. The camera's eye
// first moving logs its own time, as a check on the key press. A later open restarts it: the menu may read a save's header first.
#pragma once

namespace tpw::testing {

void start_load_timer();
// From the CreateFileA trace, for each file opened to read.
void load_timer_file_opened(const char *name);

}  // namespace tpw::testing
