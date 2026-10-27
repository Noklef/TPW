// A channel's sound source (the channel's +0x18; soundchannel.h): the back-ends on a common base (vtable 0x7119b8), among them the
// classes with vtables 0x711920 (set in 0x6d1e30) and 0x711b08 (set in 0x6d2d60). The class names are invented. Reimplementation of
// 0x6d21d0 and 0x6d37b0; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d21d0 (slot 16 of 0x711920, thiscall (out, low), pops 8): Obj_SetRateAtC (11025 for low, else 22050; its out the low argument's
// own stack slot, sic): negative (the rate already so), *out = 0. Else the device singleton's (SoundDevice_Get) SoundDevice_SetFormat
// (out, stereo, 16 bits, the rate +0xc). Returns out.
u32 *__fastcall SoundSource711920_SetRate(u8 *source, void *, u32 *out, u32 low);

// 0x6d37b0 (slot 35 of 0x711b08, thiscall (out, front, top), pops 0xc): without the COM object +0x1c *out = -1; else its slot 11
// (front's x, 0, front's z, top's x, y, z: six floats moved through the x87, a signalling NaN quieted) answered into *out. Returns out.
u32 *__fastcall SoundSource711b08_SetOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *top);

// 0x6d3020 (thiscall; name invented): +0x30 = 2 * +4 + 1 events, +0x2c a new array of them (Mem_Alloc; not checked), each
// CreateEventA (0, 0, 0, 0).
void __fastcall SoundSource711b08_CreateEvents(u8 *source);
// 0x6d3070 (thiscall; name invented): the caps (0x24 bytes, the size set) from +0x18's COM slot 11; 1 when their flags +4 have bits
// 8, 2 and 0x40, else 0.
u32 __fastcall SoundSource711b08_HasCaps(u8 *source);

}
