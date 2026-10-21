// A sound mixer's voice (an entry of the mixer's voice pages, 0x50 bytes; soundmixer; the class name is invented): +0xc an info block,
// +0x24 its sound, +0x2c its id, +0x30 flags, +0x48 a pending word, +0x4c a parameter block (0x20 bytes: a byte, then dwords). The sound
// system [0xfb1f24] takes settings for a sound as stack functors (vtable, sound, data). Reimplementation of 0x6b7150, 0x6bc900 and the
// methods below (with the switch and sequence voices' 0x6be450, 0x6be5e0, 0x6be6d0, 0x6be660 and 0x6be730); the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b7150 (thiscall): with no parameter block at +0x4c, one of 0x20 bytes (Mem_Alloc; +0 the byte and +4-+0x18 cleared, +0x1c left)
// stored there, or 0 when the allocation fails.
void __fastcall SoundVoice_EnsureParams(u8 *voice);

// 0x6bc900 (thiscall (out, value), pops 8): with a sound at +0x24, a stack record (0x9c bytes: the fields the sound system reads
// cleared, +0x80 / +0x84 / +0x8c 0.25 / 0.5 / 1.0, SetThenFlag28_14 (value)) handed to the sound system's slot 4 (out, a stack
// functor: vtable 0x70a850, the sound, the record). Without one, the value's low word kept at +0x48, flag 0x8000 set and *out = -1.
// Returns out.
u32 *__fastcall SoundVoice_SetPending(u8 *voice, void *, u32 *out, u32 value);

// 0x6bcb80 (thiscall): unless flag 0x4000, the byte at +0x44 handed to the sound system in a record as 0x6bc900's: 100 as
// SetThenFlag28_14 (0), handed on even without a sound; else, with a sound, SetThenFlag9c_14 (the byte). Returns 0 when the sound
// system left the record's +0x18 bit 1 set and +0x9c nonzero (a sound still there); else flag 0x400 cleared, 0x100 set and returns 1.
u32 __fastcall SoundVoice_SendByte44(u8 *voice);

// The voice's sound definition (+4): +4 its variations' count, +8 the variations (0x2a bytes each), +0x10 flags. A variation (the
// current one at +8): +0 its samples' count (the low word: SfxVariation_SampleCount), +8 the samples (0x10 bytes each, +4 a rising
// threshold), +0x16 / +0x1a words for the parameter block, +0x1e its weight, +0x22 its radius (a float). +0x10 the voice's position
// (three floats), +0x1c a time, +0x20 the value handed to slot 8, +0x3c a start time, +0x40 a radius scale (a float), +0x45 / +0x46 the
// last variation / sample picked, +0x47 its categories (bits 0-3: the sound system's bytes +0x44-+0x47 enable them). The voice's own
// vtable: slot 7 (+0x1c) the squared distance (a float in st0), slot 8 (+0x20) a start. Names below are invented.

// 0x6bbf00 (thiscall): with the current variation's words +0x16 or +0x1a nonzero, SoundVoice_EnsureParams and the block's bytes 1 / 2
// those words' low bytes, byte 3 0; else, with a block, its bytes 1-3 0.
void __fastcall SoundVoice_ApplyVariation(u8 *voice);
// 0x6bc650 (thiscall; a float in st0): with flag 0x80, the sound system's +0x2c times +0x40 (at the FPU's precision); else the current
// variation's radius, or 0.0 (0x70a834) without one.
double __fastcall SoundVoice_Radius(const u8 *voice);
// 0x6bc680 (thiscall (avoid repeat, a byte), pops 4; vtable slot): the current variation's sample: none with no samples, the first
// with one; else the game's random number (0xfb1f20 = 0x19660d x + 0x3c6ef35f) rotated right 13, its high word, picks the first sample
// whose threshold is at least that (none past the last: 0). Avoiding a repeat over more than 2 samples, the same index as last time
// (+0x46, compared as a byte) becomes (index + 1, a signed byte) % count (unsigned). The index's low byte kept at +0x46. Returns it.
u8 *__fastcall SoundVoice_PickSample(u8 *voice, void *, u8 avoidRepeat);
// 0x6bc760 (thiscall): 1 (flag 0x4000 cleared) when every category bit (+0x47) has its sound system byte set; else 0: unless flag
// 0x4000 is set already, it is set when the definition's flags have 0xc, and the sound (+0x24) is stopped (the sound system's slot 4
// with the stack functor 0x70a2d0) and forgotten.
u8 __fastcall SoundVoice_CategoryEnabled(u8 *voice);
// 0x6bc810 (thiscall (avoid repeat), pops 4; vtable slot): 0 without a definition; with one variation, it (+8) and 1; else a random
// number as SoundVoice_PickSample's against the running sum of the weights: the first variation reaching it, or the first one when
// none does (then 1 at once). Avoiding a repeat as there (+0x45, the count a dword); then SoundVoice_ApplyVariation. Returns 1.
u8 __fastcall SoundVoice_PickVariation(u8 *voice, void *, u8 avoidRepeat);
// 0x6bca10 (thiscall): with flag 0x1000, SoundVoice_FollowListener first. The radius (stored as a float) squared at the FPU's
// precision against slot 7: below it (or unordered) and without flag 8, SoundVoice_Cull and 0; else flag 0x800 cleared, slot 8 with
// +0x20 and 1.
u8 __fastcall SoundVoice_UpdateAudible(u8 *voice);
// 0x6bca70 (thiscall): flag 0x800 set; playing (flag 1) with a sound, the sound stopped (as SoundVoice_CategoryEnabled; kept at +0x24).
void __fastcall SoundVoice_Cull(u8 *voice);
// 0x6bcab0 (thiscall (out, command), pops 8; vtable slot): with a sound, the sound system's slot 4 with the command and the out
// argument's own stack slot as its out (sic); then by the command's slot 0 (asked again each test): 1 flag 0x10; 2 with no start
// time (+0x3c), flag 0x2000 and the start time Time_GetMs; 3 with flag 0x2000, +0x1c += the time since then. *out = 0 (out as it is
// then). Returns out.
u32 *__fastcall SoundVoice_Command(u8 *voice, void *, u32 *out, u8 *command);
// 0x6bd5f0 (thiscall): the listener's position (the sound system's +0x60: three floats) with x kept within the parameter block's
// dwords +8 (below or unordered: it) and +0x10 (above: it), z within +0x14 (below or unordered) and +0xc (above: tested first), the
// bounds stored as floats, becomes +0x10 (Fn_6bb220). Playing (flag 1), a stack record (as SoundVoice_SetPending's: +0xc 0x20, or 0x820
// when the sound system's +0x28 has 0x43 and 0x80; +0x14 1, +0x30 the position) goes to the sound system's slot 4 with the sound.
void __fastcall SoundVoice_FollowListener(u8 *voice);

// 0x6bbe90 (thiscall (scale, a float's bits), pops 4): the flags (+0x30) from the definition's word +0x10: 6 with 8 (else 0), then 2
// with 4, 8 with 0x200, 0x200 with 0x20, 0x20000 with 0x4000; a scale neither 0.0 nor unordered kept at +0x40 with flag 0x80.
void __fastcall SoundVoice_SetFlags(u8 *voice, void *, u32 scale);
// 0x6bbf60 (thiscall (out, owner, definition, position, scale), pops 0x14; slot 4 of the voices' vtables): the owner (+0x28) kept and
// its slot 1 called, the definition (+4) and the position (+0x10, three dwords) kept, the categories (+0x47) the byte +8 of the owner's
// +8; the definition's word +0x12 nonzero makes the parameter block (SoundVoice_EnsureParams) and its byte 0 that word's low byte. Then
// SoundVoice_SetFlags (scale) and slot 14 (0): answering 0 with no current variation (+8), or with no sample (+0xc, else slot 15 (0)
// sets it) after SoundVoice_ApplyVariation, the owner's slot 0, +0x28 cleared and *out = -1; else +0x1c = Time_GetMs + the sample's +8
// + 250 and *out = 0. Returns out.
u32 *__fastcall SoundVoice_Init(u8 *voice, void *, u32 *out, u8 *owner, u8 *definition, const u32 *position, u32 scale);

// The switch voice (vtable 0x70a498) and the sequence voice (0x70a440), on the voice class 0x70a7e0 (Class70a7e0_Ctor, fieldops.h); their
// slot 14 picks the variation as SoundVoice_PickVariation does for the base. A switch voice's current node (+8): +4 a count, +0x26 that many
// 8-byte branches ({a variation (its weight +0x1e), -, a low byte at +6, a high byte at +7}).

// 0x6be450 (thiscall (value), pops 4): with no current node but a definition, its first variation becomes the node, and 1 when that has
// samples (SfxVariation_SampleCount). With a node of no branches it is cleared and 0. Else the branches whose bytes +6 <= value <= +7:
// none sets the parameter block's byte +4 to 0x7f, flag 0x40 and 0; else the game's random number (as SoundVoice_PickSample's: advanced,
// rotated right 13; here all of it) modulo the sum of their weights (a zero sum divides by zero), and the first of them whose running sum
// reaches it becomes the node (none: the node stays). Returns 1.
u8 __fastcall SoundVoice_PickBranch(u8 *voice, void *, u32 value);
// 0x6be5e0 (thiscall (unused), pops 4; slot 14 of 0x70a498): SoundVoice_PickBranch with the parameter block's byte +4; when that
// answers 1, SoundVoice_ApplyVariation and 1; else 0.
u8 __fastcall SoundVoice_SwitchVariation(u8 *voice, void *, u32 unused);
// 0x6be6d0 (thiscall (unused), pops 4; slot 14 of 0x70a440): 1 without a definition; with no current variation, the first (and 1); else
// the next one (+0x2a), and 1 while its index (a signed division) is below the count; past the end, with flag 4 the first again (1), else
// 0 (the variation left past the end).
u8 __fastcall SoundVoice_NextVariation(u8 *voice, void *, u32 unused);
// 0x6be660 / 0x6be730 (thiscall): Class70a7e0_Ctor, then the vtable 0x70a498 / 0x70a440. Return the voice.
u8 *__fastcall SwitchVoice_Ctor(u8 *voice);
u8 *__fastcall SequenceVoice_Ctor(u8 *voice);

}  // extern "C"
