#include "advisorscoring.h"

#include <cstdint>

#include "../world/thingscan.h"
#include "../world/world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *table() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))); }

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
