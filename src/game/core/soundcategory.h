// A sound category (the statically linked sound library's: cat_<name>BANK.map and cat_<name>SFX.map, tools/sndmap.py): reimplementation of its
// SFX.map loading, 0x6bd7a0 and 0x6c0360-0x6c0850 (the header check 0x6c0310 is Obj_CheckId, core/methods).
//
// The category object: +0x30 a Handle (the folder its maps are opened in), three std::deque<u32> (dequeiter.h's layout) at +0x38 (A: each
// category's record, a 0x34-byte object whose +0x24 is a PtrList of the banks its sounds use), +0x68 (B: the banks) and +0x98 (C: the BANK.map's
// banks' indices in B), +0xc8 the category being loaded (its index in A) and +0xcc a byte: the SFX.map's pick weights are delta-coded.
//
// SFX.map is a memory image fixed up in place (offsets in bytes): a category (0x18) {u32 sounds, u32 -> its sounds, ...}; a sound (0x14) {u32
// number, u32 variations, u32 -> its variations, u32, u16 flags at +0x10, ...}; a variation (0x2a) {u16 samples, u16, u32 links, u32 -> its samples,
// ..., u16 at +0x10 and +0x12, u32 pick weight at +0x1e, f32, u32 -> its links at +0x26}; a sample (0x10) {..., u16 bank at +0xc, ...}; a link (8)
// {u32 the next variation (1-based), ...}. The categories come first, then each one's sounds, each sound's variations and each variation's samples
// and links, in the order the fixup walks them.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6bd7a0 (cdecl; a variation; name unconfirmed): its sample count, the low word of its first dword.
u32 __cdecl SfxVariation_SampleCount(const u8 *variation);

// 0x6c0550 (thiscall, `this` unused; a sound, pops 4; name unconfirmed): a sound flagged 4 and 2 with one variation of one sample, no links (its +0x26 0)
// and both words at +0x10 / +0x12 0 is flagged 8 instead of 4.
void __fastcall SfxMap_MarkSimpleSound(u8 *category, void *, u8 *sound);

// 0x6c06e0 (thiscall; data, variation, pops 8; name unconfirmed): the variation's samples at data (+8) and its links after them (+0x26); each sample's
// bank (1-based in the BANK.map, 0 none) becomes deque C's entry for it (its low word), and a bank the category's record (deque A at +0xc8) does not
// list yet is appended to the record's list (+0x24). Returns the end of the links.
u8 *__fastcall SfxMap_FixVariation(u8 *category, void *, u8 *data, u8 *variation);

// 0x6c0610 (thiscall; data, sound, pops 8; name unconfirmed): the sound's variations at data (+8), each fixed up from the end of the last; then,
// with +0xcc set, each variation's pick weight becomes its difference from the one before (the original values chained), and each link's next
// variation number becomes a pointer to that variation. Returns the end of the last variation's links.
u8 *__fastcall SfxMap_FixSound(u8 *category, void *, u8 *data, u8 *sound);

// 0x6c05b0 (thiscall; data, a category record of the image, pops 8; name unconfirmed): its sounds at data (+4), each fixed up from the end of the
// last, then SfxMap_MarkSimpleSound. Returns the end.
u8 *__fastcall SfxMap_FixCategory(u8 *category, void *, u8 *data, u8 *record);

// 0x6c0510 (thiscall; data, out, count, pops 0xc): *out = data, then each of the count category records there is fixed up, the first from the
// end of the records. Returns the end.
u8 *__fastcall SfxMap_Fixup(u8 *category, void *, u8 *data, u8 **out, u32 count);

}  // extern "C"
