// The sound commands' runs: each command class (constructed on the poster's stack: SoundCmd_Ctor 0x51bb40 sets the base vtable
// 0x700b58 and the value at +4, the poster then its own one-slot vtable) is posted to the sound manager [0x802bcc], whose engine
// calls the slot as thiscall (out, target), pops 8, and the run applies the value to the target (a sound object: +0x20 and +0x24 its
// parts) and returns out. Reimplementation of the runs whose callees are ported: 0x6b5640, 0x6b5670, 0x6b56a0, 0x6b5770, 0x6b5790,
// 0x6b5840, 0x6b58b0, 0x6b5990, 0x6b59f0 and 0x6b5a50. The names are the vtables' (the commands' meanings are unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b5770 (vtable 0x700b2c, posted by SoundCategory_Release 0x51b620): target's slot 8 (out, the value).
u32 *__fastcall SoundCmd700b2c_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b56a0 (vtable 0x700b34): the value as a float (fld / fstp: a signalling NaN comes out quiet) stored at +0x2c of target's +0x24
// (Set2c_6b9220); *out = 0.
u32 *__fastcall SoundCmd700b34_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5790 (vtable 0x700b48): target's slot 7 (out, the value).
u32 *__fastcall SoundCmd700b48_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5840 (vtable 0x700b50): slot 9 of target's +0x24 (out, the value).
u32 *__fastcall SoundCmd700b50_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b5640 / 0x6b5670 (vtables 0x700b64 / 0x700b68, posted by 0x51bcf0 / 0x51bd30): slot 4 of target's +0x24 (out, a stack
// functor: its vtable 0x70a268 / 0x70a270 and a second dword left unset). The command's value is not read.
u32 *__fastcall SoundCmd700b64_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b68_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b59f0 / 0x6b5a50 / 0x6b5990 (vtables 0x700b70 / 0x700b74 / 0x700b78, posted by the sound update 0x51bd80 with 0x803ad8 /
// 0x803adc / 0x803ae0): Bytes44_Set(target's +0x24, out, 3 / 1 / 2, the value).
u32 *__fastcall SoundCmd700b70_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b74_Run(const u8 *command, void *, u32 *out, u8 *target);
u32 *__fastcall SoundCmd700b78_Run(const u8 *command, void *, u32 *out, u8 *target);

// 0x6b58b0 (vtable 0x700ba8): target's slot 6 (out, the value).
u32 *__fastcall SoundCmd700ba8_Run(const u8 *command, void *, u32 *out, u8 *target);

}  // extern "C"
