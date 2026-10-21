// A sound resource (vtable 0x70ab00; the class name is invented) and the SoundFont bank built on it (vtable 0x711ba8: +0x14 the SoundFont
// synthesiser (sfman.h) or null, +0x18 flags, bit 0 loaded). Reimplementation of 0x6d4970, 0x6d4aa0 and 0x6d4cd0 (slots 5 and 6, 0x6d4b00 and 0x6d4b10, are in methods.h); the names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d4970 (thiscall a; pops 4; slot 7 of both vtables): the resource's format from its slot 6 (thiscall (out, &format, a): format
// points at a 0x30-byte buffer on the stack, which the slot fills or repoints); 0 when the slot's answer (*its return) < 0, else the
// length in ms: the dword +4 (fild), halved for +0x1b bit 0 and again for +0x1a == 0x10, over the word +0x18 (fild), times 1000.0 (0x711ba4),
// stored as a float and loaded again, rounded to an int (fistp: 0x80000000 out of range).
s32 __fastcall SoundResource_LengthMs(u8 *self, void *, u32 a);
// 0x6d4aa0 (thiscall (out, path, b); pops 0xc; slot 3): *out = 0 when loaded (+0x18 bit 0) already; else with a synthesiser, its
// SfMan_LoadBank (path, 0) and, when that answers >= 0, +0x18 |= 1 and *out = 0; else *out = -1. Returns out. (b unused.)
u32 *__fastcall SfBank_Load(u8 *self, void *, u32 *out, u32 path, u32 b);
// 0x6d4cd0 (thiscall; slot 4): with a synthesiser, its SfMan_UnloadBank, then +0x14 and +0x18 cleared.
void __fastcall SfBank_Unload(u8 *self);

}  // extern "C"
