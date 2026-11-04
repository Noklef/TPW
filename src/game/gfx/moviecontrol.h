// The movie's reader kept ahead, its rewind, the sound's volume, its stop and the sound's thread. Reimplementation of 0x66f380, 0x66f480, 0x66f520, 0x66f580, 0x6727c0 and 0x672800; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the port and the CRT's by default; the bench's recorders).
struct MovieControlEnv {
    s32(__fastcall *next)(u8 *player);                                           // MovieChunks_Next
    u32(__cdecl *beginThread)(u32 start, u32 stackSize, u8 *argument);           // 0x67e050 (the CRT's _beginthread, not ported)
};
void MovieControl_SetEnv(const MovieControlEnv *env);

extern "C" {

// 0x66f380 (thiscall on the player): with nothing kept (+0x3ac -1) a rewind; still nothing: done. Under 0x16 chunks waiting in each ring (+0x384, +0x388, signed), the next chunk (its answer in +0x3b0): the end: a rewind. A sound chunk (2): with the
// settings' +0x88, only with +0x388 under 0x16: chunks until one is not sound, the last in +0x3ac. A video chunk (1): with the settings' +0x84, the next into +0x3ac: another video one clears +0x84; sound: chunks until one is not sound, into +0x3ac.
void __fastcall MovieControl_ReadAhead(u8 *player);

// 0x66f480 (thiscall on the player): with the flag 0x8000 of the settings +4: Sleep (0), the file back to the place +0x3b8 (mmioSeek from its start), the next chunk's answer in +0x3ac. Without: +0x3ac -1.
void __fastcall MovieControl_Rewind(u8 *player);

// 0x66f520 (cdecl (settings, volume)): the sound +0x5c's SetVolume when there is one. 0.
u32 __cdecl MovieControl_SetVolume(u8 *settings, u32 volume);

// 0x66f580 (cdecl (settings)): -1 in the state (+0x9c) 0, 6 or 7, or without a player (+0). Else the state 5 (InterlockedExchange), and Sleep (10) until the player's thread, by its +0x3e0, leaves it at 7 (set) or 6. 0.
s32 __cdecl MovieControl_Stop(u8 *settings);

// 0x6727c0 (thiscall on the player): the flag +0x3c0 set (InterlockedExchange); the thread 0x6724a0 started with the player, its handle in +0x3c4, its priority 15. 1 when it did not start.
u32 __fastcall MovieControl_StartThread(u8 *player);

// 0x672800 (thiscall on the player): 1 without a thread; else the flag +0x3c0 cleared, and again after Sleep (10) until the old value (the thread's answer) is not 0. 0.
u32 __fastcall MovieControl_StopThread(u8 *player);
}
