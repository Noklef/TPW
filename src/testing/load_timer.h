// Level load timer (test tooling), to compare a hooked run's load with a run with the hooks off. F9, pressed as the load is clicked,
// starts it and resets the profile. The camera's eye first moving (it holds still through a load, and the game writes it as the level
// starts) or a second F9 ends it, and logs the time and the profile of the load alone. Each file opened to read goes to the log with its
// time, a check on both ends: a save's open and the last open of the burst bracket the load.
#pragma once

namespace tpw::testing {

void start_load_timer();
// From the CreateFileA trace, for each file opened to read.
void load_timer_file_opened(const char *name);

}  // namespace tpw::testing
