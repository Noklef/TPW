#include "ridesweep.h"

#include <cstdint>

#include "object.h"
#include "staff.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 DefWord(const u8 *thing) {
    u16 word;
    Object_DefWord284(thing, nullptr, &word);
    return word;
}

// The things of the table newest first (the world at [0x7cf83c]) that pass the test, the callback called for each; how many.
template <typename Test> s32 Walk(ObjectCallback callback, Test test) {
    u8 *world = global<u8 *>(0x007cf83c);
    s32 count = 0;
    u16 handle;
    World_FirstThing(world, nullptr, &handle);
    while (handle != 0) {
        u8 *thing = reinterpret_cast<u8 **>(0x007cfb90)[handle * 5];
        if (test(thing)) {
            callback(thing);
            ++count;
        }
        u16 next;
        handle = *World_NextThing(world, nullptr, &next, thing);
    }
    return count;
}

template <typename Test> s32 Sweep(ObjectCallback callback, Test test) {
    return Walk(callback, [&test](const u8 *thing) { return at<u8>(thing, 2) == 3 && test(thing); });
}

// A staff list: from the head First writes, the things chained through the word at +0x210; the callback called for each; how many.
s32 StaffList(u16 *(__fastcall *first)(const u8 *, void *, u16 *), ObjectCallback callback) {
    s32 count = 0;
    u16 handle;
    first(global<u8 *>(0x007cf83c), nullptr, &handle);
    while (handle != 0) {
        u8 *const thing = Thing_ByHandle(handle);
        callback(thing);
        ++count;
        handle = at<u16>(thing, 0x210);
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

s32 __cdecl Things_ForEachSelected(u32 mask, ObjectCallback callback) {
    s32 count = 0;
    if (mask & 1) count += Walk(callback, [](const u8 *thing) { return Thing_IsStaff(thing) != 0; });
    if (mask & 2) count += StaffList(World_FirstHandyman, callback);
    if (mask & 4) count += StaffList(World_FirstMechanic, callback);
    if (mask & 8) count += StaffList(World_FirstGuard, callback);
    if (mask & 0x10) count += StaffList(World_FirstEntertainer, callback);
    if (mask & 0x20) count += StaffList(World_FirstResearcher, callback);
    if (mask & 0x40) count += Walk(callback, [](const u8 *thing) { return at<u8>(thing, 2) == 1; });
    if (mask & 0x80) count += Sweep(callback, [](const u8 *thing) { return DefWord(thing) == 0 && at<u32>(Object_GetDef(thing), 0x4ac) == 0; });
    if (mask & 0x100) count += Objects_ForEachOfKind(2, callback);
    if (mask & 0x200) count += Objects_ForEachOfKind(1, callback);
    if (mask & 0x400) count += Objects_ForEachOfKind(3, callback);
    if (mask & 0x800) count += Objects_ForEachKind3NoFlags(callback);
    if (mask & 0x1000) count += Objects_ForEachKind3Flag1(callback);
    if (mask & 0x2000) count += Objects_ForEachWordSet(callback);
    if (mask & 0x4000) count += Objects_ForEachUpgrade2(callback);
    if (mask & 0x8000) count += Objects_ForEachKind3Flag2(callback);
    return count;
}

}  // extern "C"
