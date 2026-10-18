// A countdown in frames before a deferred step: 0x4a9380 arms it (0x7cc150 = 3, the pending flag 0x7cc190 = 1), the frame update
// 0x48a580 steps it each frame (0x4a9350) and runs 0x4a9180 once it is 0 while the flag is set (that clears the flag). What the step is
// for is not known yet. Reimplementation of 0x4a9350.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a9350 (cdecl): the counter at 0x7cc150 stepped down if above 0; 1 when it is 0 and 0x7cc190 is
// set, else 0.
u32 __cdecl Countdown7cc150_Step();
}  // extern "C"
