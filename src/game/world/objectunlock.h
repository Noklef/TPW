// An object definition's availability against the global save (the count of set bytes left), and its unlocking: reimplementation of
// 0x4d4b90, 0x4d4ad0, 0x4d5b30 and 0x4d4fa0. The names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d4b90 (stdcall (id), pops 4; name invented): whether the definition of the id (ObjectDefs_Find on 0x7890a0, not checked) asks for
// no more (its s32 +0xc4) than the global save's count left (GlobalSave_CountSetLeft of GlobalSave_Instance; signed). Only AL is set.
u8 __stdcall ObjectDef_IsUnlocked(u32 id);

// 0x4d4ad0 (stdcall (id), pops 4; name invented): the definition's s32 +0xc4 read first (ObjectDefs_Find, not checked); with the mode
// setting (0xfb3b7c, read the first time under the guard 0xfb34ba & 1) 1, true. Otherwise false unless ObjectDef_IsUnlocked (inlined: the
// definition looked up again); then the id goes into the global save's set (Object_AddKeyed on GlobalSave_Instance, the +0xc4 its
// amount): whether it was new. Only AL is set.
u8 __stdcall ObjectDef_Unlock(u32 id);

// 0x4d5b30 (thiscall, this unused; called from 0x4d5235, 0x4d5824 and 0x4d5b05): when the global save's count (GlobalSave_CountSet of
// GlobalSave_Instance, inlined there) is 1, the advisor messages 0x137 and 0x13b (AdvisorMsg_Init: type 0, a 0, b -1, flag 0, scored)
// each posted as message 05 (Obj_CtorPart8, Object_PostMessage).
void __fastcall ObjectUnlock_FirstAdvice(u8 *self);

// 0x4d4fa0 (thiscall (group, level), pops 8; called from 0x4d4c0c and five more with the group 0-3 and the level an unlock count
// gave; its chunk from 0x4d5020 listed as a function of its own): the advisor message 0xd7, 0xd8 or 0xd9 for the level 1, 2 or 3
// (AdvisorMsg_Init: type 0, a 0, b the group, flag 0, scored) posted as message 05 (Obj_CtorPart8, Object_PostMessage); another level
// none (a debug print there). Then ObjectUnlock_FirstAdvice either way.
void __fastcall ObjectUnlock_AdviseLevel(u8 *self, void *, u32 group, s32 level);
}
