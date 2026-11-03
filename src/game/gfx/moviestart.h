// A movie's start, its end and its stop, as the game runs them on the video library: reimplementation of 0x51b010, 0x51b300, 0x51b350 and 0x51b3e0 (names invented, unconfirmed). State: [0x802bc0] movies allowed,
// [0x802bbc] one is on, [0x802bb4] the library's player, [0x802404] and [0x802408] the two surfaces' interfaces, 0x8023f0 the 0x7b8 bytes of settings handed to the library, [0x802ba8] the volume (0-1023).
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct MovieStartEnv {
    u32(__cdecl *soundWord)();                                          // Sound_QueryWord0
    u32(__cdecl *soundOn)();                                            // Sound_IsOn
    u32(__fastcall *flip)(u8 *renderer);                                // 0x564210 (agent B's)
    u8 *(__cdecl *open)(u32 file, u32 mode, u8 *settings);              // 0x66f4e0 (agent C's)
    u32(__cdecl *setVolume)(u8 *player, s32 volume);                    // 0x66f520 (agent C's)
    u32(__cdecl *playing)(u8 *player);                                  // 0x66f540 (agent C's)
    u32(__cdecl *stop)(u8 *player);                                     // 0x66f580 (agent C's)
};

void MovieStart_SetEnv(const MovieStartEnv *env);

extern "C" {
// 0x51b010 (cdecl (file, mode, x, y, overlay)): 0 unless movies are allowed, file is set and none is on. The settings zeroed; [0x802bb0], [0x8023e8] = 0; +0xc = the renderer [0x879a24]'s dword 0. Its back (+0x14) and front (+0x10) surfaces
// QueryInterface'd for 0x707210 (a failure: 0), then each for 0x707220, front to [0x802404], back to [0x802408] (a failure: 0, nothing undone). The settings' fields: [0x802460] = [0x7a1968], [0x802498] = 2, [0x8023f8] = 1 (7 with overlay),
// [0x802464] = [0x7a1930] * 352 / 480, the flags [0x8023f4] (0x3030180, 0xb030180 with bit 0 of [0x7a1a8c]; x -1: 4, else 1 and [0x802458] = x; y -1: 8, else 2 and [0x80245c] = y; overlay: 0x200; the sound word set: 0x200000; sound on: 0x800000),
// [0x802444] = the sound word. The back surface, when set, cleared twice (Blt with a blank DDBLTFX, flags 0x1000400, until it answers 0; a flip after each). The library's open: its player in [0x802bb4]; none: the interfaces released and nulled.
// [0x802bbc] = 1 either way. With a player, its volume [0x802ba8] * 10000 / 1023 - 10000 and the answer 1.
u32 __cdecl Movie_Start(u32 file, u32 mode, s32 x, s32 y, u32 overlay);

// 0x51b300 (cdecl): 1 when no movie is on or none allowed. Else Sleep(10), and 1 (then none on) when there is no player or it has finished, 0 while it plays.
u32 __cdecl Movie_Finished();

// 0x51b350 (cdecl): with a movie on and allowed: a player still playing stopped, the interfaces released, all four words 0.
void __cdecl Movie_Stop();

// 0x51b3e0 (cdecl): Movie_Stop's work, and movies no longer allowed.
void __cdecl Movie_Forbid();
}
