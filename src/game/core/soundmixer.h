// The sound mixer (vtable 0x70a2cc; the class name is invented): +4 a critical section, +0x1c a list of voice pages (next at +8),
// +0x20 / +0x24 the chains of free / active voices (doubly linked: next at +0x34, previous at +0x38; Chain20_* / Chain24_* in
// fieldops), +0x30 a stamp counter (Obj_Inc30), +0x34 a byte its worker sets when it has stopped, +0x38 the stop request, +0x3c an
// object and +0x40 / +0x44 blocks it owns. A voice: +0x24 its sound, +0x2c its id (the low 16 bits the slot Pages100_EntryAt finds,
// the high 16 a stamp), +0x30 flags. Reimplementation of 0x6b6120, 0x6b6170, 0x6b6260, 0x6b6440, 0x6b6550, 0x6b6830, 0x6b6990,
// 0x6b6ab0 and 0x6b6ba0; the names are unconfirmed.
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

}  // extern "C"
