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

// 0x4da920 (thiscall): the thing constructor (Thing_Construct, cell 1, kind 0xa), mNextObject (+0xc) = 0, then the HQ added to the message
// centre's slot 7. Returns hq. (The cell the original pushes has this's high address bits above the 1: Thing_Construct takes 16 bits.)
u8 *__fastcall MechanicHQ_Ctor(u8 *hq);
// 0x5089c0 (thiscall): the thing constructor (cell 1, kind 9), the training record at +0xc (StaffTraining_Ctor), the five 12-byte strike
// levels from +0x24 zeroed, +0x60 (word) and +0x64 = 0, then the message centre's slots 0xc and 0xe. Returns hq.
u8 *__fastcall StaffHQ_Ctor(u8 *hq);
}
