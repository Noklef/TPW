// A movie's life: the player made, its thread started, the loop that shows the frames, all let go at the end.
// Reimplementation of 0x66dff0, 0x66eef0, 0x66efd0, 0x66f050, 0x66f090, 0x66f4e0 and 0x670710; the names are invented, unconfirmed. The handle is the settings (the first dword the player).
#pragma once

#include "../../../re/types.h"

// What the movie's life calls (the ports by default; the bench's recorders: each has its own bench).
struct MovieLifeEnv {
    void *(__cdecl *calloc)(u32 count, u32 size);                      // 0x67fcb0
    u32(__fastcall *open)(const char *file, u8 *player);               // MovieFile_Open
    void(__fastcall *readHeader)(u8 *player);                          // MoviePlayer_ReadHeader
    void(__fastcall *soundStart)(u8 *player);                          // 0x672850
    void(__fastcall *bufferStart)(u8 *player);                         // 0x6700d0
    u32(__cdecl *beginThread)(u32 start, u32 stack, u8 *settings);     // CRT_beginthread
    void(__cdecl *endThread)();                                        // 0x67e180
    u32(__fastcall *startThread)(u8 *player);                          // MovieControl_StartThread
    void(__fastcall *readAhead)(u8 *player);                           // MovieControl_ReadAhead
    void(__fastcall *createStream)(u8 *player);                        // MovieSound_CreateStream
    void(__fastcall *showFrame)(u8 *player, u8 *entry);                // 0x670c20
    void(__fastcall *present)(u8 *player);                             // MovieSurface_Present
    void(__fastcall *swapFields)(u8 *player);                          // MoviePlayer_SwapFields
    void(__fastcall *stopThread)(u8 *player);                          // MovieControl_StopThread
    void(__fastcall *releaseStream)(u8 *player);                       // MovieSound_ReleaseStream
};
void MovieLife_SetEnv(const MovieLifeEnv *env);

extern "C" {

// 0x66dff0 (cdecl): a cleared player of 0x670 bytes, 0 without one; its settings' +0xa0 the value. The file opened: failing, the player freed and 0. Else its header read, the
// settings' first dword the player, the sound and the buffer started, the state +0x9c 1. The settings.
u8 *__cdecl MovieLife_Create(const char *file, u32 value, u8 *settings);

// 0x66eef0 (cdecl): the time played (+0x61c - +0x618) into the settings' +0x90, the file, the sound and the surfaces let go, the state +0x9c 7, the settings' first dword 0, the player freed. 0.
u32 __cdecl MovieLife_Destroy(u8 *settings);

// 0x66efd0 (cdecl): with the settings' flag 0x10000: a thread on 0x66f050, its handle in +0xac (-1: answered). The state +0x9c 2, each 20 ms, until it was 3; then 3 again. 0.
// Without the flag the movie played here (0x66f050). 1.
u32 __cdecl MovieLife_Start(u8 *settings);

// 0x66f050 (cdecl): the movie played; with the player's +0x3e0 let go. With a thread (the settings' flag 0x10000, as it was before) the thread ended.
void __cdecl MovieLife_ThreadMain(u8 *settings);

// 0x66f090 (thiscall on the settings): the loop. With the flags 0x30000 the thread's priority +0xa8. The sound's thread started (failing: only the stream let go and the state 6). The state +0x9c set to 3 each
// turn, the old one acted on: 4 waits, 5 ends, only 3 goes on: the reader kept ahead, the sound's stream made once, a frame due shown (late by more than 2, +0x98, the kinds 1, 2,
// 5, 6 only and not presented), the settings' callback +0xb0 with 0x40000 (nonzero: ends), its place cleared and the rings moved on. No frames left, no sound (or the sound
// not looked after, no sound's chunks and its stream not playing): ends. With a sound, a wait of up to 151 turns for it to catch up: the distance into +0x98 (not below 0). At the
// end the time +0x61c, the sound's thread and stream let go, the state 6.
void __fastcall MovieLife_Run(u8 *settings);

// 0x66f4e0 (cdecl): MovieLife_Create; the player's +0x3e0 set; MovieLife_Start: the settings when that answered 0, else 0.
u8 *__cdecl MovieLife_OpenAndStart(const char *file, u32 value, u8 *settings);

// 0x670710 (thiscall on the player): with +0x3c: its seven tables freed and cleared, an overlay (+0x4c 2) hidden, and after 20 ms the settings' +0x24 (unless flag 0x4000000), +0x20 and +0x1c
// (unless flag 0x800) released and cleared; with +0x3e4 the settings' +0x10 too. +0x3c cleared.
void __fastcall MovieLife_ReleaseSurfaces(u8 *player);
}
