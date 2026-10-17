#include "namedobjects.h"

#include <cstdint>

#include "string.h"
#include "vobj3.h"

namespace {

constexpr u32 kNames[] = {0x00786b68, 0x00785e78, 0x00785f98, 0x007862c8, 0x00785f68, 0x007862f8, 0x00786d10,
                          0x007866e8, 0x007862e0, 0x00785ed8, 0x00785f80, 0x00786aa8, 0x00786040, 0x007867d8};

u8 **registry() { return *reinterpret_cast<u8 ***>((uintptr_t)0x00786b9c); }

}  // namespace

extern "C" {

u32 __cdecl NamedObjects_RemoveGlobals() {
    u32 result = 0;
    for (const u32 name : kNames) {
        u8 *object;
        result = NamedList_Find(registry(), nullptr, reinterpret_cast<const u8 *>((uintptr_t)name), &object);
        if (result == 1) result = ListRef_Remove(registry(), nullptr, object);
    }
    return result;
}
}
