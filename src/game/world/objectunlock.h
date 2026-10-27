// An object definition's availability against the global save (the count of set bytes left): reimplementation of 0x4d4b90. The name is
// invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d4b90 (stdcall (id), pops 4; name invented): whether the definition of the id (ObjectDefs_Find on 0x7890a0, not checked) asks for
// no more (its s32 +0xc4) than the global save's count left (GlobalSave_CountSetLeft of GlobalSave_Instance; signed). Only AL is set.
u8 __stdcall ObjectDef_IsUnlocked(u32 id);
}
