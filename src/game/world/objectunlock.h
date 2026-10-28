// An object definition's availability against the global save (the count of set bytes left), and its unlocking: reimplementation of
// 0x4d4b90 and 0x4d4ad0. The names are invented.
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
}
