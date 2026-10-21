// A GUI window that shows a level with an animated light (its child 0xd2d06; the callers about 0x4a9cc4 set four of them). Its state
// lives in the window's dwords +0xa0 (Window_DwordA0): 0 the light's position (16.16), 1 the clock when last moved, 2 the speed, 3 the
// phase (0-3). Reimplementation of 0x4a95b0 and 0x4a9fc0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a95b0 (cdecl (window, level); name unconfirmed): level 0 stops it: in phase 3 or 1 the phase becomes 2, the light told 0x20
// (Window_Notify1) and the position 0x200000 (any other phase: nothing). Any other level starts it: the speed ((level + 0x14) << 16) / 100
// (signed); in phase 0 or 2 first the clock (GuiClock_Now), the phase 3, the position 0x300000 and the light told 0x30.
void __cdecl LevelMeter_SetLevel(u8 *window, u32 level);

// 0x4a9fc0 (cdecl (window, message, a, b); name unconfirmed): the window's handler. On message 0x1e in a phase other than 0 the
// position moves on by twice the speed (dword 1 read, not used): phase 1 wraps to 0 past 0x1f (in 16.16); phase 2 past 0x2f turns to
// phase 0 at 0x30; phase 3 past 0x3f turns to phase 1 at 0. The light is told the position's integer part (in any other phase, 0x1e), the
// position and the clock (GuiClock_Now) are kept. Every message then goes to Window_CallDefault (its result returned).
s32 __cdecl LevelMeter_OnMessage(u8 *window, u32 message, u32 a, u32 b);
}
