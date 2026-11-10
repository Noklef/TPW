// The sparkle on the dialog [0x7cc4b4]. Reimplementation of 0x4b8fd0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct Dialog7cc4b4SparkleEnv {
    u8 *(__fastcall *readMode)(u8 *setting);                                        // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *fn)());                                      // crt_atexit, raw
    u16(__fastcall *getRect)(u8 *window, void *, u8 *rect, u32 inner);              // Window_GetRect
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z);                           // Particle_Spawn
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt
};
void Dialog7cc4b4_SetSparkleEnv(const Dialog7cc4b4SparkleEnv *env);

extern "C" {

// 0x4b8fd0 (cdecl): nothing without the dialog or in the mode 2 of the setting 0xfb3b7c (read the first time, guard 0xfb34bd bit 0, atexit 0x5d1750). The child 0x1e0ea's rectangle (not checked), the middle of its words +6, +0xa as (mid * 75000, a dword) / 0x600: the effect 0x62 at (50000, 0, that), the sound 4 of the category [0x803a48] at (0, 0, 0).
void __cdecl Dialog7cc4b4_Sparkle();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gDialog7cc4b4sparkle_fb34bd, 0x00fb34bd, 0x1);
