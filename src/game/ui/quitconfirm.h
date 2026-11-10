// The answer of the quit message box. Reimplementation of 0x4c0830; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders).
struct QuitConfirmEnv {
    u8 *(__fastcall *readMode)(u8 *setting);                  // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *fn)());                // crt_atexit, raw
    void(__fastcall *setMode)(u8 *setting, void *, u32 mode); // Setting_SetMode
};
void QuitConfirm_SetEnv(const QuitConfirmEnv *env);

extern "C" {

// 0x4c0830 (cdecl (answer), a message box's callback): with the answer 0, [[0xf82884] +0x14] = 2 and the setting 0xfb3b7c (read the first time, guard 0xfb34bc bit 0, atexit 0x5d1750) given the mode 1.
void __cdecl QuitConfirm_Answered(u32 answer);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gQuitconfirm_fb34bc, 0x00fb34bc, 0x1);
