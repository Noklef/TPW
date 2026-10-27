// The sound mixer (vtable 0x70a2cc; the class name is invented): +4 a critical section, +0x1c a list of voice pages (next at +8),
// +0x20 / +0x24 the chains of free / active voices (doubly linked: next at +0x34, previous at +0x38; Chain20_* / Chain24_* in
// fieldops), +0x30 a stamp counter (Obj_Inc30), +0x34 a byte its worker sets when it has stopped, +0x38 the stop request, +0x3c an
// object and +0x40 / +0x44 blocks it owns. A voice: +0x24 its sound, +0x2c its id (the low 16 bits the slot Pages100_EntryAt finds,
// the high 16 a stamp), +0x30 flags. Reimplementation of 0x6b6120, 0x6b6170, 0x6b6260, 0x6b6440, 0x6b6550, 0x6b6830, 0x6b6990,
// 0x6b6ab0, 0x6b6ba0, 0x6b62b0 and 0x6b69b0, and of the voice commands 0x6b64b0, 0x6b6510 and 0x6b65a0 (out: 0, or -1 when no voice has the id); the
// names are unconfirmed. 0x6b6650 adds a voice to the array at +0x3c (0x50 bytes each, its slots' pointers at +0x40, their count +0x4c).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b6170 (thiscall): the destructor: the vtable reset to 0x70a2cc, +0x35 = 1, the stop request (+0x38) set (InterlockedExchange)
// and a spin until the worker's byte +0x34 is set; every active voice's slot 3; the page list freed node by node (Mem_Free); the
// blocks at +0x44 and +0x40 freed; the object at +0x3c (when there is one) destroyed through its slot 2 (3); the critical section
// deleted.
void __fastcall SoundMixer_Dtor(u8 *mixer);

// 0x6b6120 (thiscall (flags), pops 4): SoundMixer_Dtor, then the mixer freed for flags bit 0. Returns the mixer.
u8 *__fastcall SoundMixer_DeletingDtor(u8 *mixer, void *, u32 flags);

// 0x6b6260 (thiscall): every active voice with flag 0x400 counted (InterlockedIncrement on a local) and its slot 0xb called. Returns
// the count.
u32 __fastcall SoundMixer_TouchFlagged(u8 *mixer);

// 0x6b6440 (thiscall (voice), pops 4): under the lock, the voice's slot 3 and the voice moved from the active chain to the free one
// (Chain24_MoveTo20).
void __fastcall SoundMixer_Retire(u8 *mixer, void *, u8 *voice);

// 0x6b6550 (thiscall (arg), pops 4): for every active voice: arg not 0: the voice's slot 0xc (arg); else its slot 6 (out: the
// argument's own stack slot, a stack functor: vtable 0x70a2d0 and the voice's +0x24).
void __fastcall SoundMixer_ForEachActive(u8 *mixer, void *, u32 arg);

// 0x6b6830 (thiscall (id), pops 4): 0 for id 0; else under the lock the voice in the id's slot (Pages100_EntryAt) when its id at
// +0x2c is the id, else 0.
u8 *__fastcall SoundMixer_FindVoice(u8 *mixer, void *, u32 id);

// 0x6b6990 (thiscall (voice), pops 4): the voice's id (+0x2c) gets the next stamp (Obj_Inc30 on the mixer) added in its high 16
// bits. Returns the new id.
u32 __fastcall SoundMixer_StampVoice(u8 *mixer, void *, u8 *voice);

// 0x6b6ab0 (cdecl (status, out, count)): with out not null, a voice page block of count * 0x50 + 0xc bytes (Mem_Alloc; the size
// wraps): +0 vtable 0x70a2d8, +4 count * 0x50, +8 0, stored in *out and *status = 0; *status = -1 with no out or no block.
// Returns status.
s32 *__cdecl VoicePage_New(s32 *status, u8 **out, u32 count);

// 0x6b6ba0 (thiscall): the 0x7c bytes at the block +0x44 points to cleared and +0x48 = 0. Returns 0.
u32 __fastcall SoundMixer_ClearLevels(u8 *mixer);

// 0x6b62b0 (thiscall; the worker's step): SoundMixer_ClearLevels, the time (Time_GetMs), then for each active voice (its next read
// first): with flag 0x10, or past its end (Obj_PastEnd), retired (SoundMixer_Retire); else with flag 0x100 its slot 6 (out, the
// stack functor 0x70a2d0 and its sound); else unless flag 0x40, when its categories are enabled (SoundVoice_CategoryEnabled): slot
// 0xb when the mixer's byte +0x50 and its flag 0x400 are set, then, when audible (SoundVoice_UpdateAudible), into the loudest ones
// (Top_Insert, +0x44 / +0x4c). Then each of those (the array at +0x44 up to a null) gets slot 5 (out, the time).
void __fastcall SoundMixer_Update(u8 *mixer);

// 0x6b64b0 (thiscall (out, id, params), pops 0xc): the voice for the id (SoundMixer_FindVoice) gets a parameter block
// (SoundVoice_EnsureParams; none when the allocation failed: a fault, sic), the 16 bytes at params copied to its +8, and flag 0x1000.
// Returns out.
u32 *__fastcall SoundMixer_SetVoiceParams(u8 *mixer, void *, u32 *out, u32 id, const u32 *params);

// 0x6b6510 (thiscall (out, id, value), pops 0xc): SoundVoice_SetPending (out, value) on the voice for the id. Returns out.
u32 *__fastcall SoundMixer_SetVoicePending(u8 *mixer, void *, u32 *out, u32 id, u32 value);

// 0x6b65a0 (thiscall (out, id, value), pops 0xc): the voice for the id's slot 0xc (value); *out = 0. Returns out.
u32 *__fastcall SoundMixer_SendVoice(u8 *mixer, void *, u32 *out, u32 id, u32 value);

// 0x6b69b0 (thiscall (out), pops 4; name invented): a new page of 100 voices (VoicePage_New); *out = -1 when that fails. Else the
// page appended to the list at +0x1c (after its last page, through +8), each voice given the next id (+0x2c, the mixer's counter
// +0x2c then raised), its flags +0x30 0 and pushed on the free chain (Chain20_PushFront), the pages' count +0x28 raised and *out = 0.
// Returns out.
u32 *__fastcall SoundMixer_AddPage(u8 *mixer, void *, u32 *out);
// 0x6b6650 (thiscall (owner, definition, info), pops 0xc; name invented): the first slot (+0x40, count +0x4c) that is 0; none: 0. Else
// the array's voice there (+0x3c + slot * 0x50) constructed (Voice70a4f0_Ctor) and its slot 4 (SoundVoice_Init: out the info argument's
// own slot (sic), owner, definition, info + 0xc, info's +0x18): negative, 0 (the voice left constructed); else the slot = the voice, 1.
u32 __fastcall SoundMixer_AddVoice(u8 *mixer, void *, u8 *owner, u8 *definition, u8 *info);

}  // extern "C"
