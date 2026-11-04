// An advisor message sent to the message centre: reimplementation of 0x59b620 and 0x59b870. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void AdvisorSend_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x59b870 (thiscall (out, offset), pops 8): a message whose byte +0x14 is 0 gives 0 and out is not set. Else the byte cleared; gAdvisorScoring (0xfb3540) built the first time. The speech id is DefField24 of the message's id (+0xc),
// plus offset when offset (signed) is under DefField28. Id 0x266: *out = 0, returns 0. Else *out = Advisor_Say(id) and a message 18 (Table768fb8_Lookup(id), the word +0, the dword +4, -1) posted to the message centre; returns 1.
u32 __fastcall AdvisorMsg_Send(u8 *self, void *, u32 *out, s32 offset);
// 0x59b620 (thiscall (out, offset), pops 8): as AdvisorMsg_Send, but first the score of the id (AdvisorScoring_ScoreDetail): under [0xfb3560] (signed) gives 0 and out is not set. Its word and dword, when not 0, replace +0 and +4.
u32 __fastcall AdvisorMsg_SendScored(u8 *self, void *, u32 *out, s32 offset);
}
