// The movie's sound: its device made and asked, and the thread that keeps its stream filled from the sound's ring.
// Reimplementation of 0x6724a0, 0x672520, 0x672850 and 0x672910; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; the bench's recorders).
struct MovieSoundThreadEnv {
    void(__cdecl *endThread)();                                          // 0x67e180
    u32(__fastcall *distance)(u8 *player);                               // 0x672730
    u32(__fastcall *bytes)(void *buffer);                                // 0x672950
    u32(__stdcall *create)(const void *guid, void **place, void *outer); // 0x6d76e0: DirectSoundCreate
};
void MovieSoundThread_SetEnv(const MovieSoundThreadEnv *env);

extern "C" {

// 0x6724a0 (cdecl, the thread on the player): after 10 ms, while the flag +0x3c0 was set (it is set again each turn: the player's stop clears it): with a stream (+0x368) and its play
// distance at least the next block's bytes (+0x374's entry), the block written; +0x3bc counted; 10 ms. The thread ended.
void __cdecl MovieSoundThread_Main(u8 *player);

// 0x672520 (fastcall (player, distance)): with a block waiting (+0x388): unless simulated (+0x3cc), the stream locked at +0x370 for its bytes (restored and locked again until that answers
// 0), copied in and unlocked; the place on (modulo +0x36c), +0x374 counted, the block's time in +0x394, +0x388 counted down, the block cleared. Without one: the distance of silence (a
// failed lock: none written), and the place on, +0x394 counted.
void __fastcall MovieSoundThread_Write(u8 *player, u32 distance);

// 0x672850 (thiscall on the player): with the settings' flag 0x800000: the device +0x54 made when there is none (+0x3dc set). None still: the primary +0x58 cleared. Else its capabilities
// asked, bit 5 of the flags in +0x3d0, and +0x3d4 the primary's bytes with flag 0x400000 and a primary, else 0x8000. 0.
u32 __fastcall MovieSoundThread_OpenDevice(u8 *player);

// 0x672910 (fastcall (place, window)): DirectSoundCreate into the place, cooperative level 2 on the window; a failure (the object released and cleared, when it was the level): 1. Else 0.
u32 __fastcall MovieSoundThread_CreateDevice(void **place, u32 window);
}
