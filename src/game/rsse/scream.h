// The RSSE's screams (the riders' screams of a ride: 0x555edb starts one, 0x439467 / 0x4394af move and level it): reimplementation of
// their start (0x551130), volume (0x551290), move (0x5512d0) and stop (0x551300). A scream is a sound handle; the volume is the mean of two inputs as
// Sound_SetParam's parameter 6 (signed halving, at most 100, at least 0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x551130 (cdecl (scream, level, a, b, x, y, z); name from the game's message "RSSE: Started screaming without stopping first."): with
// a scream already (not 0) that message given to the empty Debug_Printf (0x5da3c0, not called here) and 0. Level 0: 0. Else a sound by
// the level (0x47 for 1, 0x48 below 4 (signed: so for a negative level too), 0x49 below 8, 0x4a from 8 on) played in the kids'
// category [0x803a24] at (x, y, z) (Sound_PlayAt with owner 0); when that gives one, its volume set as RSSE_SetScreamVolume does
// (its answer not kept). The played sound is returned (0: none).
u32 __cdecl RSSE_StartScream(u32 scream, s32 level, s32 a, s32 b, s32 x, s32 y, s32 z);

// 0x551290 (cdecl (scream, a, b); name invented): 0 for no scream; else Sound_SetParam(scream, 6, (a + b) / 2 (32-bit sum, signed
// halving) cut to 0..100), whose answer is returned.
u32 __cdecl RSSE_SetScreamVolume(u32 scream, s32 a, s32 b);

// 0x5512d0 (cdecl (scream, x, y, z); name invented): 0 for no scream; else Sound_SetPosition(scream, x, y, z)'s answer.
u32 __cdecl RSSE_MoveScream(u32 scream, s32 x, s32 y, s32 z);

// 0x551300 (cdecl (scream); name invented): a scream (not 0) stopped (Sound_Stop). (The original's EAX is the scream or Stop's
// leftover: no caller reads it.)
void __cdecl RSSE_StopScream(u32 scream);
}
