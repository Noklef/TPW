// An entry of a sound object's table of 0x2c-byte slots (its +0x28; the class name is invented): +0 a COM object of the sound library
// (its interface unidentified: slots 5, 7, 13, 14, 20, 38, 40, 42, 46, 48 and 59 are used here; 11 and 12 lock and unlock its data),
// the settings kept while it has none (sent again by SoundSlot_Restart): +4 a vector (+0x24 bit 2), +0x10 a pitch step (bit 1), +0x14
// a vector (bit 4), +0x20 a gain 0-127 (bit 8; 0x6d6db0 reads it as a size, 0x2400 when 0), +0x28 a value (bit 0x20). Reimplementation
// of 0x6d6dd0-0x6d6eb9 and 0x6d6ec0-0x6d7368; the names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d6dd0 (thiscall): +0 to +0xc, +0x14 to +0x1c and +0x24 cleared (+0x10 and +0x20 left). Returns the slot.
u8 *__fastcall SoundSlot_Init(u8 *slot);
// 0x6d6df0 (thiscall value; pops 4): with an object, its slot 7 (value). Returns its answer, or 0 without one.
u32 __fastcall SoundSlot_Call7(u8 *slot, void *, u32 value);
// 0x6d6e10 (thiscall (out, value); pops 8): *out = the object's slot 5 (value), or -1 without one. Returns out.
u32 *__fastcall SoundSlot_Call5(u8 *slot, void *, u32 *out, u32 value);
// 0x6d6e40 (thiscall object; pops 4): the old object (if any) stopped (its slot 14) and handed to the release list (releaselist.h), then the new one stored. Returns it.
u8 *__fastcall SoundSlot_Set(u8 *slot, void *, u8 *object);
// 0x6d6e70 (thiscall (out, a, b); pops 0xc): *out = the object's slot 59 (a, b), or -1 without one. Returns out.
u32 *__fastcall SoundSlot_Call59(u8 *slot, void *, u32 *out, u32 a, u32 b);
// 0x6d6eb0 (thiscall): 1 with an object, else 0.
u32 __fastcall SoundSlot_IsSet(u8 *slot);
// 0x6d6ec0 (thiscall out; pops 4): with an object, each setting whose +0x24 bit is set sent again (SoundSlot_SetDistances with +0x14,
// SetPosition with +4, SetPitch with +0x10, SetGain with +0x20, Set48 with +0x28, in that order; their answers dropped), then
// *out = its slot 13 (1); without one, *out = -1. Returns out.
u32 *__fastcall SoundSlot_Restart(u8 *slot, void *, u32 *out);
// 0x6d7060 (thiscall): the guard of 0x6d6f70's locked data {+0 the slot, +4 a flag, +8 a size}: with the flag, the sound system's
// (0xfb2724) slot 10 (an out on the stack, the size).
void __fastcall SoundSlotGuard_Dtor(u8 *guard);
// 0x6d7080 (thiscall out; pops 4): with an object, it stopped (slot 14), handed to the release list and +0 cleared; *out = 0. Returns out.
u32 *__fastcall SoundSlot_Free(u8 *slot, void *, u32 *out);
// 0x6d7130 (thiscall (out, vector); pops 8): +0x24 |= 2; with an object, *out = -1 if any of the three floats is a NaN, else its slot
// 20 (x, 0, z) (the y dropped, sic); without one, the vector kept at +4 and *out = -1. Returns out.
u32 *__fastcall SoundSlot_SetPosition(u8 *slot, void *, u32 *out, const u32 *vector);
// 0x6d71f0 (thiscall (out, steps); pops 8): +0x24 |= 1; with an object, *out = its slot 42 (the float for steps: 0 for 0, the table at
// 0x784010 for steps > 0 (through the x87), the dword at 0x784610 - 4 * steps for steps < 0); without one, steps kept at +0x10 and
// *out = -1. Returns out.
u32 *__fastcall SoundSlot_SetPitch(u8 *slot, void *, u32 *out, s32 steps);
// 0x6d7260 (thiscall (out, vector); pops 8): +0x24 |= 4; with an object, its slot 38 (x, y, 1) and slot 46 (z), *out = 0; without one,
// the vector kept at +0x14 and *out = -1. Returns out.
u32 *__fastcall SoundSlot_SetDistances(u8 *slot, void *, u32 *out, const u32 *vector);
// 0x6d72d0 (thiscall (out, gain); pops 8): +0x24 |= 8; with an object, *out = its slot 40 (gain / 127 as a float: fild of the unsigned
// gain, fmul 0x711c18); without one, gain kept at +0x20 and *out = -1. Returns out.
u32 *__fastcall SoundSlot_SetGain(u8 *slot, void *, u32 *out, u32 gain);
// 0x6d7330 (thiscall (out, value); pops 8): +0x24 |= 0x20; with an object, *out = its slot 48 (value); without one, value kept at +0x28
// and *out = -1. Returns out.
u32 *__fastcall SoundSlot_Set48(u8 *slot, void *, u32 *out, u32 value);

}  // extern "C"
