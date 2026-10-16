// The weather renderer's small setters: reimplementation of 0x57d910, 0x57dc40 and 0x57dc70.
//
// The renderer is the object at [0x8bcbac] (allocated at 0x57d120, built by 0x57d1d0). Its +0 is
// the kinds of weather showing (bit 1 and bit 2; the constructor sets 1), +4 asks for its
// textures to be reloaded (cleared by the loader, 0x57d850), +0x3318 is how many drops fall
// (read by the drawing, 0x57f02b), and +0x3338 / +0x333c are the fall speeds of kinds 1 and 2
// (5.0 and 0.7 from the constructor; 0x580113 picks one by bit 1).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x57d910 (thiscall; count): +0x3318 = count (Weather_Update clamps it to [0, [0x785530]]).
void __fastcall WeatherFx_SetDropCount(u8 *fx, void *, u32 count);

// 0x57dc40 (thiscall; speed, a float moved as a dword): the fall speed of each kind showing
// (+0x3338 when bit 1, +0x333c when bit 2).
void __fastcall WeatherFx_SetFallSpeed(u8 *fx, void *, u32 speed);

// 0x57dc70 (thiscall): +4 = 1, the textures to be reloaded.
void __fastcall WeatherFx_RequestReload(u8 *fx);
}
