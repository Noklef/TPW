// The advisor scoring's (gAdvisorScoring, 0xfb3540; its file laid out by AdvisorScoring_Construct in core/balancefile) scores: the
// criteria the table at 0x76e348 (set up at 0x5a0a51) points to. Reimplementation of 0x59c9d0 and 0x59ca00.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x59c9d0 / 0x59ca00 (thiscall; names unconfirmed): the park's people above a bound, weighted: Things_CountPeopleStat1a4Above /
// Things_CountPeopleStat1a8Above (on the game table entry of the game object [[0x7cf6ec]], GameTable7cfb90_Get1da720_519510) with the
// byte +0xb0 / +0xb8 as the bound, times the s32 +0xac / +0xb4.
s32 __fastcall AdvisorScoring_PeopleAbove1a4(u8 *scoring);
s32 __fastcall AdvisorScoring_PeopleAbove1a8(u8 *scoring);
}
