#include "advisorscoring.h"

#include <cstdint>

#include "../world/thingscan.h"
#include "../world/world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *table() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))); }

// +0xc..+0x1c of the game table entry for +0x1da718 of [[0x7cf6ec]]: the dword of a staff kind.
s32 entry_dword(u32 offset) {
    return at<s32>(reinterpret_cast<void *>((uintptr_t)GameTable7cfb90_Get1da718_519450(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))), offset);
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
}  // extern "C"
