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

// 0x4dadc0 (thiscall (event), pops 4, the HQ unused; Thing_HandleMessage's case 10): only an event of kind 7 (its slot 0; else asked again
// for the empty Debug_Printf, and nothing). The mechanic is its word +8. From the world's ([0x80239c]) word +0x1da73e along the HQs'
// chain (the game table 0x7cfb90's things, the next +0x210): an HQ whose word +0x218 is the mechanic ends it with nothing when the
// mechanic's +0x64 is set (Get64_4e03d0), else with the advisor message 0x46; one with +0x218 0 whose mode (Mode19c_ToIndex) is not 3 or
// 4 ends it with nothing; the chain's end gives 0x45, no first HQ 0x44. The message (AdvisorMsg_Init: type the mechanic, a its word +0xe,
// b -1, flag 0, no score; on the stack, bytes +2, +3 and +0x17 never written) posted to the world's advisor (GameTable7cfb90_Get1da71c_5194d0,
// Advisor_Post).
void __fastcall MechanicHQ_HandleMessage(u8 *hq, void *, u8 *event);
// 0x5089c0 (thiscall): the thing constructor (cell 1, kind 9), the training record at +0xc (StaffTraining_Ctor), the five 12-byte strike
// levels from +0x24 zeroed, +0x60 (word) and +0x64 = 0, then the message centre's slots 0xc and 0xe. Returns hq.
u8 *__fastcall StaffHQ_Ctor(u8 *hq);
}
