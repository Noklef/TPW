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
// 0x6bb9f0 (thiscall; a float in st0; slot 7 of 0x70a7e0 and the voices'): +0x22 the definition's (+4) word +0xc. The squared distance
// on the ground (|x - the listener's x|, |z - z|, from +0x10 / +0x18 and the sound system's +0x60, at the FPU's precision, stored as a
// float), 0 without a listener. Not above 0 (or unordered): +0x20 0xffff; else that times 0.01 (stored as a float, fistp): up to
// 0xffff (unsigned), +0x20 0xffff minus it, else +0x20 kept. Returns the squared distance (name invented).
double __fastcall SoundVoice_Distance(u8 *voice);
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

// 0x6bbb80 (thiscall (key, value: bytes), pops 8; slot 10 of 0x70a7e0 / 0x70a440): with no parameter block one made (EnsureParams),
// then each of its bytes 0-3 equal to the key gets the value at +4 on; when one of bytes 1-3 did (byte 0 not counted, sic) and flag
// 0x4000 is clear, slot 13.
void __fastcall SoundVoice_SetParam(u8 *voice, void *, u32 key, u32 value);
// 0x6be580 (thiscall (key, value), pops 8; slot 10 of 0x70a498): SoundVoice_SetParam; then with flag 0x40 and the block's byte +4 not
// 0x7f, slot 14 (1) answering nonzero with a current variation: SoundVoice_PickSample (1) becomes +0xc, and a sample clears flag 0x40.
void __fastcall SwitchVoice_SetParam(u8 *voice, void *, u32 key, u32 value);
// 0x6be610 / 0x6be680 (cdecl (owner, memory, definition, info)): with memory, SwitchVoice_Ctor / SequenceVoice_Ctor there, then slot 4
// (out: the memory argument's own stack slot, sic; the owner, the definition, info + 0xc as the position, info's +0x18 as the scale);
// the voice, or 0 without memory or when *out is negative (the voice left as it is).
u8 *__cdecl SwitchVoice_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);
u8 *__cdecl SequenceVoice_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);
// 0x6bdc60 / 0x6bdd30 / 0x6bd7b0 / 0x6be330 (names invented): the same with Voice70a390_Ctor / Voice70a2e0_Ctor / Voice70a3e8_Ctor /
// Voice70a878_Ctor.
u8 *__cdecl Voice70a390_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);
u8 *__cdecl Voice70a2e0_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);
u8 *__cdecl Voice70a3e8_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);
u8 *__cdecl Voice70a878_Create(u8 *owner, u8 *memory, u8 *definition, u8 *info);

// The linked voice (vtable 0x70a970) and the linked switch voice (0x70a9e0, on it): a voice whose parameter block's +0x1c is the voice it
// follows (the parent: its position +0x10, categories +0x47, current node +8 and parameter block's byte +4); a parent's block +0x18 heads
// the chain of its linked voices (each the next at its own block's +0x18).

// 0x6c3a00 (thiscall (parent), pops 4): Class70a7e0_Ctor, the vtable 0x70a970, a parameter block made as SoundVoice_EnsureParams does
// (the heap's null not checked after, sic) and its +0x1c = the parent. Returns the voice.
u8 *__fastcall LinkedVoice_Ctor(u8 *voice, void *, u8 *parent);
// 0x6c3de0 (thiscall (parent), pops 4): LinkedVoice_Ctor, then the vtable 0x70a9e0. Returns the voice.
u8 *__fastcall LinkedSwitchVoice_Ctor(u8 *voice, void *, u8 *parent);
// 0x6be260 (thiscall; name invented): Class70a7e0_Ctor, the vtable 0x70a878, the parameter block (SoundVoice_EnsureParams) and its +0x1c
// 0 (no parent; a failed allocation faults there, sic). Returns the voice.
u8 *__fastcall Voice70a878_Ctor(u8 *voice);
// 0x6bdd80 (thiscall; name invented): Class70a7e0_Ctor, the vtable 0x70a390, the parameter block (SoundVoice_EnsureParams), then the
// vtable 0x70a2e0. Returns the voice.
u8 *__fastcall Voice70a2e0_Ctor(u8 *voice);
// 0x6bdcb0 (thiscall; an SEH frame; name invented): Class70a7e0_Ctor, the vtable 0x70a390 and the parameter block (SoundVoice_EnsureParams,
// inlined). Returns the voice.
u8 *__fastcall Voice70a390_Ctor(u8 *voice);
// 0x6c3a80 (thiscall (definition, time), pops 8): +4 = the definition, SoundVoice_SetFlags (0), the categories (+0x47) the parent's;
// slot 14 (0) answering 0, or slot 15 (0) (it becomes +0xc) 0: 0. Else +0x1c = slot 19 () + the time; 1.
u8 __fastcall LinkedVoice_Init(u8 *voice, void *, u8 *definition, u32 time);
// 0x6c3c70 (thiscall): with a sound (+0x24), a stack record (as SoundVoice_FollowListener's, +0xc and +0x14 cleared too: +0xc 0x20, or
// 0x820 when the sound system's +0x28 has 0x43 and 0x80, both only without flag 8 (+0x30); +0x14 1, +0x30 the parent's position +0x10)
// goes to the sound system's slot 4 with the sound.
void __fastcall LinkedVoice_FollowParent(u8 *voice);
// 0x6c3e00 (thiscall (unused), pops 4; slot 14 of 0x70a9e0): SoundVoice_PickBranch on the parent's node (+8, copied here first) with the
// parent's block's byte +4, each node chosen stored at both voices' +8: with none but a definition, its first variation, 1 when that has
// samples; a node of no branches cleared (both) and 0; no branch in range: the parent's block's byte +4 = 0x7f, its flag 0x40 and 0;
// else the random pick (all of the rotated number) and 1, or 0 when no running sum reaches it.
u8 __fastcall LinkedSwitchVoice_PickBranch(u8 *voice, void *, u32 unused);
// 0x6bc090 (thiscall): 100 without a current variation (+8). Its bytes +0xc / +0xd (unsigned) put in order (an XOR swap through
// the variation, +8 re-read at each step), the range their difference; with a parameter block (+0x4c) and slot 16 (1, a local) answering
// AL nonzero: the local times the range (32 bits) / 100 + the low byte. Else the game's random number (advanced, all of it rotated right
// 13) modulo the range (none drawn for an empty range) + the low byte.
u32 __fastcall SoundVoice_RollVolume(u8 *voice);
// 0x6bc170 (thiscall): as SoundVoice_RollVolume, on the signed bytes +0xe / +0xf, slot 16 with 2; 0 without a current variation.
u32 __fastcall SoundVoice_RollPitch(u8 *voice);
// 0x6bc240 (thiscall (record), pops 4; slot 18 of the voices' vtables; name invented): SoundChannel_LoadBank of the sound system
// [0xfb1f24] with the sample's (+0xc) word +0xc; null: 0. Else the record's Obj_SetFirstFlag80 (the sample's dword +0, re-read) and
// Obj_SetSecondFlag8 (the resource); with flag 0x200 (+0x30) SetThenFlag8_14_6c50b0 (the voice's +4) and the record's +0xc |= 0x200;
// then +0xc |= 4 with flag 0x20000, else 0xc0. Returns 1.
u32 __fastcall SoundVoice_SetRecordBank(u8 *voice, void *, u8 *record);
// 0x6bdf00 (thiscall (record), pops 4; slot 18 of 0x70a338, 0x70a440, 0x70a498, 0x70a878 and 0x70a8d0; name invented):
// SoundVoice_SetRecordBank; 0 when it answers 0. Else the record's SetThenFlag4c_14_6c50f0 (the voice) and, when either word +0x10 /
// +0x12 of the voice's +8 is nonzero, its FlagThenSet14_58_6c5220 (the voice's slot 19). Returns 1.
u32 __fastcall SoundVoice_SetRecordBankLinked(u8 *voice, void *, u8 *record);
// 0x6bc2d0 (thiscall; slot 19): the current variation's (+8, not tested for null) words +0x10 / +0x12 both 0: 0. With a parameter block
// (+0x4c) and slot 16 (4) answering (AL): +0x10 + (+0x12 - +0x10) * the percentage / 100 (signed). Else the words put in order (swapped
// in place) and +0x10 + a random draw (the full rotated number) modulo their difference, or +0x10 when they are equal.
u32 __fastcall SoundVoice_RollWord(u8 *voice);
// 0x6bba90 (thiscall (value), pops 4; slot 8): +0x20 = value; with flag 1 (+0x30) the record (as 0x6bc900's) with +0x2c = value
// (SetThenFlag2c_14_6c5150) to the sound system, without testing the sound (+0x24) or the system for null.
void __fastcall SoundVoice_SetValue20(u8 *voice, void *, u32 value);
// 0x6bbbe0 (thiscall; slot 13, SoundVoice_SetParam's): with a sound (+0x24) and the sound system, the record with the volume
// (SetThenFlag28_14_6c5130: SoundVoice_RollVolume) unless flag 0x400 (+0x30), and the pitch (SetThenFlag24_14_6c5170:
// SoundVoice_RollPitch), to the system.
void __fastcall SoundVoice_SendLevels(u8 *voice);
// 0x6bbce0 (thiscall (out, position), pops 8; slot 9): the three dwords to +0x10; with a sound (+0x24), a record to the system: with
// flag 8 (+0x30) +0x10 set to the ints (0, 1, 0) (Fn_6bb220, sic); else +0x10 scaled by the system's float +0x2c (Vec3_Scale), the
// record's +0xc |= 0x20, and 0x800 when the system's +0x28 has 0x43 and 0x80. Then +0x14 |= 1, +0x30 = the voice + 0x10. *out = 0;
// returns out.
u32 *__fastcall SoundVoice_SetPosition(u8 *voice, void *, u32 *out, const u32 *position);
// 0x6bc410 (thiscall (now, record); pops 8; slot 17 of the voices' vtables): 0 without a sample (+0xc), a variation (+8), the sound system
// or the voice's slot 18 (record). Else the record's volume (SetThenFlag28_14_6c5130: the word +0x48 with flag 0x8000 (+0x30), else
// SoundVoice_RollVolume), +0x94 / +0x98 (the system + 0x58, the byte +0x47), pitch (SetThenFlag24_14_6c5170: the signed word +0x4a with
// flag 0x10000, else SoundVoice_RollPitch), +0x2c (+0x20), +0x50 = the voice and +0x14 |= 0x2000, +0x48 (SetThenFlag48_14_6c5090: +0x2c).
// A sample of length 0 gives way to the variation's first (its +8); +0x3c = slot 19; 0 when Obj_PastEnd (now). Then the level
// (SoundVoice_SetRecordLevel). With flag 8: +0x10 = the ints (0, 1, 0) (Fn_6bb220, sic), the record's +0xc |= 1. Else, with now -
// (+0x1c - length - 250) modulo the length (DIV: a zero length faults) past 1000, flag 2 clear, a length past 1000 and under 1000 left,
// 0; otherwise +0x38 = 2.0f, +0x3c the radius (2.1f unless above 2.0), +0x40 the system's slot 11 as a float, +0x14 |= 0x200, +0x10
// scaled by the system's +0x2c (Vec3_Scale), +0xc |= 0x20 (0x1000 with bit 3 of the system's +0x28, 0x800 when that has 0x43 and 0x80).
// Then +0x30 = the voice + 0x10, +0x14 |= 1, and with flag 4 SetThenFlag44_14_6c5110 (9999). Returns 1.
u32 __fastcall SoundVoice_FillRecord(u8 *voice, void *, u32 now, u8 *record);
// 0x6c39c0 (thiscall (now, record), pops 8; slot 17 of 0x70a4f0; name invented): SoundVoice_FillRecord, and when that answers 1 the
// record's +0x50 (the voice) = 0 and its flag 0x2000 (+0x14) cleared again; returns 1. Else 0.
u32 __fastcall SoundVoice_FillRecordUnowned(u8 *voice, void *, u32 now, u8 *record);
// 0x6bc3c0 (thiscall (record), pops 4): with flag 8 (+0x30), FlagThenSet14_5c_6c51d0 on the record with 0; else, when
// Obj_Flags28_Bit1IfBit0 of the sound system answers nonzero, the same with the system's slot 10 (the voice + 0x10) as a float, else
// that answer (0). Returns what the last call answered.
u32 __fastcall SoundVoice_SetRecordLevel(u8 *voice, void *, u8 *record);
// 0x6c3ae0 (thiscall (unused, record), pops 8; slot 17 of 0x70a970 / 0x70a9e0): 0 without +0xc, a current variation (+8), the sound
// system or the voice's slot 18 (record). Else the record's volume (SetThenFlag28_14_6c5130: SoundVoice_RollVolume), +0x94 / +0x98
// (Obj_Set94_98_Flag40000: the system + 0x58, the byte +0x47), pitch (SetThenFlag24_14_6c5170: SoundVoice_RollPitch), +0x2c
// (SetThenFlag2c_14_6c5150: the parent's (+0x4c, +0x1c) +0x20) and level (SoundVoice_SetRecordLevel). With flag 8 (+0x30): the record's
// +0x30 = the address of an unset local of this frame (sic), +0x14 and +0xc |= 1. Else +0x38 = 2.0f, +0x3c the radius (SoundVoice_Radius,
// 2.1f unless above 2.0), +0x40 the system's slot 11 as a float, +0x14 |= 0x201, +0x30 the parent's position (+0x10), +0xc |= 0x20, and
// 0x1000 with bit 3 of the system's +0x28, 0x800 when that has 0x43 and 0x80. Then with flag 4 (+0x30), SetThenFlag44_14_6c5110 (9999).
// Returns 1.
u32 __fastcall LinkedVoice_FillRecord(u8 *voice, void *, u32 unused, u8 *record);

// 0x6bd800 (thiscall (out, now), pops 8; slot 5 of 0x70a3e8, 0x70a4f0, 0x70a970 and 0x70a9e0; name invented): the voice started. Without
// the sound system or a variation (+8): *out = -1. With flag 0x1 or 0x10 (+0x30): *out = 0. Else flag 1 set, a stack record (as
// SoundVoice_SetPending's) filled by the voice's slot 17 (now, record): 0 sets flag 0x10 and *out = -1; else SoundChannel_PlayRecord (the
// system, the record, the voice's +0x24), flag 1 cleared again when it answers 0. *out = 0. Returns out.
u32 *__fastcall SoundVoice_Start(u8 *voice, void *, u32 *out, u32 now);
// 0x6bdf50 (slot 5 of 0x70a338, 0x70a440, 0x70a498, 0x70a878 and 0x70a8d0; name invented): SoundVoice_Start with, after the fill and with
// flag 4, the record's +0x44 = 1 (SetThenFlag44_14_6c5110).
u32 *__fastcall SoundVoice_StartCounted(u8 *voice, void *, u32 *out, u32 now);
// 0x6bde00 (thiscall (a, unused), pops 8; slot 0 of 0x70a338, 0x70a440, 0x70a498, 0x70a878 and 0x70a8d0; name invented): the voice's
// slot 14 (1) answering 0 in its low byte: that whole answer (sic). Without the sound system 0; without a variation (+8) the system's
// address (sic). Else +0xc = slot 15 (1) (0: 0), a stack record (as SoundVoice_Start's) through slot 18 (0: 0), then
// SoundChannel_PickVoice (the system, re-read; a, a dropped local, the record).
u32 __fastcall SoundVoice_PickChannelVoice(u8 *voice, void *, u32 a, u32 unused);

// 0x6be380 (thiscall (list), pops 4; name invented): the parameter block's (+0x4c) shuffle bag (+0x1c): none, a new one (Mem_Alloc 0x10,
// ShuffleBag_Ctor over the list; null when the new fails); else ShuffleBag_SetList when its list (+0) is another.
void __fastcall SoundVoice_EnsureShuffleBag(u8 *voice, void *, u8 *list);
// 0x6be420 (thiscall (unused), pops 4; slot 14 of 0x70a878; name invented): SoundVoice_EnsureShuffleBag with the definition (+4), the
// current variation (+8) = ShuffleBag_Next (refill: flag 4 of +0x30); 1 when there is one (a byte: SETNZ over the pointer in EAX).
u8 __fastcall SoundVoice_ShuffleVariation(u8 *voice, void *, u32 unused);
// 0x6b71d0 / 0x6b7210 (thiscall; names invented): Class70a7e0_Ctor, then the vtable 0x70a3e8 / 0x70a4f0. Return the voice.
u8 *__fastcall Voice70a3e8_Ctor(u8 *voice);
u8 *__fastcall Voice70a4f0_Ctor(u8 *voice);
// 0x6b7040 (thiscall (count), pops 4; an SEH frame; name invented): new[] of count voices 0x70a4f0: a block of count * 0x50 + 4 (Mem_Alloc),
// the count at +0, the voices from +4 constructed (CRT_VectorCtorIterator with Voice70a4f0_Ctor; 0x6b7230, the 0x70a3e8 voice's
// destructor, for the unwinding); the array (null when the allocation fails) into the holder's +0. Returns the holder.
u8 *__fastcall Voices70a4f0_New(u8 *holder, void *, u32 count);
// 0x6b7250 / 0x6b72f0 (thiscall; slot 21 of the vtables 0x70a390 / 0x70a2e0; an SEH frame; names invented): a new linked voice / linked
// switch voice (Mem_Alloc 0x50, LinkedVoice_Ctor / LinkedSwitchVoice_Ctor) whose parent is this voice; null when the allocation fails.
u8 *__fastcall Voice70a390_NewLinked(u8 *voice);
u8 *__fastcall Voice70a2e0_NewLinkedSwitch(u8 *voice);
// 0x6bb9b0 (thiscall; the voice class 0x70a7e0's destructor; name invented): the vtable 0x70a7e0, +0x2c's high word cleared, the parameter
// block (+0x4c) freed (Mem_Free) unless null, then the owner's (+0x28, unless null) slot 0.
void __fastcall Class70a7e0_Dtor(u8 *voice);
// 0x6bb990 (thiscall (flags), pops 4; slot 2 of 0x70a7e0; name invented): Class70a7e0_Dtor, the voice freed with flag 1. Returns the voice.
u8 *__fastcall Class70a7e0_DeletingDtor(u8 *voice, void *, u8 flags);
// 0x6b7230 / 0x6b7240 (thiscall; names invented): the vtable 0x70a3e8 / 0x70a338, then Class70a7e0_Dtor.
void __fastcall Voice70a3e8_Dtor(u8 *voice);
void __fastcall Voice70a338_Dtor(u8 *voice);
// 0x6b71f0 / 0x6b72d0 (thiscall (flags), pops 4; slot 2 of 0x70a3e8 / 0x70a338; names invented): the destructor, the voice freed with
// flag 1. Return the voice.
u8 *__fastcall Voice70a3e8_DeletingDtor(u8 *voice, void *, u8 flags);
u8 *__fastcall Voice70a338_DeletingDtor(u8 *voice, void *, u8 flags);
// 0x6b70e0 (thiscall (flags), pops 4; slot 2 of 0x70a4f0; name invented): flag 2, the array's (count at -4) voices destroyed, the last first
// (CRT_VectorDtorIterator with Voice70a3e8_Dtor), and its block freed (sic: flag 1 not tested); else Voice70a3e8_Dtor and the voice
// freed with flag 1. Returns the voice.
u8 *__fastcall Voice70a4f0_VectorDeletingDtor(u8 *voice, void *, u8 flags);
// 0x6be300 (thiscall; name invented): the vtable 0x70a878, the parameter block's (+0x4c, not checked) +0x1c freed unless null, the
// vtable 0x70a338, Class70a7e0_Dtor. 0x6be2e0 (slot 2 of 0x70a878, (flags), pops 4): it, the voice freed with flag 1; returns the voice.
void __fastcall Voice70a878_Dtor(u8 *voice);
u8 *__fastcall Voice70a878_DeletingDtor(u8 *voice, void *, u8 flags);
// 0x6be9c0 (thiscall; name invented): the vtable 0x70a8d0, the parameter block's (+0x4c, not checked) +0x1c, unless null, freed after its
// +8; the vtable 0x70a338, Class70a7e0_Dtor. 0x6be9a0 (slot 2 of 0x70a8d0, (flags), pops 4): it, the voice freed with flag 1; returns the voice.
void __fastcall Voice70a8d0_Dtor(u8 *voice);
u8 *__fastcall Voice70a8d0_DeletingDtor(u8 *voice, void *, u8 flags);
// 0x6be960 (thiscall (count), pops 4; the 0x70a8d0 voice constructor's (0x6be7a0) parameter pairs; name invented): *self = a block of
// count 8-byte pairs from the game heap (null when it fails). Returns self.
u32 *__fastcall VoicePairs_Alloc(u32 *self, void *, u32 count);
// 0x6be880 (thiscall (unused), pops 4; slot 14 of 0x70a8d0; name invented): flag 2 (+0x30) set; the parameter block's (+0x4c) +0x1c a
// queue (+0 its count, +4 the next index, +8 its 8-byte entries {a category index, a sound}): the index taken and moved on (even past the
// count); none left: 0. Else the category singleton's deque A (its begin +0x3c, DequeIter_Advance) at the category index: a record, its +8
// an SFX.map sound table; either null: 0. Else SfxSounds_Find with a stack play command (vtable 0x700b90: the category, the sound, the
// voice's position +0x10-+0x18, range 0; its `this` an unset local, unused): none, 0; else it is the definition (+4) and
// SoundVoice_PickVariation (0)'s answer. AL only on the failures (the rest of EAX left over).
u8 __fastcall Voice70a8d0_PickQueuedSound(u8 *voice, void *, u32 unused);
// The group voice 0x70a390's (names invented): its children (linked voices) listed at the parameter block's +0x18, each the next's in
// its own block's +0x18. 0x6bd930 (thiscall; an SEH frame): the vtable 0x70a390, each child deleted (slot 2, flag 1; the next read
// first), the vtable 0x70a3e8, Class70a7e0_Dtor. 0x6b72b0 (thiscall (flags), pops 4; slot 2): it, the voice freed with flag 1.
void __fastcall Voice70a390_Dtor(u8 *voice);
u8 *__fastcall Voice70a390_DeletingDtor(u8 *voice, void *, u32 flags);
// 0x6bd9b0 (thiscall (out, command), pops 8; slot 6): each child: the command's +4 its sound (+0x24), its slot 6 (out: the command's own
// argument slot), deleted (slot 2, 1) when the command's slot 0 answers 1; the list then emptied (the kept children too, sic), and
// SoundVoice_Command (out, command). Returns the out.
u32 *__fastcall Voice70a390_Command(u8 *voice, void *, u32 *out, u8 *command);
// 0x6bda20 (thiscall (out, position), pops 8; slot 9): the position into +0x10, flag 8: Fn_6bb220 (0, 1, 0) on it, else Vec3_Scale by
// the sound system's +0x2c; LinkedVoice_FollowParent on each child; *out = 0. Returns the out.
u32 *__fastcall Voice70a390_SetPosition(u8 *voice, void *, u32 *out, const u32 *position);
// 0x6bda90 (thiscall (key, value), pops 8; slot 10): SoundVoice_SetParam, then flag 0x40 cleared unless the block's byte +4 is 0x7f.
void __fastcall Voice70a390_SetParam(u8 *voice, void *, u32 key, u32 value);
// 0x6bdac0 (thiscall; slot 13): each child's slot 13.
void __fastcall Voice70a390_SendLevels(u8 *voice);
// 0x6bdbb0 (thiscall (prev, time), pops 8): a new child (the voice's slot 21) after prev (its block's +0x18), LinkedVoice_Init
// (the voice's definition +4, time): 1, SoundVoice_ApplyVariation and 1; else the child deleted (slot 2, 1) unless null, unlinked, 0.
u8 __fastcall Voice70a390_AddChild(u8 *voice, void *, u8 *prev, u32 time);
// 0x6bdb10 (thiscall (time), pops 4): without children: the block's byte 0 set and flag 0x20 clear: +8 = 0 and flag 0x20; then
// Voice70a390_AddChild (the voice, time). Returns the first child.
u8 *__fastcall Voice70a390_FirstChild(u8 *voice, void *, u32 time);
// 0x6bdb50 (thiscall (time), pops 4): unless flag 0x10, from Voice70a390_FirstChild on: each child's slot 5 (out: the time's own
// argument slot, sic; time), and past its end (+0x1c, unsigned) the last one gets a next (Voice70a390_AddChild).
void __fastcall Voice70a390_StartChildren(u8 *voice, void *, u32 time);
// 0x6bdc10 (thiscall (time), pops 4): the children whose end (+0x1c) the time has reached (unsigned) unlinked and deleted (slot 2, 1).
void __fastcall Voice70a390_PruneChildren(u8 *voice, void *, u32 time);
// 0x6bdae0 (thiscall (out, time), pops 8; slot 5): unless the byte +0x30 has 0x50, Voice70a390_StartChildren and _PruneChildren;
// *out = 0. Returns the out.
u32 *__fastcall Voice70a390_Start(u8 *voice, void *, u32 *out, u32 time);

}  // extern "C"
