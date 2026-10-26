// The advisor scoring's (gAdvisorScoring, 0xfb3540; its file laid out by AdvisorScoring_Construct in core/balancefile) scores: the
// criteria the table at 0x76e348 (set up at 0x5a0a51) points to. Reimplementation of 0x59c9d0 and 0x59ca00, and of the staff and object
// criteria listed below.
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
// The hiring limits are the dwords at 0x7852ec, 0x7852e8, 0x7852f0, 0x7852f4 and 0x7852f8 for kinds 4, 5, 6, 7, 8 (Staff_CountTypes).
// The counts are Things_CountStaffOfKind's, the averages Things_AverageOfKind's (signed). The game table getters the originals call
// first (GameTable7cfb90_Get1da720_519510, into ECX for the stdcall counts) are left out where their answers are not used.

// 0x59efa0 / 0x59f020 / 0x59f0a0 / 0x59f120: when the limit of kind 5 / 6 / 4 / 7 is the scoring's +0x38c / +0x398 / +0x3a4 / +0x3b0
// and that many of the kind or more are hired, the dword after it (+0x390 / +0x39c / +0x3a8 / +0x3b4); else 0. 0x59f1a0: kind 7's limit
// against +0x3b0 again (sic), then kind 8's count against +0x3bc: +0x3c0.
s32 __fastcall AdvisorScoring_StaffAtLimit5(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffAtLimit6(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffAtLimit4(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffAtLimit7(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffAtLimit8(const u8 *scoring);

// 0x59efe0 / 0x59f060 / 0x59f0e0 / 0x59f160 / 0x59f1e0: when the limit of kind 5 / 6 / 4 / 7 / 8 is not the scoring's +0x38c / +0x398 /
// +0x3a4 / +0x3b0 / +0x3bc and exactly the limit's number of the kind are hired, +0x394 / +0x3a0 / +0x3ac / +0x3b8 / +0x3c4; else 0.
s32 __fastcall AdvisorScoring_StaffFull5(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffFull6(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffFull4(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffFull7(const u8 *scoring);
s32 __fastcall AdvisorScoring_StaffFull8(const u8 *scoring);

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

// The object criteria (thiscall on the scoring; names unconfirmed): an object scan of world/thingscan on the game table entry
// (GameTable7cfb90_Get1da720_519510 of [[0x7cf6ec]], which the scans do not read) with a local handle. Category 2 / 3 / 4 the scan's
// category argument; 156 its categories 1, 5 and 6; Def4ac2 the definitions with 2 at +0x4ac.
// 0x59fb60, 0x59fbd0, 0x59fc40, 0x59fcb0, 0x59fd20 (the fewest walk-aways per 100 customers) and 0x59fd90, 0x59fe00, 0x59fe70, 0x59fee0
// (the unhappiest), with the bounds +0x4d8, +0x4e0, ..., +0x518 (8 apart); 0x59ff50 (bound +0x520) as 0x59fd20 (the fewest walk-aways
// of Def4ac2, sic: there is no unhappiest of Def4ac2): with a handle found and the scan's value below the bound (signed), the handle to
// +4 (u16), the value to +8 and (value - bound) times the s32 after the bound returned (32-bit); else 0. (The original's Debug_Printf,
// empty, is left out.)
s32 __fastcall AdvisorScoring_FewestWalkAways2(u8 *scoring);
s32 __fastcall AdvisorScoring_FewestWalkAways156(u8 *scoring);
s32 __fastcall AdvisorScoring_FewestWalkAways3(u8 *scoring);
s32 __fastcall AdvisorScoring_FewestWalkAways4(u8 *scoring);
s32 __fastcall AdvisorScoring_FewestWalkAwaysDef4ac2(u8 *scoring);
s32 __fastcall AdvisorScoring_Unhappiest2(u8 *scoring);
s32 __fastcall AdvisorScoring_Unhappiest156(u8 *scoring);
s32 __fastcall AdvisorScoring_Unhappiest3(u8 *scoring);
s32 __fastcall AdvisorScoring_Unhappiest4(u8 *scoring);
s32 __fastcall AdvisorScoring_UnhappiestDef4ac2(u8 *scoring);

// 0x59ffc0, 0x5a0010, 0x5a0060, 0x5a00b0, 0x5a0100 (the most walk-aways) and 0x5a0150, 0x5a01a0, 0x5a01f0, 0x5a0240, 0x5a0290 (the
// happiest) with the bounds +0x528, +0x530, ..., +0x570: as above with the value above the bound (signed).
s32 __fastcall AdvisorScoring_MostWalkAways2(u8 *scoring);
s32 __fastcall AdvisorScoring_MostWalkAways156(u8 *scoring);
s32 __fastcall AdvisorScoring_MostWalkAways3(u8 *scoring);
s32 __fastcall AdvisorScoring_MostWalkAways4(u8 *scoring);
s32 __fastcall AdvisorScoring_MostWalkAwaysDef4ac2(u8 *scoring);
s32 __fastcall AdvisorScoring_Happiest2(u8 *scoring);
s32 __fastcall AdvisorScoring_Happiest156(u8 *scoring);
s32 __fastcall AdvisorScoring_Happiest3(u8 *scoring);
s32 __fastcall AdvisorScoring_Happiest4(u8 *scoring);
s32 __fastcall AdvisorScoring_HappiestDef4ac2(u8 *scoring);

// 0x59c8c0-0x5a07c0, 61 of them (tests/fncmp/advisorfields.py lists them): the scoring object's dword getters (thiscall, MOV EAX,[ECX + offset];
// RET; no direct callers, names invented), each named for its offset.
s32 __fastcall AdvisorScoring_Fielda0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1bc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1c0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1c4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1c8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1cc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1d0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1d4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1d8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1dc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1e0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1e4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1e8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1ec(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1f0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1f4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1f8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field1fc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field200(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field204(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field208(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field20c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field210(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field214(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field218(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field21c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field248(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field24c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field250(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field254(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field2c4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field300(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field304(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field308(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field30c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field310(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field31c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field320(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field324(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field328(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field350(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3d4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3d8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3dc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3e0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3e8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3ec(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3f0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3f4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3f8(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field3fc(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field400(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field404(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field408(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field40c(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field424(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field428(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field578(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field5b4(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field5c0(const u8 *scoring);
s32 __fastcall AdvisorScoring_Field5f8(const u8 *scoring);
// 0x59f9a0 and 0x59f9b0 (thiscall, the object unread; names invented): the constants 10000 and 9999.
s32 __fastcall AdvisorScoring_Const10000(const u8 *scoring);
s32 __fastcall AdvisorScoring_Const9999(const u8 *scoring);
}
