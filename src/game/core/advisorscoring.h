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

// The staff criteria (thiscall on the scoring; names unconfirmed). Staff kinds: 4 handyman, 5 mechanic, 6 entertainer, 7 researcher,
// 8 guard (as Staff_TrainAll's).
// The counts are Things_CountStaffOfKind's, the averages Things_AverageOfKind's (signed). The game table getters the originals call
// first (GameTable7cfb90_Get1da720_519510, into ECX for the stdcall counts) are left out where their answers are not used.

// 0x59d620 / 0x59d680 / 0x59d6e0 / 0x59d740 / 0x59d7a0: kind 4 / 5 / 7 / 6 / 8 hired, the game table entry of [[0x7cf6ec]]
// (GameTable7cfb90_Get1da718_519450)'s signed dword +0x10 / +0xc / +0x18 / +0x14 / +0x1c not above 0, and the kind's average below
// +0x194 / +0x19c / +0x1a4 / +0x1ac / +0x1b4: the dword after it; else 0.
s32 __fastcall AdvisorScoring_StaffLow4(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffLow5(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffLow7(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffLow6(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffLow8(const u8 *scoring);

// 0x59f630 / 0x59f6a0 / 0x59f710 / 0x59f780 / 0x59f7f0: that entry's dword for kind 7 / 8 / 5 / 6 / 4 (as above) above 0, more of the kind
// hired than +0x444 / +0x450 / +0x45c / +0x468 / +0x474 and its average above the next dword: the one after that; else 0.
s32 __fastcall AdvisorScoring_StaffHigh7(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffHigh8(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffHigh5(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffHigh6(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffHigh4(const u8 *scoring);
}
