// A coaster car's sounds (+0x54..+0x70: the two looped sounds +0x64 / +0x68, their parameters +0x5c / +0x60, the mode +0x38):
// reimplementation of 0x439780.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x439780 (cdecl; name invented): a new mode (1, 2, 4, 8 or 16: 15, 30, 70, 90 or 50; any other 0) sets the +0x68 sound's
// parameter +0x5c to its value (unless 0) and becomes +0x38; then the same sound's parameter +0x60 set to value. Nothing reaches
// the sound while 0x785a2c is set; each Sound_SetParam's answer replaces +0x68.
void __cdecl CoasterCar_SetSoundMode(u8 *car, s32 mode, u32 value);
}
