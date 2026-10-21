// An entry of a sound object's table of 0x2c-byte slots (its +0x28; the class name is invented): +0 a COM object of the sound library
// (its interface unidentified: slots 5, 7, 14 and 59 are used here), +0x20 a size (0x6d6db0: 0x2400 when 0). Reimplementation of
// 0x6d6dd0-0x6d6eb9; the names are invented.
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

}  // extern "C"
