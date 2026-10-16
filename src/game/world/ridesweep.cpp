#include "ridesweep.h"

#include <cstdint>

#include "object.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 DefWord(const u8 *thing) {
    u16 word;
    Object_DefWord284(thing, nullptr, &word);
    return word;
}

template <typename Test> s32 Sweep(ObjectCallback callback, Test test) {
    u8 *world = global<u8 *>(0x007cf83c);
    s32 count = 0;
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle != 0) {
        u8 *thing = reinterpret_cast<u8 **>(0x007cfb90)[handle * 5];
        if (at<u8>(thing, 2) == 3 && test(thing)) {
            callback(thing);
            ++count;
        }
        u16 next;
        handle = *World_NextThing(world, nullptr, &next, thing);
    }
    return count;
}

bool Kind3(const u8 *thing) { return DefWord(thing) == 0 && at<u32>(Object_GetDef(thing), 0x4ac) == 3; }

}  // namespace

extern "C" {

s32 __cdecl Objects_ForEachOfKind(u32 kind, ObjectCallback callback) {
    return Sweep(callback, [kind](const u8 *thing) { return DefWord(thing) == 0 && at<u32>(Object_GetDef(thing), 0x4ac) == kind; });
}

s32 __cdecl Objects_ForEachWordSet(ObjectCallback callback) {
    return Sweep(callback, [](const u8 *thing) { return DefWord(thing) != 0; });
}

s32 __cdecl Objects_ForEachUpgrade2(ObjectCallback callback) {
    return Sweep(callback, [](const u8 *thing) { return DefWord(thing) != 0 && at<u32>(Object_GetDef(thing), 0x288) == 2; });
}

s32 __cdecl Objects_ForEachKind3Flag1(ObjectCallback callback) {
    return Sweep(callback, [](const u8 *thing) { return Kind3(thing) && (at<u8>(thing, 0x32) & 1) != 0; });
}

s32 __cdecl Objects_ForEachKind3NoFlags(ObjectCallback callback) {
    return Sweep(callback, [](const u8 *thing) { return Kind3(thing) && (at<u8>(thing, 0x32) & 3) == 0; });
}

s32 __cdecl Objects_ForEachKind3Flag2(ObjectCallback callback) {
    return Sweep(callback, [](const u8 *thing) { return Kind3(thing) && (at<u8>(thing, 0x32) & 2) != 0; });
}

}  // extern "C"
