// The ride instances: the table at 0x7a4610 (an instance per ride index, from 1), with its live count 0x7a4084, its holes count 0x7a3ebc
// and its first hole 0x7aedf4 (a slot, the index - 1). Reimplementation of 0x463c80 (Ride_Destroy; agent B's hub of the 0x463060
// closure). Its calls into the unported 0x42a190 (the camera's following stopped) and into 0x436fa0 (CoasterLink_Destroy, coaster/registry:
// the ride's coaster link dropped, which calls Ride_Destroy back; in the game the port) go through an env.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What Ride_Destroy calls in the game, so that it can run outside it (the benches point these at recorders).
struct RidesEnv {
    void(__cdecl *stopFollowing)();     // 0x42a190
    void(__cdecl *forgetCoaster)(u32 ride);   // 0x436fa0
};
// Ride_Destroy's calls go to env (nullptr: the game's).
void Rides_SetEnv(const RidesEnv *env);

// 0x463c80 (cdecl (ride); name invented): nothing for ride 0 or an empty slot (the original's Debug_Printf (5, ..., ride) left out);
// else for its instance: Highlight_Clear when it is the highlighted one (0x7a1ce0), 0x7afda0 cleared when it is that, the env's
// stopFollowing when the camera follows the ride (Camera_IsFollowing (the index)), Highlight_Unflash with its flag 0x4000000 (+4), the
// env's forgetCoaster (ride), Instance_Dispose; its slot emptied. With no holes the count goes down, and unless it was the last slot the
// holes count becomes 1 and the first hole this slot (the last: the first hole goes down by one, sic). With holes: the last slot (holes +
// count - 1) drops the count and the trailing empty slots from the holes count (scanning down from the new end); any other slot is one
// hole more, the count one less, and the first hole when below it.
void __cdecl Ride_Destroy(u32 ride);
}
