// A coaster car's sounds (+0x54..+0x70: the two looped sounds +0x64 / +0x68, their parameters +0x5c / +0x60, the mode +0x38):
// reimplementation of 0x4392a0, 0x4393e0, 0x439670 and 0x439780.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x439780 (cdecl; name invented): a new mode (1, 2, 4, 8 or 16: 15, 30, 70, 90 or 50; any other 0) sets the +0x68 sound's
// parameter +0x5c to its value (unless 0) and becomes +0x38; then the same sound's parameter +0x60 set to value. Nothing reaches
// the sound while 0x785a2c is set; each Sound_SetParam's answer replaces +0x68.
void __cdecl CoasterCar_SetSoundMode(u8 *car, s32 mode, u32 value);

// 0x439670 (cdecl (coaster, car), the coaster unused; name invented): the looped sounds +0x64 and +0x68 told Sound_Forward6 and the scream
// +0x70 stopped (RSSE_StopScream), each when set and while 0x785a2c is 0 (read for each); then +0x54..+0x70 (8 dwords) cleared.
void __cdecl CoasterCar_StopSounds(u8 *coaster, u8 *car);

// 0x4392a0 (cdecl (coaster, car, quiet); name invented): CoasterCar_StopSounds; then, while 0x785a2c is 0, +0x54..+0x60 the linked
// elements 5-8 of the coaster's ride (Rides_GetLinkedElement of +0x24) and the looped sounds +0x64 / +0x68 its linked sounds 0 / 1 played
// in the category [0x803a3c] at the car's position (Sound_PlayRideElement with owner 0; x, y, z the __ftol of the floats +0x48, +0x4c,
// +0x50, taken z first); unless quiet, CoasterCar_SetSoundMode (2, 100).
void __cdecl CoasterCar_StartSounds(u8 *coaster, u8 *car, u32 quiet);

// 0x4393e0 (cdecl (link, car, x, y, z); name invented, unconfirmed): the car's scream kept up to its place on the track: the link's +0x9c
// entry (0x38 bytes) at the car's +0x74 (fistp). With the car screaming (flag 0x100): the entry's bit 2 moves the +0x6c sound (while
// 0x785a2c is 0) or, with none, plays one (KidSound_PlayGraded (1, 100, 100) unless the car's flag 0x10 or the link's +8 bit 0x40); bit 2
// clear moves a +0x6c sound (0x785a2c not looked at); the entry's bit 4 (and neither flag) keeps the +0x70 scream (moved, volume 0x4b,
// 0x4b) while 0x785a2c is 0; else the scream stopped (while 0x785a2c is 0), +0x70 cleared and the flag 0x100 cleared. Not screaming: bit
// 2 plays the +0x6c sound (neither flag, 0x785a2c 0); bit 4 (neither flag) starts a scream (RSSE_StartScream (0, +0x10 / 2 + 1, 0x4b,
// 0x4b)) when +0x70 is none and 0x785a2c 0, and sets the flag 0x100.
void __cdecl CoasterCar_UpdateScream(u8 *link, u8 *car, s32 x, s32 y, s32 z);
}
