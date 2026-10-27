#include "advisorscoring.h"

#include <cstdint>

#include "../math/x87.h"
#include "../world/analyser.h"
#include "../world/money.h"
#include "../world/object.h"
#include "../world/objectdefs.h"
#include "../world/research.h"
#include "../world/staff.h"
#include "../world/thingscan.h"
#include "../world/world.h"
#include "accessors.h"
#include "globalsave.h"

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

}  // extern "C"
