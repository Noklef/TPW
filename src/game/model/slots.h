// A table of model slots (0xb0 bytes each at 0x790398; +4 a holder that is also its
// animator), used by the 0x54e000 and 0x598000 front-end code: reimplementations of
// 0x429a60, 0x429ba0, 0x429d60, 0x429df0, 0x429e90, 0x429ee0, 0x429f60 and 0x429fe0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x429df0: Animator_Update(holder, 1, 0) when the slot has a holder.
void __cdecl ModelSlot_Animate(u32 slot);

// 0x429d60 ("Kill advisor"): when the slot's animator has a holder (+8) that is awake (Holder_IsAwake), bank 0xc's entry 0 is played (Animator_Play, mode 2,
// speed 1, layer 0) and the holder leaves the renderer: by the animator's rectangle (+0xc0 / +0xc4, Obj_RemoveFromRect) when it has flag
// 0x4000, else Holder_Sleep.
void __cdecl ModelSlot_KillAdvisor(u32 slot);

// 0x429e90 ("Initial advisor call"; cdecl; entry, slot): Animator_Play(holder, 5, entry - 1, 0x10, 1.0f, 0) when the slot has a holder, else 1000.
u32 __cdecl ModelSlot_CallAdvisor(u32 entry, u32 slot);

// 0x429ee0 / 0x429f60: clear / set flag 0x10 of the face of the slot's first part with key
// (+4) and flag 0x400 (Model_FindPartByKey4), when there is one.
void __cdecl ModelSlot_ClearPartFlag(u32 key, u32 slot);
void __cdecl ModelSlot_SetPartFlag(u32 key, u32 slot);

// 0x429fe0: the slot's holder.
u8 *__cdecl ModelSlot_Holder(u32 slot);

// 0x429e20: frees the slot's model, when it has one (+4): its part at +0x48 (through the
// virtual base offset at +4 of its vbtable) leaves the renderer's tag 0x10 list
// (Renderer_RemoveFromSlot), the model is disposed of (Instance_Dispose) and the instance at
// +0 freed (Instance_Free, not kept); both pointers are then cleared.
void __cdecl ModelSlot_Free(u32 slot);

// 0x429a60 (thiscall; the table's element constructor, CRT_VectorCtorIterator over the 4 slots; the SEH frame left out): only the part at +0x48 is built, a class with
// virtual bases: its vbtables 0x6fdd00 (+0) and 0x6fdcf8 (+0xc) and vtable 0x6fdce8 (+8) are set, then through the offsets those vbtables hold (re-read for each store)
// the bases get vtables 0x6fdcd8 and 0x6fdcc8 with their displacement dwords before them, +0x5c = 0 and the base's +0x50 = 2; then twice the scene flags are touched
// (SceneFlags_Touch of 0x8bd508) and the base gets +0x2c, +0x30 = 0 and +0x34 = 1.0f, then +4 .. +0x10 = 1.0f and +0x54 = -1. Returns the slot.
u8 *__fastcall ModelSlot_Ctor(u8 *slot);

// 0x429ba0 (cdecl (folder, c, suffix, slot); name unconfirmed): the advisor's model loaded into the slot: its matrix (+8) the identity, +0x38
// .. +0x40 = 0.6f, -0.6f, 0.2f; the part's (+0x48) base (through its vbtable's +8, read again for each) gets +0x54 = -1 and +4 .. +0x10 =
// [0x6fdcb0] (1.0f) after the scene flags are touched (SceneFlags_Touch of 0x8bd508), then +0x2c / +0x30 / +0x34 = 26.666666f,
// -26.666666f, 800.0f after a second touch. The name "Advisor%s.md2" with the suffix, or "Advisor.md2" without (sprintf), the ride's
// quickload indexes and texture directories for it (Quickload_LoadRide(name, folder, c)), the model loaded (RideModel_Load(name,
// 0x50120, the slot's +0, no name out, 1 layer)), the instance cloned into +4 (Instance_Clone(+0, 0, 0x2117, +4, 0)) and reset
// (Animator_Reset(1, 1, 1)) and given flag 4 (+4: read through +4 unchecked); then with the instance there: Holder_FlagModel on its
// holder (+8), its +0xc8 the slot's matrix, the part's other base (the vbtable's +4) added to the renderer [0x87a6d0] with tag 0x10
// (Renderer_Add) and the holder's model's root (+4, +0x78) matrix (+0x10) scaled by 0.01125f, 0.001f, 0.015f (Matrix_ScaleAxesV).
void __cdecl ModelSlot_LoadAdvisor(const char *folder, const char *c, const char *suffix, u32 slot);

// 0x598ca0 (cdecl; kind): on the model slot at [0xf79674] the part flag of the parts named by the kind: -1 clears the flags of the keys at 0xf797a4 (while bit 0x10000 of [0xf79660]
// is clear, each when set) and sets those of the keys at 0xf797b4 (four each); 0 clears 0x13 and 0x14; 1, 2, 4, 5, 6, 7, 9, 10, 11, 13 set 0x13 and 0x14; 14 sets 0x15; other kinds
// do nothing here. Finally, for a kind other than 0 and while that bit is clear, the flag of the part with the kind as its key is cleared (ModelSlot_ClearPartFlag).
void __cdecl ModelSlot_ApplySelection(s32 kind);
}
