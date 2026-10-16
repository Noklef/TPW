#include "thingscan.h"

#include <cstdint>

#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

void *world() { return *reinterpret_cast<void **>(0x0080239c); }
const u8 *thing_of(u16 handle) { return static_cast<const u8 *>(reinterpret_cast<ThingSlot *>(0x007cfb90)[handle].thing); }

// The count of things of kind 3 with the flag bit(s) set and the dword at +0x68 zero.
s32 CountKind3(u8 flag) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && (at<u8>(thing, 0x32) & flag) && at<u32>(thing, 0x68) == 0)
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

}  // namespace

extern "C" {

s32 __cdecl Things_CountKind3Flag1() { return CountKind3(1); }

s32 __cdecl Things_CountKind3Flag2() { return CountKind3(2); }

s32 __stdcall Things_MaxKind3Byte50(u32 value) {
    s32 best = -1;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && at<u16>(thing, 0xe) == (u16)value && best < (s32)at<u8>(thing, 0x50))
            best = at<u8>(thing, 0x50);
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return best;
}

}  // extern "C"
