#include "thingscan.h"

#include <cstdint>

#include "../core/indexed.h"
#include "analysercell.h"
#include "object.h"
#include "objectdefs.h"
#include "things.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

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

// The count of things of kind 4 to 8.
s32 CountKinds4To8() {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        const u8 kind = at<u8>(thing, 2);
        if (kind == 4 || kind == 5 || kind == 6 || kind == 8 || kind == 7)
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

template <typename Extra> s32 CountKind1OnPlainCell(Extra extra) {
    s32 count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 1 && AnalyserCell_TypePlain(thing) && extra(thing))
            ++count;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return count;
}

// Takes a thing out of the list of things headed by the word at game + headOffset and linked by the word at +linkOffset of each.
void UnlinkThing(u8 *game, const u8 *thing, u32 headOffset, u32 linkOffset) {
    const u16 handle = at<u16>(thing, 0);
    auto link = [&](u16 h) -> u16 & { return at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)h), linkOffset); };
    if (at<u16>(game, headOffset) == handle) {
        at<u16>(game, headOffset) = link(handle);
        return;
    }
    u16 before = at<u16>(game, headOffset);
    u16 next = link(before);
    while (next != handle) {
        before = next;
        next = link(before);
    }
    link(before) = link(handle);
}

}  // namespace

extern "C" {

void __fastcall Things_Unlink1da73c(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da73c, 0x210); }
void __fastcall Things_Unlink1da73e(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da73e, 0x210); }
void __fastcall Things_Unlink1da740(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da740, 0x210); }
void __fastcall Things_Unlink1da742(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da742, 0x210); }
void __fastcall Things_Unlink1da744(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da744, 0x210); }
void __fastcall Things_Unlink1da746(u8 *game, void *, const u8 *thing) { UnlinkThing(game, thing, 0x1da746, 0xc); }

u32 __cdecl Things_SumChain() {
    u32 sum = 0;
    u32 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da742);
    while (handle) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * handle);
        sum += global<u32>(0x0078545c + 12 * at<u32>(thing, 0x1e4));
        handle = at<u16>(thing, 0x210);
    }
    return sum;
}

s32 __cdecl Things_CountKind3Flag1() { return CountKind3(1); }

s32 __cdecl Things_CountStaff() { return CountKinds4To8(); }

void __cdecl Things_TopThreeValues(u8 *first, u8 *second, u8 *third) {
    s32 counts[23] = {};
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 1 && AnalyserCell_TypePlain(thing))
            ++counts[at<s32>(thing, 0x30)];  // (sic: not checked against the 23)
        World_NextThing(world(), nullptr, &handle, thing);
    }
    counts[0] = 0;
    u8 best[3] = {0, 1, 2};
    for (s32 k = 3; k < 0x17; ++k) {
        s32 gain = 0, at_index = -1;
        for (s32 j = 0; j < 3; ++j) {
            if (counts[k] - counts[best[j]] > gain) {
                gain = counts[k] - counts[best[j]];
                at_index = j;
            }
        }
        if (at_index >= 0) best[at_index] = (u8)k;
    }
    *first = counts[best[0]] == 0 ? 0x17 : best[0];
    *second = counts[best[1]] == 0 ? 0x17 : best[1];
    *third = counts[best[2]] == 0 ? 0x17 : best[2];
}

s32 __cdecl Things_CountOfKindAround(u32 kind, u32 cell, s32 radius, s32 firstOnly) {
    const u32 index = (cell & 0xffff) - 1;
    const s32 x = (s32)(index & 0x7f), y = (s32)(index >> 7);
    const u32 xmin = (u32)(x - radius < 0 ? 0 : x - radius), ymin = (u32)(y - radius < 0 ? 0 : y - radius);
    const u32 xmax = (u32)(x + radius > 0x7f ? 0x7f : x + radius), ymax = (u32)(y + radius > 0x7f ? 0x7f : y + radius);
    s32 count = 0;
    for (u32 cx = xmin; cx <= xmax; ++cx) {
        for (u32 cy = ymin; cy <= ymax; ++cy) {
            const u32 id = ((cy << 7) + cx + 1) & 0xffff;
            u32 handle = at<u16>(global<u8 *>(0x008023a0), id * 0x44 - 0x20);
            while (handle) {
                const u8 *thing = thing_of((u16)handle);
                if (at<u8>(thing, 2) == kind) {
                    if (firstOnly) return 1;
                    ++count;
                }
                handle = at<u16>(thing, 10);
            }
        }
    }
    return count;
}

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

s32 __cdecl Things_CountKind1OnPlainCell() {
    return CountKind1OnPlainCell([](const u8 *) { return true; });
}

s32 __cdecl Things_CountKind1OnPlainCellWith210() {
    return CountKind1OnPlainCell([](const u8 *t) { return at<u32>(t, 0x210) != 0; });
}

s32 __cdecl Things_CountKind1OnPlainCellMode2() {
    return CountKind1OnPlainCell([](const u8 *t) { return at<s32>(t, 0x24) == 2; });
}

namespace {

u32 Surplus(const u8 *object, u32 limitAt, u32 kind) {
    const s32 limit = Things_CountByDef4ac(0, 0) + at<s32>(object, limitAt);
    return Things_CountStaffOfKind(kind) > limit ? at<u32>(object, limitAt + 4) : 0;
}

}  // namespace

u32 __fastcall Staff_SurplusKind5(const u8 *object) { return Surplus(object, 0x360, 5); }

u32 __fastcall Staff_SurplusKind6(const u8 *object) { return Surplus(object, 0x368, 6); }

u32 __fastcall Staff_SurplusKind4(const u8 *object) { return Surplus(object, 0x370, 4); }

u32 __fastcall Staff_SurplusKind7(const u8 *object) { return Surplus(object, 0x378, 7); }

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
