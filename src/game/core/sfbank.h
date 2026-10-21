// A sound resource (vtable 0x70ab00; the class name is invented) and the SoundFont bank built on it (vtable 0x711ba8: +0x14 the SoundFont
// synthesiser (sfman.h) or null, +0x18 flags, bit 0 loaded). Reimplementation of 0x6d4970, 0x6d4aa0, 0x6d4cd0, 0x6d4b50, 0x6d49f0, 0x6d4a40 and 0x6d4a20 (slots 5 and 6, 0x6d4b00 and 0x6d4b10, are in methods.h); the names are invented.
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
// 0x6d4b50 (thiscall (out, voice); pops 8; slot 15): without a synthesiser *out = -1; else the program voice[0] - 1 (taken modulo the
// synthesiser's programs' count +0x18 when above it, unsigned), sent by SfMan_Send: for voice +0x14 bit 4 a message 4 with +0x20, bit 1
// a 0x10 with +0x28, bit 16 a 0x20 with __ftol(+0x5c * 127.0), each with 0x3c; then the program's name (SfMan_GetName into a String) picks
// the note: 0x30 for "_1ol" in it, else 0x24 for "_2ol", else 0x18 for "_3ol", else 0x3c; a 1 with 0x7f and that note sent. *out = 0.
// The answers are not looked at. Returns out.
u32 *__fastcall SfBank_Play(u8 *self, void *, u32 *out, const u8 *voice);
// 0x6d49f0 (thiscall (source, synth); pops 8): SoundResource_Ctor (soundindex.h), +0x18 = 0, +0x14 = synth, vtable 0x711ba8. Returns the object.
u8 *__fastcall SfBank_Ctor(u8 *self, void *, u8 *source, u8 *synth);
// 0x6d4a40 (thiscall): vtable 0x711ba8; SfBank_Unload, then SoundResource_Dtor's work.
void __fastcall SfBank_Dtor(u8 *self);
// 0x6d4a20 (thiscall (flags); pops 4; slot 2): SfBank_Dtor, the block freed (Mem_Free) for flags bit 0. Returns the object.
u8 *__fastcall SfBank_DeletingDtor(u8 *self, void *, u32 flags);

}  // extern "C"
