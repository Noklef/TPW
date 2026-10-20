#include "thingscan.h"

#include <cstdint>
#include <cstring>

#include "../coaster/registry.h"
#include "../core/accessors.h"
#include "../core/circbuf.h"
#include "../core/indexed.h"
#include "../math/x87.h"
#include "analysercell.h"
#include "mapcell.h"
#include "object.h"
#include "objectdefs.h"
#include "person.h"
#include "things.h"
#include "ridelist.h"
#include "staff.h"

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

// The squared distance in cells between two things (their column and row bytes at +5 and +7).
u32 CellDistanceSquared(const u8 *a, const u8 *b) {
    const u32 idA = ((((u32)a[7] << 7) + a[5] + 1) & 0xffff) - 1, idB = ((((u32)b[7] << 7) + b[5] + 1) & 0xffff) - 1;
    s32 dx = (s32)(idA & 0x7f) - (s32)(idB & 0x7f), dy = (s32)(idA >> 7) - (s32)(idB >> 7);
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    return (u32)(dx * dx + dy * dy);
}

// Whether the object's definition's category (+0x164) is 1, 5 or 6 (the original fetches the definition again for each).
bool category156(const u8 *thing) {
    const u32 category = at<u32>(Object_GetDef(thing), 0x164);
    return category == 1 || category == 5 || category == 6;
}

// The dword +0x4ac of the object's definition (ObjectDefs_Find on gObjectDefs by its id +0xe).
u32 def4ac(const u8 *thing) { return at<u32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe)), 0x4ac); }

// Whether the object had customers: the low half of __ftol of its customers' ring +0x1a8's newest 30 summed (CircBufU32_30_Sum; inlined in
// 0x4cbec0).
bool hadCustomers(const u8 *thing) {
    double customers;
    CircBufU32_30_Sum(thing + 0x1a8, nullptr, &customers, 0x1e);
    return (u32)(long long)customers != 0;
}

// The objects older than 30 days that the test accepts and that value() gives a figure (signed): the one with the largest (or with fewest,
// the smallest; the first on a tie), its handle to *out (0 for none); returns that figure (0 for none).
template <bool fewest, typename Test, typename Value> s32 BestObject(u16 *out, Test test, Value value) {
    *out = 0;
    bool found = false;
    s32 best = 0;   // (unset in the original until found)
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        s32 figure;
        if (at<u8>(thing, 2) == 3 && test(thing) && Object_AgeDays(thing) > 30 && value(thing, &figure)) {
            if (!found || (fewest ? figure < best : figure > best)) {
                best = figure;
                found = true;
                *out = handle;
            }
        }
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return found ? best : 0;
}

// The walk-aways per 100 customers (Object_HistoryPercent's sums, with the walk-aways' ring +0x230 for its +0x2b8); none without customers.
bool walkAways(const u8 *thing, s32 *percent) {
    if (!hadCustomers(thing)) return false;
    double walkaways, customers;
    CircBufU32_30_Sum(thing + 0x230, nullptr, &walkaways, 0x1e);
    CircBufU32_30_Sum(thing + 0x1a8, nullptr, &customers, 0x1e);
    *percent = (s32)((u32)(long long)walkaways * 100u / (u32)(long long)customers);
    return true;
}

template <bool fewest, typename Test> s32 MostWalkAways(u16 *out, Test test) { return BestObject<fewest>(out, test, walkAways); }

// The happiness (Object_HistoryAverage).
bool happiness(const u8 *thing, s32 *average) {
    *average = (s32)Object_HistoryAverage(thing);
    return true;
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

void __stdcall Things_TopThreeValues(u8 *first, u8 *second, u8 *third) {
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

u16 *__fastcall World_NearestCellWithType(u8 *world, void *, u16 *out, u32 type, u32 cell, s32 radius) {
    const u32 index = (cell & 0xffff) - 1;
    const s32 x = (s32)(index & 0x7f), y = (s32)(index >> 7);
    const s32 xmin = x - radius < 0 ? 0 : x - radius, ymin = y - radius < 0 ? 0 : y - radius;
    const s32 xmax = x + radius > 0x7f ? 0x7f : x + radius, ymax = y + radius > 0x7f ? 0x7f : y + radius;
    u16 *found = reinterpret_cast<u16 *>(world + 0x1f020);
    s32 count = 0;
    for (s32 cx = xmin; cx <= xmax; ++cx) {
        for (s32 cy = ymin; cy <= ymax; ++cy) {
            const u16 id = (u16)((cy << 7) + 1 + cx);
            const u8 *entry = global<u8 *>(0x008023a0) + (u32)id * 0x44 - 0x44;
            if (at<u32>(entry, 0x2c) != 0) {
                const u16 word = at<u16>(entry, 0x30);
                if (word == (u16)type || word == 0)
                    found[count++] = id;
            }
        }
    }
    u32 best = 9999;
    u16 result = 0;
    for (s32 i = 0; i < count; ++i) {
        const u32 id = found[i] - 1u;
        s32 dx = (s32)(id & 0x7f) - x;
        if (dx < 0) dx = -dx;
        s32 dy = (s32)(id >> 7) - y;
        if (dy < 0) dy = -dy;
        if ((u32)(dx + dy) < best) {
            best = (u32)(dx + dy);
            result = found[i];
        }
    }
    *out = result;
    return out;
}

void __fastcall Things_UnlinkFromChain(u8 *game, void *, u32 handle) {
    u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    const u16 previous = at<u16>(thing, 8);
    if (previous == 0) {
        const u16 next = at<u16>(thing, 10);
        at<u16>(game, 0x24) = next;
        if (next != 0)
            at<u16>(global<u8 *>(0x007cfb90 + 20 * next), 8) = 0;
    } else {
        at<u16>(global<u8 *>(0x007cfb90 + 20 * previous), 10) = at<u16>(thing, 10);
        const u16 next = at<u16>(thing, 10);
        if (next != 0)
            at<u16>(global<u8 *>(0x007cfb90 + 20 * next), 8) = at<u16>(thing, 8);
    }
    at<u16>(thing, 8) = 0;
    at<u16>(thing, 10) = 0;
}

s32 __stdcall Things_AnyOfKindAround(u32 kind, u32 cell, s32 radius) { return Things_CountOfKindAround(kind, cell, radius, 1); }

s32 __stdcall Things_CountOfKindAround(u32 kind, u32 cell, s32 radius, s32 firstOnly) {
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

u32 __fastcall Staff_SurplusKind8(const u8 *object) { return Surplus(object, 0x380, 8); }

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

s32 __fastcall Things_FacilityScore(void *) {
    s32 plain = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 1 && AnalyserCell_TypePlain(thing)) ++plain;
        World_NextThing(world(), nullptr, &handle, thing);
    }
    auto cap = [](s32 v, s32 most) { return v < most ? v : most; };
    s32 score = cap(plain, 1000) * 20 / 1000;
    score += cap(Things_CountByDef4ac(0, 0) * 3 / 2, 20);
    score += cap(Things_CountByDef4ac(1, 0) * 2, 10);
    score += cap(Things_CountByDef4ac(2, 0) * 2, 10);
    score += cap(Things_CountByDef4ac(3, 0), 10);
    score += cap(Things_CountByDef4ac(0, 2), 10);
    for (u32 kind = 4; kind <= 8; ++kind) score += cap(Things_CountStaffOfKind(kind), 4);  // (5 and 4 inlined, Thing_IsStaff and the kind)
    return score;
}

void __fastcall Things_NearestRestArea(const u8 *thing, void *, u16 *out) {
    u16 best = 0;
    u32 bestDistance = 0xffffffff;
    for (u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da746); handle != 0;) {
        const u8 *other = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (other[0x32] & 2) {
            const u32 distance = CellDistanceSquared(other, thing);
            if (distance < bestDistance) {
                bestDistance = distance;
                best = handle;
            }
        }
        handle = at<u16>(other, 0xc);
    }
    *out = best;
}

void __fastcall Things_NearestFlag80Close(const u8 *thing, void *, u16 *out) {
    u16 best = 0;
    u32 bestDistance = 0xffffffff;
    for (u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da746); handle != 0;) {
        const u8 *other = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if ((other[0x32] & 0x80) && Rides_GetElement(Rides_FindById(at<s32>(other, 0x24)), 0) == 0) {
            const u32 distance = CellDistanceSquared(other, thing);
            if (distance <= 4 && distance < bestDistance) {
                bestDistance = distance;
                best = handle;
            }
        }
        handle = at<u16>(other, 0xc);
    }
    *out = best;
}

u16 *__fastcall Things_HeadForNearestFlag40(u8 *sprite, void *, u16 *out, u32 radius) {
    u16 best = 0;
    u32 bestDistance = 0xffffffff;
    for (u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da746); handle != 0;) {
        const u8 *other = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (other[0x32] & 0x40) {
            const u32 distance = CellDistanceSquared(other, sprite);
            if (distance < radius * radius && distance < bestDistance && Sprite_SetDestinationCell(sprite, nullptr, at<u16>(other, 0x36)) != 0) {
                bestDistance = distance;
                best = handle;
            }
        }
        handle = at<u16>(other, 0xc);
    }
    *out = best;
    return out;
}

u16 *__fastcall Cell_FirstObject(const u8 *cell, void *, u16 *out) {
    u16 handle = at<u16>(cell, 0x24);
    while (handle != 0) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (thing[2] == 3)
            break;
        handle = at<u16>(thing, 0xa);
    }
    *out = handle;
    return out;
}

u16 *__cdecl Map_ObjectAtCell(u16 *out, s32 *x, s32 *y) {
    if (!Map_InBounds(*x, *y)) {
        *out = 0;
        return out;
    }
    const u16 id = (u16)(((u32)*y << 7) + (u32)*x + 1);
    u8 *const world = global<u8 *>(0x007cf83c);
    const u8 *cell = world + (u32)id * 0x44 + 0x294;
    const u32 type = at<u32>(cell, 8);
    if (type != 4 && type != 9 && type != 0xa) {
        *out = 0;
        return out;
    }
    u16 where = id;
    const u16 parent = at<u16>(cell, 0x10);
    if (parent != id) {
        where = parent;
        cell = world + (u32)parent * 0x44 + 0x294;
    }
    u16 handle;
    Cell_FirstObject(cell, nullptr, &handle);
    if (handle != 0)
        where = at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)handle), 0x34);
    const u32 index = (u32)where - 1;
    *x = (s32)(index & 0x7f);
    *y = (s32)(index >> 7);
    *out = handle;
    return out;
}

u16 *__cdecl Map_ObjectOrEntranceAt(u16 *out, s32 x, s32 y) {
    const s32 first = (s32)(global<u32>(0x007855bc) - 1), last = (s32)(global<u32>(0x007855c4) + 1);  // (wrapping)
    if (x >= first && x <= last && y == global<s32>(0x007855c0)) {
        *out = at<u16>(global<u8 *>(0x007cf83c), 0x1da732);
        return out;
    }
    Map_ObjectAtCell(out, &x, &y);
    return out;
}
}  // extern "C"

namespace {
// The thing in slot (handle & 0xffff) of the thing table (as the finders read it inline, without Thing_ByHandle).
u8 *slotThing(u16 handle) { return *reinterpret_cast<u8 *const *>(0x007cfb90 + 20 * static_cast<u32>(handle)); }

u8 *categoryWorld() { return *reinterpret_cast<u8 *const *>(0x007cf83c); }

// A staff list's next handle: the list head (first) when there is no previous thing, else the previous thing's link word at +0x210.
u16 staffNext(const u8 *previous, u16 *(__fastcall *first)(const u8 *, void *, u16 *)) {
    u16 out;
    return previous ? *reinterpret_cast<const u16 *>(previous + 0x210) : *first(categoryWorld(), nullptr, &out);
}

// The average of a byte of the things that pass: each float at the offset cut to an integer and its low byte, summed (each sum by the x87
// and cut again); none: the float [0x70031c].
template <typename Test> double averageByte(Test test, u32 offset) {
    s32 sum = 0, count = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (test(thing)) {
            sum = (s32)(long long)(X87Reg((double)(u8)(long long)at<float>(thing, offset)) + X87Reg((double)sum)).v;
            ++count;
        }
        World_NextThing(world(), nullptr, &handle, thing);
    }
    if (count <= 0) return global<float>(0x0070031c);
    return (X87Reg((double)sum) / X87Reg((double)count)).v;
}

// The coaster (a thing of kind 3 whose definition is of kind 3, of the definition asked for unless that is 0) the take test picks from
// its link's figures (Object_LinkParams' b and d), the output its handle (0 beforehand); the figure take keeps, 0 when none.
template <typename Take> u32 bestCoaster(u16 *out, u32 def, Take take) {
    *out = 0;
    bool found = false;
    u32 best = 0;
    u16 handle;
    World_FirstThing(world(), nullptr, &handle);
    while (handle) {
        const u8 *thing = thing_of(handle);
        if (at<u8>(thing, 2) == 3 && Object_DefIsKind3(thing) && (!(u16)def || (u16)def == at<u16>(thing, 0xe))) {
            u32 a, b, c, d;
            Object_LinkParams(thing, &a, &b, &c, &d);
            if (take(found, best, b, d)) {
                found = true;
                *out = handle;
            }
        }
        World_NextThing(world(), nullptr, &handle, thing);
    }
    return found ? best : 0;
}

}  // namespace

extern "C" {

u8 *__cdecl Things_FindNextOfCategory(s32 category, const u8 *previous) {
    u8 *const world = categoryWorld();
    u16 out;
    switch (category) {
    case 1: {   // the next staff member (kind 4-8) in the in-use list
        u8 *found = nullptr;
        u16 handle = *(previous ? World_NextThing(world, nullptr, &out, previous) : World_FirstThing(world, nullptr, &out));
        while (handle && !found) {
            u8 *const thing = slotThing(handle);
            if (Thing_IsStaff(thing)) found = thing;
            handle = *World_NextThing(world, nullptr, &out, thing);
        }
        return found;
    }
    case 2: {   // (the handymen's list: its head read inline)
        const u16 handle = previous ? *reinterpret_cast<const u16 *>(previous + 0x210) : *reinterpret_cast<const u16 *>(world + 0x1da73c);
        return handle ? Thing_ByHandle(handle) : nullptr;
    }
    case 4: {   // (the mechanics' list, its thing read from the table inline)
        const u16 handle = staffNext(previous, &World_FirstMechanic);
        return handle ? slotThing(handle) : nullptr;
    }
    case 8:
    case 0x10:
    case 0x20: {
        const u16 handle = staffNext(previous, category == 8 ? &World_FirstGuard : category == 0x10 ? &World_FirstEntertainer : &World_FirstResearcher);
        return handle ? Thing_ByHandle(handle) : nullptr;
    }
    case 0x40: {   // the next thing of kind 1
        u8 *found = nullptr;
        u16 handle = *(previous ? World_NextThing(world, nullptr, &out, previous) : World_FirstThing(world, nullptr, &out));
        while (handle && !found) {
            u8 *const thing = Thing_ByHandle(handle);
            if (thing[2] == 1) found = thing;
            handle = *World_NextThing(world, nullptr, &out, thing);
        }
        return found;
    }
    case 0x80: return Things_FindNextByDef4ac(0, previous);
    case 0x100: return Things_FindNextByDef4ac(2, previous);
    case 0x200: return Things_FindNextByDef4ac(1, previous);
    case 0x400: return Things_FindNextByDef4ac(3, previous);
    case 0x800: return Things_FindNextShopNoFlags(previous);
    case 0x1000: return Things_FindNextShopFlag1(previous);
    case 0x2000: return Things_FindNextWithDefWord(previous);
    case 0x4000: return Things_FindNextWithDefWordUpgrade2(previous);
    case 0x8000: return Things_FindNextKind3Flag2(previous);
    default: return nullptr;
    }
}

u8 *__cdecl Things_FindPrevOfCategory(s32 category, const u8 *previous) {
    u8 *const world = categoryWorld();
    u16 out;
    switch (category) {
    case 1: {   // the previous staff member; with no previous thing, from the oldest thing (found by walking the whole list: sic, a null thing
                // when there are none)
        u16 handle;
        if (previous) {
            handle = *World_PrevThing(world, nullptr, &out, previous);
        } else {
            u8 *last = nullptr;
            for (u16 h = *World_FirstThing(world, nullptr, &out); h; h = *World_NextThing(world, nullptr, &out, last)) last = Thing_ByHandle(h);
            Thing_GetHandle(last, nullptr, &handle);
        }
        u8 *found = nullptr;
        while (handle && !found) {
            u8 *const thing = Thing_ByHandle(handle);
            if (Thing_IsStaff(thing)) found = thing;
            handle = *World_PrevThing(world, nullptr, &out, thing);
        }
        return found;
    }
    case 2:
    case 4:
    case 8:
    case 0x10:
    case 0x20:
    case 0x40: {   // (all of these: the previous thing of kind 1)
        u8 *found = nullptr;
        u16 handle = *(previous ? World_PrevThing(world, nullptr, &out, previous) : Things_LastHandle(&out));
        while (handle && !found) {
            u8 *const thing = slotThing(handle);
            if (thing[2] == 1) found = thing;
            handle = *World_PrevThing(world, nullptr, &out, thing);
        }
        return found;
    }
    case 0x80: return Things_FindPrevByDef4ac(0, previous);
    case 0x100: return Things_FindPrevByDef4ac(2, previous);
    case 0x200: return Things_FindPrevByDef4ac(1, previous);
    case 0x400: return Things_FindPrevByDef4ac(3, previous);
    case 0x800:
    case 0x1000: {   // the previous shop (kind 3 object, definition word +0x284 0 and dword +0x4ac 3) with neither flag of +0x32 (0x800) / with flag 1 (0x1000)
        u8 *found = nullptr;
        u16 handle = *(previous ? World_PrevThing(world, nullptr, &out, previous) : Things_LastHandle(&out));
        while (handle && !found) {
            u8 *const thing = category == 0x800 ? slotThing(handle) : Thing_ByHandle(handle);
            if (thing[2] == 3 && *Object_DefWord284(thing, nullptr, &out) == 0 && *reinterpret_cast<const s32 *>(Object_GetDef(thing) + 0x4ac) == 3 &&
                (category == 0x800 ? (thing[0x32] & 3) == 0 : (thing[0x32] & 1) != 0))
                found = thing;
            handle = *World_PrevThing(world, nullptr, &out, thing);
        }
        return found;
    }
    case 0x2000: return Things_FindPrevWithDefWord(previous);
    case 0x4000: return Things_FindPrevWithDefWordUpgrade2(previous);
    case 0x8000: return Things_FindPrevKind3Flag2(previous);
    default: return nullptr;
    }
}

u8 *__cdecl Things_FindNextOfCategoryWrap(s32 category, const u8 *previous) {
    u8 *const next = Things_FindNextOfCategory(category, previous);
    return next ? next : Things_FindNextOfCategory(category, nullptr);
}

u8 *__cdecl Things_FindPrevOfCategoryWrap(s32 category, const u8 *previous) {
    u8 *const found = Things_FindPrevOfCategory(category, previous);
    return found ? found : Things_FindPrevOfCategory(category, nullptr);
}

void __cdecl ThingBrowser_Step() {
    const u8 *&current = *reinterpret_cast<const u8 **>(0x007cc308);
    const u32 choice = *reinterpret_cast<const u32 *>(0x007cc30c) - 1;
    if (choice > 5) {
        current = nullptr;
        return;
    }
    const s32 category = 2 << choice;
    current = Things_FindNextOfCategory(category, current);
    if (!current) current = Things_FindNextOfCategory(category, nullptr);
}

s32 __fastcall Things_MostWalkAwaysOf(void *, void *, u32 category, u16 *out) {
    return MostWalkAways<false>(out, [category](const u8 *thing) { return at<u32>(Object_GetDef(thing), 0x164) == category; });
}

s32 __fastcall Things_MostWalkAwaysOf156(void *, void *, u16 *out) {
    return MostWalkAways<false>(out, [](const u8 *thing) { return category156(thing); });
}

s32 __fastcall Things_FewestWalkAwaysOf(void *, void *, u32 category, u16 *out) {
    return MostWalkAways<true>(out, [category](const u8 *thing) { return at<u32>(Object_GetDef(thing), 0x164) == category; });
}

s32 __fastcall Things_FewestWalkAwaysOf156(void *, void *, u16 *out) {
    return MostWalkAways<true>(out, [](const u8 *thing) { return category156(thing); });
}

s32 __fastcall Things_MostWalkAwaysDef4ac2(void *, void *, u16 *out) {
    return MostWalkAways<false>(out, [](const u8 *thing) { return def4ac(thing) == 2; });
}

s32 __fastcall Things_FewestWalkAwaysDef4ac2(void *, void *, u16 *out) {
    return MostWalkAways<true>(out, [](const u8 *thing) { return def4ac(thing) == 2; });
}

s32 __fastcall Things_HappiestOf(void *, void *, u32 category, u16 *out) {
    return BestObject<false>(out, [category](const u8 *thing) { return at<u32>(Object_GetDef(thing), 0x164) == category; }, happiness);
}

s32 __fastcall Things_UnhappiestOf(void *, void *, u32 category, u16 *out) {
    return BestObject<true>(out, [category](const u8 *thing) { return at<u32>(Object_GetDef(thing), 0x164) == category; }, happiness);
}

s32 __fastcall Things_HappiestOf156(void *, void *, u16 *out) {
    return BestObject<false>(out, [](const u8 *thing) { return category156(thing); }, happiness);
}

s32 __fastcall Things_UnhappiestOf156(void *, void *, u16 *out) {
    return BestObject<true>(out, [](const u8 *thing) { return category156(thing); }, happiness);
}

s32 __fastcall Things_HappiestDef4ac2(void *, void *, u16 *out) {
    return BestObject<false>(out, [](const u8 *thing) { return def4ac(thing) == 2; },
                             [](const u8 *thing, s32 *average) { return hadCustomers(thing) && happiness(thing, average); });
}

double __fastcall Things_AveragePersonStat19c(void *, void *, u32) {
    if (Get1da710_51a280(static_cast<const u8 *>(world()))) return global<float>(0x0070031c);
    return averageByte([](const u8 *thing) { return at<u8>(thing, 2) == 1 && AnalyserCell_TypePlain(thing); }, 0x19c);
}

double __fastcall Things_AverageStaffStat1f8(void *, void *, u32) {
    return averageByte([](const u8 *thing) { return at<u8>(thing, 2) >= 4 && at<u8>(thing, 2) <= 8; }, 0x1f8);
}

u32 __stdcall Things_CoasterMostFlaggedCars(u16 *out, u32 def) {
    return bestCoaster(out, def, [](bool found, u32 &best, u32, u32 cars) {
        if (found && cars <= best) return false;
        best = cars;
        return true;
    });
}

u32 __stdcall Things_TallestCoaster(u16 *out, u32 def) {
    return bestCoaster(out, def, [](bool found, u32 &best, u32 bits, u32) {
        float height;
        std::memcpy(&height, &bits, 4);
        if (found && !((double)best < (double)height || height != height)) return false;   // (a NaN height is taken, x87's FCOMP)
        best = (u32)(long long)height;
        return true;
    });
}
}  // extern "C"
