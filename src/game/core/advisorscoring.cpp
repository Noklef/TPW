#include "advisorscoring.h"

#include <cstdint>

#include "../world/thingscan.h"
#include "../world/world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *table() { return reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec))); }
}  // namespace

extern "C" {

s32 __fastcall AdvisorScoring_PeopleAbove1a4(u8 *scoring) {
    return Things_CountPeopleStat1a4Above(table(), nullptr, at<u8>(scoring, 0xb0)) * at<s32>(scoring, 0xac);
}

s32 __fastcall AdvisorScoring_PeopleAbove1a8(u8 *scoring) {
    return Things_CountPeopleStat1a8Above(table(), nullptr, at<u8>(scoring, 0xb8)) * at<s32>(scoring, 0xb4);
}
}  // extern "C"
