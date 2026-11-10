#include "advisorscoring.h"

#include <cstdint>

#include "../math/x87.h"
#include "filetime.h"
#include "../world/analyser.h"
#include "../world/mapcell.h"
#include "../world/money.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/objectmsg.h"
#include "../world/objectunlock.h"
#include "../world/research.h"
#include "../world/staff.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "accessors.h"
#include "globalsave.h"
#include "../ui/dialog74fb20.h"
#include "../ui/quitconfirm.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *table() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))); }

// The game object [[0x7cf6ec]], whether its +0x1da710 is set (Get1da710_51a280), and its research lab (GameTable7cfb90_Get1da722_519550).
u8 *game() { return **reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec); }
bool flagged() { return Get1da710_51a280(game()) != 0; }
u8 *lab() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(game())); }

// The objects whose definition's +0x4ac is 0, or 2 (Things_CountByDef4ac, both counted).
bool objects0or2() {
    const bool some0 = Things_CountByDef4ac(0, 0) > 0;
    const bool some2 = Things_CountByDef4ac(2, 0) > 0;
    return some0 || some2;
}

// The object criteria: Below for the fewest / unhappiest, Above for the most / happiest.
s32 below(u8 *scoring, s32 found, u16 handle, u32 bound) {
    if (!handle || found >= at<s32>(scoring, bound))
        return 0;
    at<u16>(scoring, 4) = handle;
    const s32 score = (s32)(((u32)found - at<u32>(scoring, bound)) * at<u32>(scoring, bound + 4));
    at<s32>(scoring, 8) = found;
    return score;
}

s32 above(u8 *scoring, s32 found, u16 handle, u32 bound) {
    if (!handle || found <= at<s32>(scoring, bound))
        return 0;
    at<s32>(scoring, 8) = found;
    const s32 score = (s32)(((u32)found - at<u32>(scoring, bound)) * at<u32>(scoring, bound + 4));
    at<u16>(scoring, 4) = handle;
    return score;
}

// +0xc..+0x1c of the game table entry for +0x1da718 of [[0x7cf6ec]]: the dword of a staff kind.
s32 entry_dword(u32 offset) {
    return at<s32>(reinterpret_cast<void *>((uintptr_t)GameTable7cfb90_Get1da718_519450(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))), offset);
}

const AdvisorCriteriaEnv kCriteriaPorts = {&ParkAnalyser_ParkScore, &Things_CountKind1OnPlainCell, &Things_AveragePersonStat19c, &Source_Field0c, &Money_AnyLoanOffer, &ParkAnalyser_FindCutOff};
const AdvisorCriteriaEnv *g_criteria = &kCriteriaPorts;

s32 crowdAverage(u8 *scoring, u32 bound) {
    if ((s32)g_criteria->score(table(), nullptr, 10) <= at<s32>(scoring, bound)) return 0;
    // (The original asks the table entry here too, a pure read, ECX for the cdecl count.)
    if (g_criteria->count() <= at<s32>(scoring, bound + 8)) return 0;
    const double average = g_criteria->average(table(), nullptr, 10);
    return !((double)at<s32>(scoring, bound + 4) >= average) ? at<s32>(scoring, bound + 0xc) : 0;
}

s32 limit_of(u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); }

s32 at_limit(const u8 *scoring, u32 limit, u32 bound, u32 kind, u32 against) {
    if (limit_of(limit) != at<s32>(scoring, bound)) return 0;
    return Things_CountStaffOfKind(kind) >= at<s32>(scoring, against) ? at<s32>(scoring, against + 4) : 0;
}

s32 full(const u8 *scoring, u32 limit, u32 bound, u32 kind) {
    if (limit_of(limit) == at<s32>(scoring, bound)) return 0;
    return Things_CountStaffOfKind(kind) == limit_of(limit) ? at<s32>(scoring, bound + 8) : 0;
}

s32 low(const u8 *scoring, u32 kind, u32 offset, u32 bound) {
    if (Things_CountStaffOfKind(kind) == 0 || entry_dword(offset) > 0) return 0;
    return Things_AverageOfKind(kind) < at<s32>(scoring, bound) ? at<s32>(scoring, bound + 4) : 0;
}

s32 high(const u8 *scoring, u32 kind, u32 offset, u32 bound) {
    if (entry_dword(offset) <= 0 || Things_CountStaffOfKind(kind) <= at<s32>(scoring, bound)) return 0;
    return Things_AverageOfKind(kind) > at<s32>(scoring, bound + 4) ? at<s32>(scoring, bound + 8) : 0;
}
}  // namespace

void AdvisorCriteria_SetEnv(const AdvisorCriteriaEnv *env) { g_criteria = env ? env : &kCriteriaPorts; }

extern "C" {

s32 __fastcall AdvisorScoring_PeopleAbove1a4(u8 *scoring) {
    return Things_CountPeopleStat1a4Above(table(), nullptr, at<u8>(scoring, 0xb0)) * at<s32>(scoring, 0xac);
}

s32 __fastcall AdvisorScoring_PeopleAbove1a8(u8 *scoring) {
    return Things_CountPeopleStat1a8Above(table(), nullptr, at<u8>(scoring, 0xb8)) * at<s32>(scoring, 0xb4);
}

s32 __fastcall AdvisorScoring_BoothPeople(const u8 *scoring) {
    if (Game_Field1da710IsZero() || !objects0or2()) return 0;
    return (s32)(ParkAnalyser_CountAtTicketBooths(table()) * at<u32>(scoring, 0xa4));
}

s32 __fastcall AdvisorScoring_NoObjects0Or2(const u8 *scoring) {
    if (!Game_Field1da710IsZero() || objects0or2()) return 0;
    return at<s32>(scoring, 0xa8);
}

s32 __fastcall AdvisorScoring_Objects0InState1NoStaff4(const u8 *scoring) {
    if (Things_CountStaffOfKind(4) > 0) return 0;
    return (s32)((u32)Objects_CountKind0InState1() * at<u32>(scoring, 0xc4));
}

s32 __fastcall AdvisorScoring_Objects0WornNoStaff4(const u8 *scoring) {
    if (Things_CountStaffOfKind(4) > 0) return 0;
    return (s32)((u32)Objects_CountKind0Below44OrState1(at<s32>(scoring, 0xc0)) * at<u32>(scoring, 0xbc));
}

s32 __fastcall AdvisorScoring_MapShareAtMostNoStaff5(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(5) > 0) return 0;
    const s32 share = Map_PercentWith2c();
    if (share > at<s32>(scoring, 0xd8)) return 0;
    return (s32)((u32)share * at<u32>(scoring, 0xd0) - at<u32>(scoring, 0xd4));
}

s32 __fastcall AdvisorScoring_MapShareAtLeastNoStaff5(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(5) > 0) return 0;
    const s32 share = Map_PercentWith2c();
    if (share < at<s32>(scoring, 0xe0)) return 0;
    return (s32)((u32)share * at<u32>(scoring, 0xdc) - at<u32>(scoring, 0xe0));
}

s32 __fastcall AdvisorScoring_TalliedNoStaff7(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(7) > 0) return 0;
    return (s32)(ParkAnalyser_Tallied(table(), nullptr, 0) * at<u32>(scoring, 0xfc) + at<u32>(scoring, 0xf8));
}

s32 __fastcall AdvisorScoring_PeopleLow19cNoStaff6(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(6) > 0 || Things_PercentPeopleStat19cBelow(table(), nullptr, 0x1e) <= 0x1e) return 0;
    return (s32)(((u32)ParkAnalyser_Kind1OnPlainHalfClamped(table()) - at<u32>(scoring, 0x10c)) * at<u32>(scoring, 0x110));
}

s32 __fastcall AdvisorScoring_Controls18ShareNoStaff8(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(8) > 0 || Research_CountNotDoneCat0() == 0) return 0;
    const s32 all = ObjectDefs_CountKind0WithControls10(0);
    if (all == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(0, 0);
    if (some == all) return 0;
    const s32 percent = (s32)((u32)some * 100u) / all;
    if (percent <= at<s32>(scoring, 0x114)) return 0;
    return (s32)(((u32)percent - at<u32>(scoring, 0x114)) * at<u32>(scoring, 0x118));
}

s32 __fastcall AdvisorScoring_Controls18AllNoStaff8(const u8 *scoring) {
    if (flagged() || Things_CountStaffOfKind(8) > 0 || Research_CountNotDoneCat0() == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(0, 0);
    return some == ObjectDefs_CountKind0WithControls10(0) ? at<s32>(scoring, 0x11c) : 0;
}

s32 __fastcall AdvisorScoring_TalliedStaff7(const u8 *scoring) {
    if (Things_CountStaffOfKind(7) == 0) return 0;
    return (s32)ParkAnalyser_Tallied(table(), nullptr, 1) > at<s32>(scoring, 0x15c) ? at<s32>(scoring, 0x160) : 0;
}

s32 __fastcall AdvisorScoring_ResearchKind0Staff8(const u8 *scoring) {
    if (Things_CountStaffOfKind(8) == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(0, 0);
    if (some == 0 || Research_CountNotDoneCat0() == 0 || Table_ScaledBySum(lab(), nullptr, 0) != 0) return 0;
    return some == ObjectDefs_CountKind0WithControls10(0) ? at<s32>(scoring, 0x22c) : 0;
}

s32 __fastcall AdvisorScoring_Controls14Staff8(const u8 *scoring) {
    if (Things_CountStaffOfKind(8) == 0) return 0;
    const s32 some = ObjectDefs_CountKind0WithControls14(0);
    if (some == 0 || some > at<s32>(scoring, 0x240) || Table_ScaledBySum(lab(), nullptr, 4) != 0) return 0;
    return at<s32>(scoring, 0x244);
}

s32 __fastcall AdvisorScoring_ResearchKind1Staff8(const u8 *scoring) {
    if (Things_CountStaffOfKind(8) == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(1, 0);
    if (some == 0 || Research_CountNotDoneCat1() == 0 || Table_ScaledBySum(lab(), nullptr, 1) != 0) return 0;
    return some == ObjectDefs_CountKind1WithControls10(0) ? at<s32>(scoring, 0x264) : 0;
}

s32 __fastcall AdvisorScoring_ResearchKind2Staff8(const u8 *scoring) {
    if (Things_CountStaffOfKind(8) == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(2, 0);
    if (some == 0 || Research_CountNotDoneCat2() == 0 || Table_ScaledBySum(lab(), nullptr, 2) != 0) return 0;
    return some == ObjectDefs_CountKind2WithControls10(0) ? at<s32>(scoring, 0x288) : 0;
}

s32 __fastcall AdvisorScoring_ResearchKind3Staff8(const u8 *scoring) {
    if (Things_CountStaffOfKind(8) == 0) return 0;
    const s32 some = ObjectDefs_CountKindWithControls18(3, 0);
    if (some == 0 || Research_CountNotDoneCat3() == 0) return 0;
    return some == ObjectDefs_CountKind3WithControls10(0) ? at<s32>(scoring, 0x2ac) : 0;
}

s32 __fastcall AdvisorScoring_TalliedAbove(const u8 *scoring) {
    if ((s32)ParkAnalyser_Tallied(table(), nullptr, 1) <= at<s32>(scoring, 0x314)) return 0;
    return (s32)(ParkAnalyser_Tallied(table(), nullptr, 1) * at<u32>(scoring, 0x318));
}

s32 __fastcall AdvisorScoring_PeopleWith210(const u8 *scoring) {
    return Things_CountKind1OnPlainCellWith210() > at<s32>(scoring, 0x4a0) ? at<s32>(scoring, 0x4a4) : 0;
}

s32 __fastcall AdvisorScoring_PeopleMode2(const u8 *scoring) {
    return Things_CountKind1OnPlainCellMode2() > at<s32>(scoring, 0x4a8) ? at<s32>(scoring, 0x4ac) : 0;
}

s32 __fastcall AdvisorScoring_TotalNoStaff(const u8 *scoring) {
    if (flagged() || (s32)ParkAnalyser_Total(table()) < at<s32>(scoring, 0x4c4) || Things_CountStaff() != 0) return 0;
    return at<s32>(scoring, 0x4c0);
}

s32 __fastcall AdvisorScoring_StaffNoPatrol(const u8 *scoring) {
    if (Things_CountStaff() <= at<s32>(scoring, 0x4c8) || Things_CountStaffWithPatrol(0) != 0) return 0;
    return at<s32>(scoring, 0x4cc);
}

s32 __fastcall AdvisorScoring_StaffAtLimit5(const u8 *scoring) { return at_limit(scoring, 0x007852e8, 0x38c, 5, 0x38c); }
s32 __fastcall AdvisorScoring_StaffAtLimit6(const u8 *scoring) { return at_limit(scoring, 0x007852f0, 0x398, 6, 0x398); }
s32 __fastcall AdvisorScoring_StaffAtLimit4(const u8 *scoring) { return at_limit(scoring, 0x007852ec, 0x3a4, 4, 0x3a4); }
s32 __fastcall AdvisorScoring_StaffAtLimit7(const u8 *scoring) { return at_limit(scoring, 0x007852f4, 0x3b0, 7, 0x3b0); }
s32 __fastcall AdvisorScoring_StaffAtLimit8(const u8 *scoring) { return at_limit(scoring, 0x007852f4, 0x3b0, 8, 0x3bc); }  // sic: kind 7's limit

s32 __fastcall AdvisorScoring_StaffFull5(const u8 *scoring) { return full(scoring, 0x007852e8, 0x38c, 5); }
s32 __fastcall AdvisorScoring_StaffFull6(const u8 *scoring) { return full(scoring, 0x007852f0, 0x398, 6); }
s32 __fastcall AdvisorScoring_StaffFull4(const u8 *scoring) { return full(scoring, 0x007852ec, 0x3a4, 4); }
s32 __fastcall AdvisorScoring_StaffFull7(const u8 *scoring) { return full(scoring, 0x007852f4, 0x3b0, 7); }
s32 __fastcall AdvisorScoring_StaffFull8(const u8 *scoring) { return full(scoring, 0x007852f8, 0x3bc, 8); }

s32 __fastcall AdvisorScoring_StaffLow4(const u8 *scoring) { return low(scoring, 4, 0x10, 0x194); }
s32 __fastcall AdvisorScoring_StaffLow5(const u8 *scoring) { return low(scoring, 5, 0xc, 0x19c); }
s32 __fastcall AdvisorScoring_StaffLow7(const u8 *scoring) { return low(scoring, 7, 0x18, 0x1a4); }
s32 __fastcall AdvisorScoring_StaffLow6(const u8 *scoring) { return low(scoring, 6, 0x14, 0x1ac); }
s32 __fastcall AdvisorScoring_StaffLow8(const u8 *scoring) { return low(scoring, 8, 0x1c, 0x1b4); }

s32 __fastcall AdvisorScoring_StaffHigh7(const u8 *scoring) { return high(scoring, 7, 0x18, 0x444); }
s32 __fastcall AdvisorScoring_StaffHigh8(const u8 *scoring) { return high(scoring, 8, 0x1c, 0x450); }
s32 __fastcall AdvisorScoring_StaffHigh5(const u8 *scoring) { return high(scoring, 5, 0xc, 0x45c); }
s32 __fastcall AdvisorScoring_StaffHigh6(const u8 *scoring) { return high(scoring, 6, 0x14, 0x468); }
s32 __fastcall AdvisorScoring_StaffHigh4(const u8 *scoring) { return high(scoring, 4, 0x10, 0x474); }

s32 __fastcall AdvisorScoring_CrowdAverage480(u8 *scoring) { return crowdAverage(scoring, 0x480); }
s32 __fastcall AdvisorScoring_CrowdAverage490(u8 *scoring) { return crowdAverage(scoring, 0x490); }

s32 __fastcall AdvisorScoring_InDebt(u8 *scoring) {
    const u32 entry = GameTable7cfb90_Get1da726_5195d0(game());
    if ((s32)g_criteria->field0c(reinterpret_cast<const TrackedSource *>((uintptr_t)entry)) >= 0) return 0;
    const u32 money = GameTable7cfb90_Get1da726_5195d0(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c));
    return g_criteria->anyLoan(reinterpret_cast<u8 *>((uintptr_t)money)) != 0 ? at<s32>(scoring, 0x340) : 0;
}

s32 __fastcall AdvisorScoring_CutOffObject(u8 *scoring) {
    u16 handle;
    const u32 entry = GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c));
    const u16 found = *g_criteria->findCutOff(reinterpret_cast<u8 *>((uintptr_t)entry), nullptr, &handle, 0);
    at<u16>(scoring, 4) = found;
    return found != 0 ? at<s32>(scoring, 0x388) : 0;
}

s32 __fastcall AdvisorScoring_FewestWalkAways2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysOf(table(), nullptr, 2, &handle);
    return below(scoring, found, handle, 0x4d8);
}

s32 __fastcall AdvisorScoring_FewestWalkAways156(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysOf156(table(), nullptr, &handle);
    return below(scoring, found, handle, 0x4e0);
}

s32 __fastcall AdvisorScoring_FewestWalkAways3(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysOf(table(), nullptr, 3, &handle);
    return below(scoring, found, handle, 0x4e8);
}

s32 __fastcall AdvisorScoring_FewestWalkAways4(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysOf(table(), nullptr, 4, &handle);
    return below(scoring, found, handle, 0x4f0);
}

s32 __fastcall AdvisorScoring_FewestWalkAwaysDef4ac2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysDef4ac2(table(), nullptr, &handle);
    return below(scoring, found, handle, 0x4f8);
}

s32 __fastcall AdvisorScoring_Unhappiest2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_UnhappiestOf(table(), nullptr, 2, &handle);
    return below(scoring, found, handle, 0x500);
}

s32 __fastcall AdvisorScoring_Unhappiest156(u8 *scoring) {
    u16 handle;
    const s32 found = Things_UnhappiestOf156(table(), nullptr, &handle);
    return below(scoring, found, handle, 0x508);
}

s32 __fastcall AdvisorScoring_Unhappiest3(u8 *scoring) {
    u16 handle;
    const s32 found = Things_UnhappiestOf(table(), nullptr, 3, &handle);
    return below(scoring, found, handle, 0x510);
}

s32 __fastcall AdvisorScoring_Unhappiest4(u8 *scoring) {
    u16 handle;
    const s32 found = Things_UnhappiestOf(table(), nullptr, 4, &handle);
    return below(scoring, found, handle, 0x518);
}

s32 __fastcall AdvisorScoring_UnhappiestDef4ac2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_FewestWalkAwaysDef4ac2(table(), nullptr, &handle);
    return below(scoring, found, handle, 0x520);
}

s32 __fastcall AdvisorScoring_MostWalkAways2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_MostWalkAwaysOf(table(), nullptr, 2, &handle);
    return above(scoring, found, handle, 0x528);
}

s32 __fastcall AdvisorScoring_MostWalkAways156(u8 *scoring) {
    u16 handle;
    const s32 found = Things_MostWalkAwaysOf156(table(), nullptr, &handle);
    return above(scoring, found, handle, 0x530);
}

s32 __fastcall AdvisorScoring_MostWalkAways3(u8 *scoring) {
    u16 handle;
    const s32 found = Things_MostWalkAwaysOf(table(), nullptr, 3, &handle);
    return above(scoring, found, handle, 0x538);
}

s32 __fastcall AdvisorScoring_MostWalkAways4(u8 *scoring) {
    u16 handle;
    const s32 found = Things_MostWalkAwaysOf(table(), nullptr, 4, &handle);
    return above(scoring, found, handle, 0x540);
}

s32 __fastcall AdvisorScoring_MostWalkAwaysDef4ac2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_MostWalkAwaysDef4ac2(table(), nullptr, &handle);
    return above(scoring, found, handle, 0x548);
}

s32 __fastcall AdvisorScoring_Happiest2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_HappiestOf(table(), nullptr, 2, &handle);
    return above(scoring, found, handle, 0x550);
}

s32 __fastcall AdvisorScoring_Happiest156(u8 *scoring) {
    u16 handle;
    const s32 found = Things_HappiestOf156(table(), nullptr, &handle);
    return above(scoring, found, handle, 0x558);
}

s32 __fastcall AdvisorScoring_Happiest3(u8 *scoring) {
    u16 handle;
    const s32 found = Things_HappiestOf(table(), nullptr, 3, &handle);
    return above(scoring, found, handle, 0x560);
}

s32 __fastcall AdvisorScoring_Happiest4(u8 *scoring) {
    u16 handle;
    const s32 found = Things_HappiestOf(table(), nullptr, 4, &handle);
    return above(scoring, found, handle, 0x568);
}

s32 __fastcall AdvisorScoring_HappiestDef4ac2(u8 *scoring) {
    u16 handle;
    const s32 found = Things_HappiestDef4ac2(table(), nullptr, &handle);
    return above(scoring, found, handle, 0x570);
}

s32 __fastcall AdvisorScoring_Fielda0(const u8 *scoring) { return at<s32>(scoring, 0xa0); }
s32 __fastcall AdvisorScoring_Field1bc(const u8 *scoring) { return at<s32>(scoring, 0x1bc); }
s32 __fastcall AdvisorScoring_Field1c0(const u8 *scoring) { return at<s32>(scoring, 0x1c0); }
s32 __fastcall AdvisorScoring_Field1c4(const u8 *scoring) { return at<s32>(scoring, 0x1c4); }
s32 __fastcall AdvisorScoring_Field1c8(const u8 *scoring) { return at<s32>(scoring, 0x1c8); }
s32 __fastcall AdvisorScoring_Field1cc(const u8 *scoring) { return at<s32>(scoring, 0x1cc); }
s32 __fastcall AdvisorScoring_Field1d0(const u8 *scoring) { return at<s32>(scoring, 0x1d0); }
s32 __fastcall AdvisorScoring_Field1d4(const u8 *scoring) { return at<s32>(scoring, 0x1d4); }
s32 __fastcall AdvisorScoring_Field1d8(const u8 *scoring) { return at<s32>(scoring, 0x1d8); }
s32 __fastcall AdvisorScoring_Field1dc(const u8 *scoring) { return at<s32>(scoring, 0x1dc); }
s32 __fastcall AdvisorScoring_Field1e0(const u8 *scoring) { return at<s32>(scoring, 0x1e0); }
s32 __fastcall AdvisorScoring_Field1e4(const u8 *scoring) { return at<s32>(scoring, 0x1e4); }
s32 __fastcall AdvisorScoring_Field1e8(const u8 *scoring) { return at<s32>(scoring, 0x1e8); }
s32 __fastcall AdvisorScoring_Field1ec(const u8 *scoring) { return at<s32>(scoring, 0x1ec); }
s32 __fastcall AdvisorScoring_Field1f0(const u8 *scoring) { return at<s32>(scoring, 0x1f0); }
s32 __fastcall AdvisorScoring_Field1f4(const u8 *scoring) { return at<s32>(scoring, 0x1f4); }
s32 __fastcall AdvisorScoring_Field1f8(const u8 *scoring) { return at<s32>(scoring, 0x1f8); }
s32 __fastcall AdvisorScoring_Field1fc(const u8 *scoring) { return at<s32>(scoring, 0x1fc); }
s32 __fastcall AdvisorScoring_Field200(const u8 *scoring) { return at<s32>(scoring, 0x200); }
s32 __fastcall AdvisorScoring_Field204(const u8 *scoring) { return at<s32>(scoring, 0x204); }
s32 __fastcall AdvisorScoring_Field208(const u8 *scoring) { return at<s32>(scoring, 0x208); }
s32 __fastcall AdvisorScoring_Field20c(const u8 *scoring) { return at<s32>(scoring, 0x20c); }
s32 __fastcall AdvisorScoring_Field210(const u8 *scoring) { return at<s32>(scoring, 0x210); }
s32 __fastcall AdvisorScoring_Field214(const u8 *scoring) { return at<s32>(scoring, 0x214); }
s32 __fastcall AdvisorScoring_Field218(const u8 *scoring) { return at<s32>(scoring, 0x218); }
s32 __fastcall AdvisorScoring_Field21c(const u8 *scoring) { return at<s32>(scoring, 0x21c); }
s32 __fastcall AdvisorScoring_Field248(const u8 *scoring) { return at<s32>(scoring, 0x248); }
s32 __fastcall AdvisorScoring_Field24c(const u8 *scoring) { return at<s32>(scoring, 0x24c); }
s32 __fastcall AdvisorScoring_Field250(const u8 *scoring) { return at<s32>(scoring, 0x250); }
s32 __fastcall AdvisorScoring_Field254(const u8 *scoring) { return at<s32>(scoring, 0x254); }
s32 __fastcall AdvisorScoring_Field2c4(const u8 *scoring) { return at<s32>(scoring, 0x2c4); }
s32 __fastcall AdvisorScoring_Field300(const u8 *scoring) { return at<s32>(scoring, 0x300); }
s32 __fastcall AdvisorScoring_Field304(const u8 *scoring) { return at<s32>(scoring, 0x304); }
s32 __fastcall AdvisorScoring_Field308(const u8 *scoring) { return at<s32>(scoring, 0x308); }
s32 __fastcall AdvisorScoring_Field30c(const u8 *scoring) { return at<s32>(scoring, 0x30c); }
s32 __fastcall AdvisorScoring_Field310(const u8 *scoring) { return at<s32>(scoring, 0x310); }
s32 __fastcall AdvisorScoring_Field31c(const u8 *scoring) { return at<s32>(scoring, 0x31c); }
s32 __fastcall AdvisorScoring_Field320(const u8 *scoring) { return at<s32>(scoring, 0x320); }
s32 __fastcall AdvisorScoring_Field324(const u8 *scoring) { return at<s32>(scoring, 0x324); }
s32 __fastcall AdvisorScoring_Field328(const u8 *scoring) { return at<s32>(scoring, 0x328); }
s32 __fastcall AdvisorScoring_Field350(const u8 *scoring) { return at<s32>(scoring, 0x350); }
s32 __fastcall AdvisorScoring_Field3d4(const u8 *scoring) { return at<s32>(scoring, 0x3d4); }
s32 __fastcall AdvisorScoring_Field3d8(const u8 *scoring) { return at<s32>(scoring, 0x3d8); }
s32 __fastcall AdvisorScoring_Field3dc(const u8 *scoring) { return at<s32>(scoring, 0x3dc); }
s32 __fastcall AdvisorScoring_Field3e0(const u8 *scoring) { return at<s32>(scoring, 0x3e0); }
s32 __fastcall AdvisorScoring_Field3e8(const u8 *scoring) { return at<s32>(scoring, 0x3e8); }
s32 __fastcall AdvisorScoring_Field3ec(const u8 *scoring) { return at<s32>(scoring, 0x3ec); }
s32 __fastcall AdvisorScoring_Field3f0(const u8 *scoring) { return at<s32>(scoring, 0x3f0); }
s32 __fastcall AdvisorScoring_Field3f4(const u8 *scoring) { return at<s32>(scoring, 0x3f4); }
s32 __fastcall AdvisorScoring_Field3f8(const u8 *scoring) { return at<s32>(scoring, 0x3f8); }
s32 __fastcall AdvisorScoring_Field3fc(const u8 *scoring) { return at<s32>(scoring, 0x3fc); }
s32 __fastcall AdvisorScoring_Field400(const u8 *scoring) { return at<s32>(scoring, 0x400); }
s32 __fastcall AdvisorScoring_Field404(const u8 *scoring) { return at<s32>(scoring, 0x404); }
s32 __fastcall AdvisorScoring_Field408(const u8 *scoring) { return at<s32>(scoring, 0x408); }
s32 __fastcall AdvisorScoring_Field40c(const u8 *scoring) { return at<s32>(scoring, 0x40c); }
s32 __fastcall AdvisorScoring_Field424(const u8 *scoring) { return at<s32>(scoring, 0x424); }
s32 __fastcall AdvisorScoring_Field428(const u8 *scoring) { return at<s32>(scoring, 0x428); }
s32 __fastcall AdvisorScoring_Field578(const u8 *scoring) { return at<s32>(scoring, 0x578); }
s32 __fastcall AdvisorScoring_Field5b4(const u8 *scoring) { return at<s32>(scoring, 0x5b4); }
s32 __fastcall AdvisorScoring_Field5c0(const u8 *scoring) { return at<s32>(scoring, 0x5c0); }
s32 __fastcall AdvisorScoring_Field5f8(const u8 *scoring) { return at<s32>(scoring, 0x5f8); }

s32 __fastcall AdvisorScoring_Const10000(const u8 *) { return 10000; }
s32 __fastcall AdvisorScoring_Const9999(const u8 *) { return 9999; }

}  // extern "C"

namespace {

u8 *world_table() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c))); }

s32 global_s32(u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); }

// count * 100 / total (the product wrapping; signed, truncated).
s32 share(s32 count, s32 total) { return (s32)((u32)count * 100) / total; }

// fild [address]; fmul float; __ftol: the bound the totals are held against.
s32 scaled(u32 address, float f) {
    const s32 n = global_s32(address);
    const double v = n < 0 ? -X87_MulInt(0u - (u32)n, f) : X87_MulInt((u32)n, f);
    return (s32)(long long)v;
}

bool theme_clear(u32 i) { return GlobalSave_ThemeByte(GlobalSave_Instance(), nullptr, i) == 0; }

s32 idle_share(const u8 *scoring, s32 value, u32 bound) {
    const s32 n = Things_CountByDef4ac(value, 0);
    if (n == 0)
        return 0;
    return share(Things_CountByDef4acIdle(value), n) > at<s32>(scoring, bound) ? at<s32>(scoring, bound + 4) : 0;
}

s32 controls_percent(const u8 *scoring, u32 low, u32 high, u32 value) {
    if (Get1da710_51a280(game()) != 0)
        return 0;
    const s32 p = ObjectDefs_PercentWithControls();
    return p > at<s32>(scoring, low) && p <= at<s32>(scoring, high) ? at<s32>(scoring, value) : 0;
}

}  // namespace

extern "C" {

s32 __fastcall AdvisorScoring_IdleObjects0Share(const u8 *scoring) {
    if (!Game_Field1da710IsZero())
        return 0;
    const s32 n = Things_CountByDef4ac(0, 0);
    if (n == 0 || n < at<s32>(scoring, 0x57c))
        return 0;
    return share(Things_CountByDef4acIdle(0), n) > at<s32>(scoring, 0x580) ? at<s32>(scoring, 0x584) : 0;
}

s32 __fastcall AdvisorScoring_IdleObjects1Share(const u8 *scoring) { return idle_share(scoring, 1, 0x588); }
s32 __fastcall AdvisorScoring_IdleObjects2Share(const u8 *scoring) { return idle_share(scoring, 2, 0x590); }

s32 __fastcall AdvisorScoring_IdleKind3Flag1Share(const u8 *scoring) {
    const s32 n = (s32)Game_RecountKind3Flag1(table());
    if (n == 0)
        return 0;
    return share(Things_CountKind3Flag1(), n) > at<s32>(scoring, 0x598) ? at<s32>(scoring, 0x59c) : 0;
}

s32 __fastcall AdvisorScoring_IdleKind3Flag2Share(const u8 *scoring) {
    const s32 n = Things_CountKind3Flag2Any();
    if (n == 0)
        return 0;
    return share(Things_CountKind3Flag2(), n) > at<s32>(scoring, 0x5a0) ? at<s32>(scoring, 0x5a4) : 0;
}

s32 __fastcall AdvisorScoring_Tallied0Above(const u8 *scoring) {
    return (s32)ParkAnalyser_Tallied(table(), nullptr, 0) > at<s32>(scoring, 0x5a8) ? at<s32>(scoring, 0x5ac) : 0;
}

s32 __fastcall AdvisorScoring_TotalAboveTheme0(const u8 *scoring) {
    const s32 total = (s32)ParkAnalyser_Total(table());
    if (total <= scaled(0x00785790, at<float>(scoring, 0x5c4)) || !theme_clear(0))
        return 0;
    return at<s32>(scoring, 0x5c8);
}

s32 __fastcall AdvisorScoring_Mood19cTheme2(const u8 *scoring) {
    const double average = Things_AveragePersonStat19c(table(), nullptr, 0);
    const s32 k = scaled(0x00785798, at<float>(scoring, 0x5cc));
    if ((double)k >= average || Things_CountKind1OnPlainCell() <= global_s32(0x0078579c) || !theme_clear(2))
        return 0;
    return at<s32>(scoring, 0x5d0);
}

s32 __fastcall AdvisorScoring_MoneyAboveTheme4(const u8 *scoring) {
    const s32 money = (s32)Money_Total(reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(game())));
    if (money <= scaled(0x007857a0, at<float>(scoring, 0x5d4)) || !theme_clear(4))
        return 0;
    return at<s32>(scoring, 0x5d8);
}

s32 __fastcall AdvisorScoring_AnalyserCountOne(const u8 *scoring) {
    const u8 *const counts = world_table() + 0x216d8;
    const s32 count = counts[0x20] ? at<s32>(counts, 0x1c) : (s32)(at<u32>(counts, 0x18) + 1);
    return count == 1 ? at<s32>(scoring, 0x5f4) : 0;
}

s32 __fastcall AdvisorScoring_ControlsPercentLow(const u8 *scoring) { return controls_percent(scoring, 0x5fc, 0x604, 0x600); }
s32 __fastcall AdvisorScoring_ControlsPercentMid(const u8 *scoring) { return controls_percent(scoring, 0x604, 0x60c, 0x608); }

s32 __fastcall AdvisorScoring_ControlsPercentHigh(const u8 *scoring) {
    if (Get1da710_51a280(game()) != 0)
        return 0;
    return ObjectDefs_PercentWithControls() > at<s32>(scoring, 0x60c) ? at<s32>(scoring, 0x610) : 0;
}

s32 __fastcall AdvisorScoring_PeopleShare(const u8 *scoring) {
    if (Get1da710_51a280(game()) != 0 || global_s32(0x00785794) == 0)
        return 0;
    s32 p = share(Things_CountKind1OnPlainCell(), global_s32(0x00785794));
    if (p > 100)
        p = 100;
    else
        p = share(Things_CountKind1OnPlainCell(), global_s32(0x00785794));
    return p > at<s32>(scoring, 0x614) ? at<s32>(scoring, 0x618) : 0;
}


s32 __fastcall AdvisorScoring_LatestCountBusStop(u8 *scoring) {
    const u8 *const counts = world_table() + 0x216d8;
    const s32 count = counts[0x20] ? at<s32>(counts, 0x1c) : (s32)(at<u32>(counts, 0x18) + 1);
    if (count <= 0)
        return 0;
    const s32 size = at<s32>(counts, 0x1c);
    if (size <= 0)
        return 0;
    s32 cursor = at<s32>(counts, 0x18);
    if (cursor < 0) {
        if (!counts[0x20])
            return 0;
        cursor += size;
    }
    if (at<s32>(counts, 4 + 4 * cursor) <= at<s32>(scoring, 0x410))
        return 0;
    u16 pos[2];
    Map_BusStopPos(game() + 0x2d8, nullptr, pos, 1);
    at<u32>(scoring, 8) = pos[0];
    return at<s32>(scoring, 0x414);
}

s32 __fastcall AdvisorScoring_StaffStat1f8NotAbove(const u8 *scoring) {
    if (Things_CountStaff() < at<s32>(scoring, 0x42c))
        return 0;
    const double average = Things_AverageStaffStat1f8(world_table(), nullptr, 0);
    return (double)at<s32>(scoring, 0x430) > average ? 0 : at<s32>(scoring, 0x434);
}

s32 __fastcall AdvisorScoring_PeopleStat19cNotAbove(const u8 *scoring) {
    if (Things_CountKind1OnPlainCell() < at<s32>(scoring, 0x438))
        return 0;
    const double average = Things_AveragePersonStat19c(world_table(), nullptr, 0);
    return (double)at<s32>(scoring, 0x43c) > average ? 0 : at<s32>(scoring, 0x440);
}

s32 __fastcall AdvisorScoring_RidesRatedAbove(u8 *scoring) {
    if (!Game_Field1da710IsZero())
        return 0;
    if (Things_CountByDef4ac(0, 0) < at<s32>(scoring, 0x41c))
        return 0;
    if (ParkAnalyser_AverageRatingDef4ac0(world_table(), nullptr, 10) <= at<s32>(scoring, 0x418))
        return 0;
    Things_BestRatedDef4ac0(reinterpret_cast<u16 *>(scoring + 4));
    return at<s32>(scoring, 0x420);
}

s32 __fastcall AdvisorScoring_StaffEntryClear(const u8 *scoring) {
    if (Things_CountStaff() <= at<s32>(scoring, 0x4d0))
        return 0;
    const u8 *const entry = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(game()));
    return entry[0x20] ? 0 : at<s32>(scoring, 0x4d4);
}

}  // extern "C"

namespace {

// 0x59e450-0x59e680's shape: not flagged; the staff of kind 5 present or not as asked; the most worn object's handle (+4) set; the
// objects' average wear at least the bound at lowAt and (highAt not 0) at most the one at highAt: the weight at weightAt times it.
s32 worn_objects(u8 *scoring, bool staff, u32 lowAt, u32 highAt, u32 weightAt) {
    if (flagged())
        return 0;
    const s32 cleaners = Things_CountStaffOfKind(5);
    if (staff ? cleaners == 0 : cleaners > 0)
        return 0;
    u16 handle;
    s32 value;
    const u16 most = *Things_MostKind3Flag1Of100Minus44(&handle, &value);
    at<u16>(scoring, 4) = most;
    if (!most)
        return 0;
    const s32 average = Things_AverageKind3Flag1Of100Minus44();
    if (average < at<s32>(scoring, lowAt) || (highAt && average > at<s32>(scoring, highAt)))
        return 0;
    return (s32)(at<u32>(scoring, weightAt) * (u32)average);
}

// 0x59e730 / 0x59e790's shape: the analyser's ring 0 full and its average (a byte) below (or above) the bound: the weight.
s32 ring_average(const u8 *scoring, bool above, u32 boundAt, u32 weightAt) {
    if (!ParkAnalyser_RingFull(world_table(), nullptr, 0))
        return 0;
    const s32 average = ParkAnalyser_RingAverage(world_table(), nullptr, 0);
    const s32 bound = at<s32>(scoring, boundAt);
    return (above ? average > bound : average < bound) ? at<s32>(scoring, weightAt) : 0;
}

}  // namespace

extern "C" {

s32 __fastcall AdvisorScoring_WornObjectsNoStaff5(u8 *scoring) { return worn_objects(scoring, false, 0x2d0, 0x2e0, 0x2d4); }

s32 __fastcall AdvisorScoring_WornObjectsStaff5(u8 *scoring) { return worn_objects(scoring, true, 0x2d8, 0x2e8, 0x2d4); }

s32 __fastcall AdvisorScoring_VeryWornObjectsNoStaff5(u8 *scoring) { return worn_objects(scoring, false, 0x2e0, 0, 0x2e4); }

s32 __fastcall AdvisorScoring_VeryWornObjectsStaff5(u8 *scoring) { return worn_objects(scoring, true, 0x2e8, 0, 0x2ec); }

s32 __fastcall AdvisorScoring_RingAverageBelow(const u8 *scoring) { return ring_average(scoring, false, 0x2f0, 0x2f4); }

s32 __fastcall AdvisorScoring_RingAverageAbove(const u8 *scoring) { return ring_average(scoring, true, 0x2f8, 0x2fc); }

}  // extern "C"

namespace {

AddrCalls g_calls;

// A ring of the table's: the average (the x87's _ftol of it, rounded in 24 bits) of its last `window` values, as many as it holds; `used` how many that was.
s32 history_average(const u8 *ring, s32 window, s32 &used) {
    const s32 cursor = at<s32>(ring, 0x244);
    const bool wrapped = at<u8>(ring, 0x24c) != 0;
    const s32 held = wrapped ? at<s32>(ring, 0x248) : cursor + 1;
    used = window > held ? held : window;
    X87Reg sum = 0.0;
    for (s32 i = 0; i < used; ++i) {
        const s32 size = at<s32>(ring, 0x248);
        if (i >= size) continue;
        if (i <= cursor)
            sum = sum + X87Reg((double)at<s32>(ring, 4 + 4 * (cursor - i)));
        else if (wrapped)
            sum = sum + X87Reg((double)at<s32>(ring, 4 + 4 * (cursor - i + size)));
    }
    return used > 0 ? (s32)(long long)(sum / X87Reg((double)used)).v : 0;   // (_ftol)
}

}  // namespace

void AdvisorScoring_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" {

s32 __fastcall AdvisorScoring_RingRatio(const u8 *scoring) {
    FileTime out;
    const FileTime *const elapsed = reinterpret_cast<const FileTime *>((uintptr_t)g_calls.Th(0x004f8800, (u32)(uintptr_t)(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec) + 0x2a0), &out));   // FunnyTime_Elapsed
    const long long time = (long long)(((unsigned long long)elapsed->high << 32) | elapsed->low);
    if ((s32)(time / 0x1792f8648000LL) < 3) return 0;
    const u8 *const rings = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c)));
    s32 used;
    const s32 first = history_average(rings + 0x1f7f0, at<s32>(scoring, 0x358), used);
    if (used == 0) return 0;
    const s32 second = history_average(rings + 0x1fc90, at<s32>(scoring, 0x358), used);
    if (!used) return 0;
    const X87Reg scaled = X87Reg((double)second) * at<float>(scoring, 0x354);
    return !(scaled.v >= (double)first) ? at<s32>(scoring, 0x35c) : 0;   // (FCOMPP's C0: below or unordered)
}

s32 __fastcall AdvisorScoring_AgedPerStaff(const u8 *scoring) {
    u8 *const game = **reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec);
    FileTime out;
    const FileTime *const elapsed = reinterpret_cast<const FileTime *>((uintptr_t)g_calls.Th(0x004f8800, (u32)(uintptr_t)(game + 0x2a0), &out));   // FunnyTime_Elapsed
    const long long time = (long long)(((unsigned long long)elapsed->high << 32) | elapsed->low);
    if ((s32)(time / 0x1792f8648000LL) < 3) return 0;
    const u32 count = g_calls.Th(0x004c9aa0, GameTable7cfb90_Get1da720_519510(game));
    const s32 staff = (s32)g_calls.Th(0x004c9a10, GameTable7cfb90_Get1da720_519510(game));   // Things_CountStaff
    if (!staff) return 0;
    return (s32)(count * 100) / staff > at<s32>(scoring, 0x4b8) ? at<s32>(scoring, 0x4bc) : 0;
}

namespace {

u8 *criterion(s32 index) { return reinterpret_cast<u8 *>((uintptr_t)0x0076e300 + (u32)index * 0x38); }

// The criterion of an id: its place in the table when that one holds the id or any other does (sic: the address by the id all the same); the two out of range or unknown, none.
const u8 *find_criterion(s32 id) {
    if (id == 0x160 || id == 0x15f) return nullptr;
    if (at<s32>(criterion(id), 4) == id) return criterion(id);
    for (u32 entry = 0x0076e304; entry < 0x00772fcc; entry += 0x38)
        if (*reinterpret_cast<const s32 *>((uintptr_t)entry) == id) return criterion(id);
    return nullptr;
}

// The state the game object is in, its function-local static (guard byte 0xfb34bc, bit 0) made at first use.
s32 game_state() {
    u8 &guard = gQuitconfirm_fb34bc;
    if (!(guard & 1)) {
        guard |= 1;
        g_calls.Th(0x00550ca0, reinterpret_cast<uintptr_t>(&gDialog74fb20_fb3b7c));
        g_calls.Cd(0x0067a7d0, 0x005d1750u);   // atexit
    }
    return *reinterpret_cast<const s32 *>(&gDialog74fb20_fb3b7c);
}

// Whether a criterion of this kind runs: kind 2 always, 0 with the state 0, 1 with the state 2.
bool kind_runs(const u8 *found) {
    if (!found) return false;
    if (at<s32>(found, 0xc) == 2) return true;
    if (at<s32>(found, 0xc) == 0 && game_state() == 0) return true;
    return at<s32>(found, 0xc) == 1 && game_state() == 2;
}

}  // namespace

s32 __fastcall AdvisorScoring_PickNext(s32 *cursor, void *, s32 steps) {
    s32 best = 0, chosen = 0x160;
    for (s32 step = 0; step < steps; ++*cursor) {
        if (*cursor >= 0x15f) *cursor = 0;
        if (at<u32>(criterion(*cursor), 0)) continue;   // (a step not counted)
        if (kind_runs(find_criterion(at<s32>(criterion(*cursor), 4)))) {
            using Score = s32(__fastcall *)(u8 *, void *);
            const u8 *const entry = criterion(*cursor);
            const s32 score = at<Score>(entry, 0x10)(reinterpret_cast<u8 *>(cursor) + at<u32>(entry, 0x14), nullptr);
            if (score > best) {
                const u32 id = at<u32>(criterion(*cursor), 4);
                const u32 record = g_calls.Th(0x005194d0, **reinterpret_cast<u32 **>((uintptr_t)0x007cf6ec), id, 0u);
                if ((u8)g_calls.Th(0x0059abc0, record)) {
                    best = score;
                    chosen = at<s32>(criterion(*cursor), 4);
                }
            }
        }
        ++step;
    }
    return chosen;
}

}  // extern "C"

extern "C" s32 __fastcall AdvisorScoring_RingDrop(const u8 *scoring) {
    FileTime out;
    const FileTime *const elapsed = reinterpret_cast<const FileTime *>((uintptr_t)g_calls.Th(0x004f8800, (u32)(uintptr_t)(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec) + 0x2a0), &out));   // FunnyTime_Elapsed
    const long long time = (long long)(((unsigned long long)elapsed->high << 32) | elapsed->low);
    const s32 months = (s32)(time / 0x1792f8648000LL);
    const u8 *const ring = reinterpret_cast<const u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x007cf83c))) + 0x216d8;
    const bool wrapped = at<u8>(ring, 0x20) != 0;
    const s32 size = at<s32>(ring, 0x1c), cursor = at<s32>(ring, 0x18);
    if ((wrapped ? size : cursor) + 1 <= 1 || months >= at<s32>(scoring, 0x3c8)) return 0;
    if (size <= 0) return 0;
    s32 latest, before;
    if (cursor >= 0)
        latest = at<s32>(ring, 4 + 4 * cursor);
    else if (wrapped)
        latest = at<s32>(ring, 4 + 4 * (cursor + size));
    else
        return 0;
    if (size <= 1) return 0;
    if (cursor >= 1)
        before = at<s32>(ring, 4 * cursor);
    else if (wrapped)
        before = at<s32>(ring, 4 * (cursor + size));
    else
        return 0;
    if (!before || latest >= before) return 0;
    const s32 drop = 100 - (s32)((u32)latest * 100u) / before;
    return drop > at<s32>(scoring, 0x3cc) ? at<s32>(scoring, 0x3d0) : 0;
}

namespace {

// The game table entries of the two game pointers, for the calls through g_calls.
u8 *game2() { return *reinterpret_cast<u8 **>((uintptr_t)0x007cf83c); }
u32 park() { return (u32)(uintptr_t)table(); }
u32 park2() { return (u32)GameTable7cfb90_Get1da720_519510(game2()); }
u32 funds() { return (u32)GameTable7cfb90_Get1da726_5195d0(game()); }
u32 funds2() { return (u32)GameTable7cfb90_Get1da726_5195d0(game2()); }
s32 staff_count(u32 kind) { return (s32)g_calls.Th(0x004c9960, park(), kind); }   // Things_CountStaffOfKind
s32 busy() { return (s32)g_calls.Th(0x004c81b0, park(), 0u); }   // ParkAnalyser_CountBusy
s32 with_patrol(u32 kind) { return (s32)g_calls.Th(0x004c9b40, park(), kind); }   // Things_CountStaffWithPatrol
s32 coverage(u32 kind, u32 part) { return (s32)g_calls.Th(0x004cd500, park2(), kind, part); }   // PatrolCoverage_Percent, of [0x7cf83c]'s
s32 days_21c38() { return (s32)g_calls.Th(0x004c90d0, park()); }   // ParkAnalyser_DaysSince21c38
s32 days_21c30() { return (s32)g_calls.Th(0x004c9070, park()); }   // ParkAnalyser_DaysSince21c30
s32 by_def(u32 kind) { return (s32)g_calls.Th(0x004ca8e0, park(), kind, 0u); }   // Things_CountByDef4ac
u32 with_controls(u32 kind, u32 second = 0) { return g_calls.Th(0x004ca7b0, park(), kind, second); }   // ObjectDefs_CountKindWithControls

// The park's age (FunnyTime_Elapsed of the game's +0x2a0) over one unit of it, signed and truncated.
s32 age_in(long long unit) {
    FileTime out;
    const FileTime *const elapsed = reinterpret_cast<const FileTime *>((uintptr_t)g_calls.Th(0x004f8800, (u32)(uintptr_t)(game() + 0x2a0), &out));
    return (s32)((long long)(((unsigned long long)elapsed->high << 32) | elapsed->low) / unit);
}
constexpr long long kMonth = 0x1792f8648000LL, kDay = 0xc92a69c000LL;

// From the month +months with +atLeast staff of the kind: (100 - the share with a patrol, per cent, signed) * +weight, unless the share is over +atMost.
s32 patrol_share(const u8 *scoring, u32 kind, u32 months, u32 atLeast, u32 atMost, u32 weight) {
    if (age_in(kMonth) < at<s32>(scoring, months)) return 0;
    const s32 staff = staff_count(kind);
    if (staff < at<s32>(scoring, atLeast)) return 0;
    const s32 share = (s32)((u32)with_patrol(kind) * 100u) / staff;
    if (share > at<s32>(scoring, atMost)) return 0;
    return (s32)((u32)(100 - share) * at<u32>(scoring, weight));
}

// (100 - the coverage) * +weight, or 0 under +floor (signed).
s32 uncovered(const u8 *scoring, u32 kind, u32 part, u32 floor, u32 weight) {
    const s32 score = (s32)((u32)(100 - coverage(kind, part)) * at<u32>(scoring, weight));
    return score < at<s32>(scoring, floor) ? 0 : score;
}

// From the month +months with staff of the kind: uncovered().
s32 patrol_cover(const u8 *scoring, u32 kind, u32 part, u32 months, u32 floor, u32 weight) {
    if (age_in(kMonth) < at<s32>(scoring, months)) return 0;
    if (!staff_count(kind)) return 0;
    return uncovered(scoring, kind, part, floor, weight);
}

// With definitions of the research (count: its call) and +0x634 days since 0x21c30, +atLeast things of the kind: +result when the share of its definitions with controls is under +below.
s32 research_share(const u8 *scoring, u32 count, u32 kind, u32 below, u32 atLeast, u32 result) {
    const u32 all = g_calls.Th(count, park(), 0u);
    if (!all || days_21c30() < at<s32>(scoring, 0x634) || by_def(kind) < at<s32>(scoring, atLeast)) return 0;
    return with_controls(kind) * 100u / all < at<u32>(scoring, below) ? at<s32>(scoring, result) : 0;
}

// With definitions of the kind with controls and none of the next kind's, +0x634 days since 0x21c30: the park's age in days * +rate + +base.
s32 research_waiting(const u8 *scoring, u32 kind, u32 base, u32 rate) {
    if (!with_controls(kind) || days_21c30() < at<s32>(scoring, 0x634) || with_controls(kind + 1)) return 0;
    return (s32)((u32)age_in(kDay) * at<u32>(scoring, rate) + at<u32>(scoring, base));
}

// In the red (Source_Field0c negative) with no loan offer: the months of it (Money_TimeInRed of [0x7cf83c]'s money), signed and truncated, in `months`.
bool months_in_red(s32 &months) {
    if ((s32)g_calls.Th(0x006ad810, funds()) >= 0) return false;
    if (g_calls.Th(0x004d0810, funds2())) return false;
    FileTime out;
    const FileTime *const time = reinterpret_cast<const FileTime *>((uintptr_t)g_calls.Th(0x004d0260, funds2(), &out));
    months = (s32)((long long)(((unsigned long long)time->high << 32) | time->low) / kMonth);
    return true;
}

}  // namespace

extern "C" {

s32 __fastcall AdvisorScoring_ObjectsOverStaff4(const u8 *scoring) {
    if (days_21c38() < at<s32>(scoring, 0x630)) return 0;
    const u32 covered = (u32)staff_count(4) * at<u32>(scoring, 0xc8);
    const s32 over = (s32)((u32)by_def(0) - covered);
    return over > 0 ? (s32)(at<u32>(scoring, 0xcc) * (u32)over) : 0;
}

s32 __fastcall AdvisorScoring_BusyOverStaff5(const u8 *scoring) {
    if (flagged() || !staff_count(5)) return 0;
    const s32 people = busy();
    if (people < at<s32>(scoring, 0xec)) return 0;
    const s32 over = (s32)((u32)people - (u32)staff_count(5) * at<u32>(scoring, 0xe4));
    return over > 0 ? (s32)(at<u32>(scoring, 0xe8) * (u32)over) : 0;
}

s32 __fastcall AdvisorScoring_BusyOverStaff7(const u8 *scoring) {
    if (flagged() || days_21c38() < at<s32>(scoring, 0x630)) return 0;
    const s32 people = busy();
    if (people < at<s32>(scoring, 0x108)) return 0;
    const s32 over = (s32)((u32)people - (u32)staff_count(7) * at<u32>(scoring, 0x100));
    return over > 0 ? (s32)(at<u32>(scoring, 0x104) * (u32)over) : 0;
}

s32 __fastcall AdvisorScoring_ChainShareStaff8(const u8 *scoring) {
    if (flagged()) return 0;
    const u32 scale = g_calls.Th(0x004c9ed0, park());   // Global7852f8_Times78548c
    if (!scale || age_in(kMonth) < at<s32>(scoring, 0x120) || !staff_count(8)) return 0;
    if (!g_calls.Th(0x004c9e40, park(), 4u)) return 0;   // World_StaffCountOfType_4c9e40
    const u32 share = g_calls.Th(0x004c9e60, park()) * 100u / scale;   // Things_SumChain
    return share < at<u32>(scoring, 0x124) ? at<s32>(scoring, 0x128) : 0;
}

s32 __fastcall AdvisorScoring_PatrolCover4(const u8 *scoring) {
    if (age_in(kMonth) < at<s32>(scoring, 0x138) || staff_count(4) < at<s32>(scoring, 0x12c) || !with_patrol(4)) return 0;
    return uncovered(scoring, 4, 2, 0x13c, 0x140);
}

s32 __fastcall AdvisorScoring_PatrolShare5(const u8 *scoring) { return patrol_share(scoring, 5, 0x150, 0x144, 0x148, 0x14c); }
s32 __fastcall AdvisorScoring_PatrolCover5(const u8 *scoring) { return patrol_cover(scoring, 5, 3, 0x150, 0x154, 0x158); }
s32 __fastcall AdvisorScoring_PatrolShare7(const u8 *scoring) { return patrol_share(scoring, 7, 0x170, 0x164, 0x168, 0x16c); }
s32 __fastcall AdvisorScoring_PatrolShare6(const u8 *scoring) { return patrol_share(scoring, 6, 0x188, 0x17c, 0x180, 0x184); }
s32 __fastcall AdvisorScoring_PatrolCover6(const u8 *scoring) { return patrol_cover(scoring, 6, 1, 0x188, 0x18c, 0x190); }

s32 __fastcall AdvisorScoring_ResearchShare0(const u8 *scoring) { return research_share(scoring, 0x004ca450, 0, 0x220, 0x224, 0x228); }

s32 __fastcall AdvisorScoring_ResearchNone0(const u8 *scoring) {
    if (with_controls(0, 1)) return 0;
    return (s32)((u32)age_in(kDay) * at<u32>(scoring, 0x234) + at<u32>(scoring, 0x230));
}

s32 __fastcall AdvisorScoring_ResearchIdleStaff8(const u8 *scoring) {
    if (age_in(kMonth) < at<s32>(scoring, 0x238) || !staff_count(8)) return 0;
    if ((s32)g_calls.Th(0x004ca4c0, park(), 0u) > 0) return 0;   // ObjectDefs_CountKind0WithControls14
    if (g_calls.Th(0x00503980, (u32)(uintptr_t)lab(), 4u)) return 0;   // Table_ScaledBySum
    return at<s32>(scoring, 0x23c);
}

s32 __fastcall AdvisorScoring_ResearchShare1(const u8 *scoring) { return research_share(scoring, 0x004ca510, 1, 0x258, 0x25c, 0x260); }
s32 __fastcall AdvisorScoring_ResearchWaiting0(const u8 *scoring) { return research_waiting(scoring, 0, 0x268, 0x26c); }
s32 __fastcall AdvisorScoring_ResearchShare2(const u8 *scoring) { return research_share(scoring, 0x004ca570, 2, 0x27c, 0x280, 0x284); }
s32 __fastcall AdvisorScoring_ResearchWaiting1(const u8 *scoring) { return research_waiting(scoring, 1, 0x28c, 0x290); }
s32 __fastcall AdvisorScoring_ResearchShare3(const u8 *scoring) { return research_share(scoring, 0x004ca5d0, 3, 0x2a0, 0x2a4, 0x2a8); }
s32 __fastcall AdvisorScoring_ResearchWaiting2(const u8 *scoring) { return research_waiting(scoring, 2, 0x2b0, 0x2b4); }

s32 __fastcall AdvisorScoring_MoneyOverLoanless(const u8 *scoring) {
    if ((s32)g_calls.Th(0x006ad810, funds()) <= at<s32>(scoring, 0x330)) return 0;
    return g_calls.Th(0x004d0690, funds2()) ? 0 : at<s32>(scoring, 0x334);   // Obj_Count28s_4d0690
}

s32 __fastcall AdvisorScoring_RedUnder3(const u8 *scoring) {
    s32 months;
    return months_in_red(months) && months < 3 ? at<s32>(scoring, 0x344) : 0;
}

s32 __fastcall AdvisorScoring_Red3To4(const u8 *scoring) {
    s32 months;
    return months_in_red(months) && months >= 3 && months < 5 ? at<s32>(scoring, 0x348) : 0;
}

s32 __fastcall AdvisorScoring_Red5(const u8 *scoring) {
    s32 months;
    return months_in_red(months) && months == 5 ? at<s32>(scoring, 0x34c) : 0;
}

s32 __fastcall AdvisorScoring_DaysBetween1(const u8 *scoring) {
    const s32 days = age_in(kDay);
    return days > at<s32>(scoring, 0x5dc) && days < at<s32>(scoring, 0x5e0) ? at<s32>(scoring, 0x5e4) : 0;
}

s32 __fastcall AdvisorScoring_DaysBetween2(const u8 *scoring) {
    const s32 days = age_in(kDay);
    return days > at<s32>(scoring, 0x5e8) && days < at<s32>(scoring, 0x5ec) ? at<s32>(scoring, 0x5f0) : 0;
}

s32 __fastcall AdvisorScoring_TotalNoExtra(const u8 *scoring) {
    if (ObjectUnlock_ExtraCount() > 0) return 0;
    const s32 total = (s32)ParkAnalyser_Total(table());
    return total > scaled(0x00785790, at<float>(scoring, 0x5b8)) ? at<s32>(scoring, 0x5bc) : 0;
}

s32 __fastcall AdvisorScoring_MeanQueueUnused(const u8 *) {
    if (!flagged()) {
        (void)table();
        (void)Objects_MeanQueuePosition();
    }
    return 0;
}
}
