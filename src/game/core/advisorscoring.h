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

// The park criteria of 0x59c8d0-0x59fb10 (thiscall on the scoring; names invented). "Flagged": the game object [[0x7cf6ec]]'s +0x1da710
// set (Get1da710_51a280; Game_Field1da710IsZero for the first two). The analyser is the game table entry GameTable7cfb90_Get1da720_519510
// of that object, the lab its Get1da722_519550 entry. Staff counts are Things_CountStaffOfKind's; the scans' game table getters that the
// originals call first are left out where their answers are not used. The bounds are signed, the products and differences wrap.
// 0x59c8d0: flagged and objects with definition +0x4ac 0 or 2 (Things_CountByDef4ac(0, 0), then (2, 0), both counted): the people at the
// ticket booths (ParkAnalyser_CountAtTicketBooths) times +0xa4. 0x59c960: flagged and no such objects: +0xa8.
s32 __fastcall AdvisorScoring_BoothPeople(const u8 *scoring);
s32 __fastcall AdvisorScoring_NoObjects0Or2(const u8 *scoring);
// 0x59ca80: no staff of kind 4: Objects_CountKind0InState1 times +0xc4.
s32 __fastcall AdvisorScoring_Objects0InState1NoStaff4(const u8 *scoring);
// 0x59cd30: not flagged, no staff of kind 7: ParkAnalyser_Tallied(0) times +0xfc plus +0xf8.
s32 __fastcall AdvisorScoring_TalliedNoStaff7(const u8 *scoring);
// 0x59ce30: not flagged, no staff of kind 6, more than 30 percent of the people below 30 (Things_PercentPeopleStat19cBelow): the
// analyser's half count (ParkAnalyser_Kind1OnPlainHalfClamped) less +0x10c, times +0x110.
s32 __fastcall AdvisorScoring_PeopleLow19cNoStaff6(const u8 *scoring);
// 0x59cea0: not flagged, no staff of kind 8, research of category 0 left (Research_CountNotDoneCat0), definitions of kind 0 with control
// +0x10 (ObjectDefs_CountKind0WithControls10(0), n) and a different count with +0x18 (ObjectDefs_CountKindWithControls18(0, 0), m): the
// percentage m * 100 / n (signed) when above +0x114, less it, times +0x118.
s32 __fastcall AdvisorScoring_Controls18ShareNoStaff8(const u8 *scoring);
// 0x59cf50: as 0x59cea0 up to the research, then m (counted first) equal to n: +0x11c.
s32 __fastcall AdvisorScoring_Controls18AllNoStaff8(const u8 *scoring);
// 0x59d3b0: staff of kind 7 and ParkAnalyser_Tallied(1) above +0x15c: +0x160.
s32 __fastcall AdvisorScoring_TalliedStaff7(const u8 *scoring);
// 0x59da30 / 0x59dd40 / 0x59dfd0: staff of kind 8, definitions of kind 0 / 1 / 2 with control +0x18 (ObjectDefs_CountKindWithControls18
// (kind, 0)), research of that category left (Research_CountNotDoneCat0 / 1 / 2), the lab's share of group 0 / 1 / 2 (Table_ScaledBySum) 0,
// and the count the same with control +0x10 (ObjectDefs_CountKind0 / 1 / 2WithControls10(0)): +0x22c / +0x264 / +0x288.
s32 __fastcall AdvisorScoring_ResearchKind0Staff8(const u8 *scoring);
s32 __fastcall AdvisorScoring_ResearchKind1Staff8(const u8 *scoring);
s32 __fastcall AdvisorScoring_ResearchKind2Staff8(const u8 *scoring);
// 0x59dbf0: staff of kind 8, definitions of kind 0 with control +0x14 (ObjectDefs_CountKind0WithControls14), not more than +0x240, and the
// lab's share of group 4 0: +0x244.
s32 __fastcall AdvisorScoring_Controls14Staff8(const u8 *scoring);
// 0x59e260: as 0x59da30 for kind and category 3, without the lab's share: +0x2ac.
s32 __fastcall AdvisorScoring_ResearchKind3Staff8(const u8 *scoring);
// 0x59e840: ParkAnalyser_Tallied(1) above +0x314: the tally (asked again) times +0x318.
s32 __fastcall AdvisorScoring_TalliedAbove(const u8 *scoring);
// 0x59f940 / 0x59f970: more people on plain cells with +0x210 set / in mode 2 (Things_CountKind1OnPlainCellWith210 / Mode2) than +0x4a0 /
// +0x4a8: +0x4a4 / +0x4ac.
s32 __fastcall AdvisorScoring_PeopleWith210(const u8 *scoring);
s32 __fastcall AdvisorScoring_PeopleMode2(const u8 *scoring);
// 0x59fa60: not flagged, the analyser's total (ParkAnalyser_Total) at least +0x4c4 and no staff (Things_CountStaff): +0x4c0.
s32 __fastcall AdvisorScoring_TotalNoStaff(const u8 *scoring);
// 0x59fac0: more staff than +0x4c8 and none of them with a patrol region (Things_CountStaffWithPatrol(0)): +0x4cc.
s32 __fastcall AdvisorScoring_StaffNoPatrol(const u8 *scoring);

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

// The park criteria of 0x5a02f0-0x5a08b0 (thiscall on the scoring; names invented). The analyser is the game table entry
// (GameTable7cfb90_Get1da720_519510) of the game object [[0x7cf6ec]] (of [0x7cf83c] for 0x5a0780); "flagged" as above. Shares are
// count * 100 / total (wrapping, signed), bounds signed; "the theme byte i" is GlobalSave_ThemeByte(GlobalSave_Instance(), i).
// 0x5a02f0: not flagged (Game_Field1da710IsZero), n objects with definition +0x4ac 0 (Things_CountByDef4ac(0, 0)), n at least +0x57c,
// and the idle ones' (Things_CountByDef4acIdle(0)) share above +0x580: +0x584. 0x5a0370 / 0x5a03d0: for +0x4ac 1 / 2, n not 0 (no
// flag, no least): the share above +0x588 / +0x590: +0x58c / +0x594.
s32 __fastcall AdvisorScoring_IdleObjects0Share(const u8 *scoring);
s32 __fastcall AdvisorScoring_IdleObjects1Share(const u8 *scoring);
s32 __fastcall AdvisorScoring_IdleObjects2Share(const u8 *scoring);
// 0x5a0430: n the kind 3 things with flag bit 0 (Game_RecountKind3Flag1 on the analyser, sic), not 0, and the idle ones'
// (Things_CountKind3Flag1) share above +0x598: +0x59c. 0x5a0490: the same for bit 1 (Things_CountKind3Flag2Any / Things_CountKind3Flag2),
// +0x5a0 / +0x5a4.
s32 __fastcall AdvisorScoring_IdleKind3Flag1Share(const u8 *scoring);
s32 __fastcall AdvisorScoring_IdleKind3Flag2Share(const u8 *scoring);
// 0x5a04f0: the analyser's tally ParkAnalyser_Tallied(0) above +0x5a8: +0x5ac.
s32 __fastcall AdvisorScoring_Tallied0Above(const u8 *scoring);
// 0x5a05a0: the analyser's total (ParkAnalyser_Total) above __ftol([0x785790] * float +0x5c4) and the theme byte 0 clear: +0x5c8.
// 0x5a0670: the money's total (Money_Total of the game table entry GameTable7cfb90_Get1da726_5195d0) above __ftol([0x7857a0] * float
// +0x5d4) and the theme byte 4 clear: +0x5d8.
s32 __fastcall AdvisorScoring_TotalAboveTheme0(const u8 *scoring);
s32 __fastcall AdvisorScoring_MoneyAboveTheme4(const u8 *scoring);
// 0x5a05f0: k = __ftol([0x785798] * float +0x5cc) below the people's average +0x19c (Things_AveragePersonStat19c on the analyser; or
// unordered with it, as fcompp's C0 has it), the people on plain cells (Things_CountKind1OnPlainCell) more than [0x78579c] and the
// theme byte 2 clear: +0x5d0.
s32 __fastcall AdvisorScoring_Mood19cTheme2(const u8 *scoring);
// 0x5a0780: the analyser's count at +0x216d8 (with its byte +0x20 set its +0x1c, else its +0x18 + 1) exactly 1: +0x5f4.
s32 __fastcall AdvisorScoring_AnalyserCountOne(const u8 *scoring);
// 0x5a07d0 / 0x5a0820 / 0x5a0870: not flagged (Get1da710_51a280 of [[0x7cf6ec]]) and the definitions' percentage with controls
// (ObjectDefs_PercentWithControls) above +0x5fc and at most +0x604 / above +0x604 and at most +0x60c / above +0x60c: +0x600 / +0x608 /
// +0x610.
s32 __fastcall AdvisorScoring_ControlsPercentLow(const u8 *scoring);
s32 __fastcall AdvisorScoring_ControlsPercentMid(const u8 *scoring);
s32 __fastcall AdvisorScoring_ControlsPercentHigh(const u8 *scoring);
// 0x5a08b0: not flagged, [0x785794] not 0 and the people on plain cells' share of it (Things_CountKind1OnPlainCell, asked again when
// the first share is not above 100; else 100) above +0x614: +0x618.
s32 __fastcall AdvisorScoring_PeopleShare(const u8 *scoring);

// More park criteria (thiscall on the scoring; names invented). 0x59f440: the analyser's (of [0x7cf83c]) count at +0x216d8 (as
// 0x5a0780 has it) above 0, and the buffer's newest sample (size +0x1c above 0; the cursor +0x18 if not negative, else, with the byte
// +0x20 set, cursor + size; the samples from +4) above +0x410: +8 the x of the bus stop B (Map_BusStopPos(G + 0x2d8, 1)), +0x414.
s32 __fastcall AdvisorScoring_LatestCountBusStop(u8 *scoring);
// 0x59f590: at least +0x42c staff (Things_CountStaff) and +0x430 not above their average +0x1f8 (Things_AverageStaffStat1f8; fcompp's
// C3 / C0, so unordered counts as not above): +0x434. 0x59f5e0: the same for the people on plain cells (Things_CountKind1OnPlainCell,
// Things_AveragePersonStat19c) against +0x438 / +0x43c: +0x440.
s32 __fastcall AdvisorScoring_StaffStat1f8NotAbove(const u8 *scoring);
s32 __fastcall AdvisorScoring_PeopleStat19cNotAbove(const u8 *scoring);
// 0x59fb10: more staff than +0x4d0 and the byte +0x20 of the game table entry for G's +0x1da718 (GameTable7cfb90_Get1da718_519450)
// clear: +0x4d4.
s32 __fastcall AdvisorScoring_StaffEntryClear(const u8 *scoring);
}
