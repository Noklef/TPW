// A channel's sound source (the channel's +0x18; soundchannel.h): the back-ends on a common base (vtable 0x7119b8), among them the
// classes with vtables 0x711920 (set in 0x6d1e30; the QSound mixer's: its open 0x6d2110 hands +0x20 to QSWaveMixInitEx) and 0x711b08 (set
// in 0x6d2d60). The class names are invented. Reimplementation of 0x6d1f40, 0x6d20a0, 0x6d21d0, 0x6d2290, 0x6d3020, 0x6d3070 and 0x6d37b0;
// the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d21d0 (slot 16 of 0x711920, thiscall (out, low), pops 8): Obj_SetRateAtC (11025 for low, else 22050; its out the low argument's
// own stack slot, sic): negative (the rate already so), *out = 0. Else the device singleton's (SoundDevice_Get) SoundDevice_SetFormat
// (out, stereo, 16 bits, the rate +0xc). Returns out.
u32 *__fastcall SoundSource711920_SetRate(u8 *source, void *, u32 *out, u32 low);

// 0x6d1f40 (thiscall (out, info), pops 8; called by the open 0x6d2110): with no mixer config (+0x20) yet: Obj_Pick40Or44 (+4 = info's +0x44
// or +0x40) and Obj_SetRateAtC (info's +0x68), both answering into info's own stack slot (sic); the config a new zeroed 0x40-byte block
// (Mem_Alloc; not checked): +0 its size 0x40, +0xc the device singleton's sound (SoundDevice_Get +0), +0x14, +0x2c, +0x30, +0x38 and +0x3c
// = +4, +0x18 = 0; with hardware 3D streaming buffers (a SoundCaps holder of the device's sound, SoundCaps_Max3DStreaming) +0x34 and +0x28
// = +4 too; with info's +0x4c bit 1 +0x24 = info's +0x48. +0x1c a new zeroed block of 4 * +4 bytes, the config's +8 = +0xc (the rate).
// *out = 0 either way. Returns out.
u32 *__fastcall SoundSource711920_Configure(u8 *source, void *, u32 *out, const u8 *info);

// 0x6d20a0 (thiscall; called by the open 0x6d2110): +0x24 = 0x10000, | 0x20000 with hardware 3D streaming buffers (a SoundCaps holder of
// the device singleton's sound, SoundCaps_Max3DStreaming).
void __fastcall SoundSource711920_SetFlags(u8 *source);

// 0x6d2290 (slot 3 of 0x711920, thiscall (out), pops 4; this unused): *out = the device singleton's sound (SoundDevice_Get +0); AL = 1 when
// it is not null (EAX's upper bytes the sound's, sic).
u32 __fastcall SoundSource711920_GetSound(u8 *source, void *, u32 *out);

// 0x6d37b0 (slot 35 of 0x711b08, thiscall (out, front, top), pops 0xc): without the COM object +0x1c *out = -1; else its slot 11
// (front's x, 0, front's z, top's x, y, z: six floats moved through the x87, a signalling NaN quieted) answered into *out. Returns out.
u32 *__fastcall SoundSource711b08_SetOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *top);

// 0x6d35d0 (slot 30 of 0x711b08, thiscall (out, index, vector), pops 0xc; name invented): the slot at the index (+0x28, 0x2c bytes each;
// not checked) asked for its position (SoundSlot_GetPosition: out, vector). Returns out.
u32 *__fastcall SoundSource711b08_GetSlotPosition(u8 *source, void *, u32 *out, u32 index, u8 *vector);

// 0x6d3020 (thiscall; name invented): +0x30 = 2 * +4 + 1 events, +0x2c a new array of them (Mem_Alloc; not checked), each
// CreateEventA (0, 0, 0, 0).
void __fastcall SoundSource711b08_CreateEvents(u8 *source);
// 0x6d3070 (thiscall; name invented): the caps (0x24 bytes, the size set) from +0x18's COM slot 11; 1 when their flags +4 have bits
// 8, 2 and 0x40, else 0.
u32 __fastcall SoundSource711b08_HasCaps(u8 *source);

}
