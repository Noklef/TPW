// The staff and mechanic headquarters (thing models 9 and 10): reimplementation of their serializers. Both start with the thing's own fields
// (Thing_Serialize); the mechanic HQ adds mNextObject (the word at +0xc), the staff HQ its training record at +0xc (StaffTraining_Serialize),
// mStrikeLevel[5] (twelve bytes each at +0x24), mStaffMemberPickedUp (the word at +0x60) and mForceStrike (the dword at +0x64).
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {
// 0x4da960 / 0x508bb0 (thiscall; stream, save, version, unused): 1 when Thing_Serialize fails, 0 for a version outside 0..1, then the fields
// saved or loaded as a serializer does (Save_Field; 1 at the first that fails). The staff HQ gives StaffTraining_Serialize's result as 0 or 1.
u32 __fastcall MechanicHQ_Serialize(u8 *hq, void *, Stream *stream, u32 save, s32 version, u32 unused);
u32 __fastcall StaffHQ_Serialize(u8 *hq, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
