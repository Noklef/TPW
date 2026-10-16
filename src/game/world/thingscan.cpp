#include "thingscan.h"

#include <cstdint>

#include "../core/indexed.h"
#include "object.h"
#include "objectdefs.h"
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

// The first thing after the previous one (the newest for none) that the test accepts, 0 for none.
template <typename Test> u8 *FindFrom(const u8 *previous, Test test) {
    u16 handle;
    if (!previous)
        World_FirstThing(world(), nullptr, &handle);
    else
        World_NextThing(world(), nullptr, &handle, previous);
    const u8 *found = nullptr;
    while (handle && !found) {
        const u8 *thing = thing_of(handle);
        if (test(thing))
            found = thing;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return const_cast<u8 *>(found);
}

// The first thing going the other way, from the one before the previous (the oldest for none), that the test accepts, 0 for none.
template <typename Test> u8 *FindBackFrom(const u8 *previous, Test test) {
    u16 handle;
    if (!previous) {
        World_FirstThing(world(), nullptr, &handle);
        const u8 *last = nullptr;
        while (handle) {
            last = thing_of(handle);
            World_NextThing(world(), nullptr, &handle, last);
        }
        Thing_GetHandle(last, nullptr, &handle);
    } else {
        World_PrevThing(world(), nullptr, &handle, previous);
    }
    const u8 *found = nullptr;
    while (handle && !found) {
        const u8 *thing = thing_of(handle);
        if (test(thing))
            found = thing;
        World_PrevThing(world(), nullptr, &handle, thing);
    }
    return const_cast<u8 *>(found);
}

u16 DefWord(const u8 *thing) {
    u16 word;
    Object_DefWord284(thing, nullptr, &word);
    return word;
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

u8 *__cdecl Things_FindNextByDef4ac(s32 value, const u8 *previous) {
    return FindFrom(previous, [value](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) == 0 && (s32)at<u32>(Object_GetDef(t), 0x4ac) == value; });
}

u8 *__cdecl Things_FindNextWithDefWord(const u8 *previous) {
    return FindFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) != 0; });
}

u8 *__cdecl Things_FindNextWithDefWordUpgrade2(const u8 *previous) {
    return FindFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) != 0 && at<u32>(Object_GetDef(t), 0x288) == 2; });
}

u8 *__cdecl Things_FindNextShopFlag1(const u8 *previous) {
    return FindFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) == 0 && at<u32>(Object_GetDef(t), 0x4ac) == 3 && (at<u8>(t, 0x32) & 1); });
}

u8 *__cdecl Things_FindNextShopNoFlags(const u8 *previous) {
    return FindFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) == 0 && at<u32>(Object_GetDef(t), 0x4ac) == 3 && !(at<u8>(t, 0x32) & 3); });
}

u8 *__cdecl Things_FindNextKind3Flag2(const u8 *previous) {
    return FindFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && (at<u8>(t, 0x32) & 2); });
}

u8 *__cdecl Things_FindPrevByDef4ac(s32 value, const u8 *previous) {
    return FindBackFrom(previous, [value](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) == 0 && (s32)at<u32>(Object_GetDef(t), 0x4ac) == value; });
}

u8 *__cdecl Things_FindPrevWithDefWord(const u8 *previous) {
    return FindBackFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) != 0; });
}

u8 *__cdecl Things_FindPrevWithDefWordUpgrade2(const u8 *previous) {
    return FindBackFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && DefWord(t) != 0 && at<u32>(Object_GetDef(t), 0x288) == 2; });
}

u8 *__cdecl Things_FindPrevKind3Flag2(const u8 *previous) {
    return FindBackFrom(previous, [](const u8 *t) { return at<u8>(t, 2) == 3 && (at<u8>(t, 0x32) & 2); });
}

u16 *__cdecl Things_LastHandle(u16 *out) {
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    const u8 *last = nullptr;
    while (handle) {
        last = thing_of(handle);
        World_NextThing(world(), nullptr, &handle, last);
    }
    Thing_GetHandle(last, nullptr, out);
    return out;
}

s32 __cdecl Things_CountKind3Flag2Any() {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && (at<u8>(thing, 0x32) & 2))
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

s32 __stdcall Things_CountStaffOfKind(u32 kind) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        const u8 k = at<u8>(thing, 2);
        if (k >= 4 && k <= 8 && k == kind && at<u8>(thing, 3) == 0)
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

s32 __stdcall Things_AverageOfKind(u32 kind) {
    s32 sum = 0;
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == kind) {
            ++count;
            sum = (s32)((u32)sum + at<u32>(thing, 0x1e4));
        }
        World_NextThing(world(), nullptr, &handle, thing);
    }
    if (count == 0)
        return 0;
    return (s32)((u32)sum * 100) / (count * 5);
}

u32 __fastcall Game_RecountKind3Flag1(u8 *game) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && (at<u8>(thing, 0x32) & 1))
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    at<s32>(game, 0x1f0e0) = count;
    at<u8>(game, 0x1f0e4) = 1;
    Out_SetFromGame1da70c(reinterpret_cast<u32 *>(game + 0x1f0dc));
    return at<u32>(game, 0x1f0e0);
}

s32 __stdcall Things_CountByDef4ac(s32 value, s32 least) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && (s32)at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x4ac) == value &&
            (value != 0 || (s32)at<u8>(thing, 0x50) >= least))
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

s32 __stdcall Things_CountByDef4acIdle(s32 value) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && (s32)at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x4ac) == value &&
            at<u32>(thing, 0x68) == 0)
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

}  // extern "C"
