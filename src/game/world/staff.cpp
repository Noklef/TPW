#include "staff.h"

#include <cstdint>

#include "../core/selectors.h"
#include "../math/x87.h"
#include "money.h"
#include "things.h"
#include "thingscan.h"
#include "world.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The squared distance between the map positions of two things (bytes +5 and +7 are the column and the row of a cell of 128 columns).
u32 SquaredDistance(const u8 *a, const u8 *b) {
    const u32 cellA = ((u32)at<u8>(a, 7) << 7) + at<u8>(a, 5) + 1;
    const u32 cellB = ((u32)at<u8>(b, 7) << 7) + at<u8>(b, 5) + 1;
    const s32 x = (s32)((cellA - 1) & 0x7f) - (s32)((cellB - 1) & 0x7f);
    const s32 y = (s32)((cellA - 1) >> 7) - (s32)((cellB - 1) >> 7);
    return (u32)(x * x + y * y);
}

// The tail both drains share: fcom with 100.0f ([0x70082c]), then 0.0f ([0x700830]).
void clamp_store(float &to, X87Reg v) {
    if (v.v > 100.0) {
        to = 100.0f;
    } else if (!(v.v >= 0.0)) {  // below 0, or unordered
        to = 0.0f;
    } else {
        to = (float)v;
    }
}

}  // namespace

extern "C" {

u32 __fastcall Staff_IsTooTired(const u8 *staff) {
    const u32 level = (u32)(long long)at<float>(staff, 0x1fc) & 0xff;  // __ftol
    return (s32)level < *reinterpret_cast<const s32 *>(0x00785324);
}

void __fastcall Staff_TireA(u8 *staff) {
    float &energy = at<float>(staff, 0x1fc);
    clamp_store(energy, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.012)) + X87Reg(energy));
    float &happiness = at<float>(staff, 0x1f8);
    clamp_store(happiness, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.005)) + X87Reg(happiness));
}

void __fastcall Staff_TireB(u8 *staff) {
    float &energy = at<float>(staff, 0x1fc);
    clamp_store(energy, X87Reg(-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.025f)) + X87Reg(energy));
    float &happiness = at<float>(staff, 0x1f8);
    const X87Reg was = happiness;  // loaded before the drain
    const float drain = (float)-X87_MulIntDouble(6 - at<s32>(staff, 0x1e4), 0.01);
    clamp_store(happiness, was + X87Reg(drain));
}

void __fastcall Staff_SetName(u8 *staff, void *, const u16 *name) {
    // The CRT's wcsncpy (0x67b0d0), a unit at a time: up to and with the NUL, then zeros.
    u16 *to = &at<u16>(staff, 0x1a0);
    u32 count = 0x20;
    while (count && (*to++ = *name++) != 0) --count;
    if (count)
        while (--count) *to++ = 0;
    at<u16>(staff, 0x1e0) = 0;
}

u16 *__fastcall Staff_GetName(u8 *staff) { return &at<u16>(staff, 0x1a0); }

u32 __fastcall Thing_IsStaff(const u8 *thing) {
    const u8 type = at<u8>(thing, 2);
    return type == 4 || type == 5 || type == 6 || type == 8 || type == 7;
}

u16 *__fastcall World_FirstHandyman(const u8 *world, void *, u16 *out) { return *out = at<u16>(world, 0x1da73c), out; }
u16 *__fastcall World_FirstMechanic(const u8 *world, void *, u16 *out) { return *out = at<u16>(world, 0x1da73e), out; }
u16 *__fastcall World_FirstEntertainer(const u8 *world, void *, u16 *out) { return *out = at<u16>(world, 0x1da740), out; }
u16 *__fastcall World_FirstResearcher(const u8 *world, void *, u16 *out) { return *out = at<u16>(world, 0x1da742), out; }
u16 *__fastcall World_FirstGuard(const u8 *world, void *, u16 *out) { return *out = at<u16>(world, 0x1da744), out; }

namespace {

void push(u8 *world, u32 head, u8 *thing, u32 next) {
    const u16 old = at<u16>(world, head);
    u16 handle;
    at<u16>(world, head) = *Thing_GetHandle(thing, nullptr, &handle);
    if (old != 0) at<u16>(thing, next) = old;
}

}  // namespace

void __fastcall World_AddHandyman(u8 *world, void *, u8 *thing) { push(world, 0x1da73c, thing, 0x210); }
void __fastcall World_AddMechanic(u8 *world, void *, u8 *thing) { push(world, 0x1da73e, thing, 0x210); }
void __fastcall World_AddEntertainer(u8 *world, void *, u8 *thing) { push(world, 0x1da740, thing, 0x210); }
void __fastcall World_AddResearcher(u8 *world, void *, u8 *thing) { push(world, 0x1da742, thing, 0x210); }
void __fastcall World_AddGuard(u8 *world, void *, u8 *thing) { push(world, 0x1da744, thing, 0x210); }
void __fastcall World_AddObject(u8 *world, void *, u8 *thing) { push(world, 0x1da746, thing, 0xc); }

u32 __fastcall Guard_IsFree(const u8 *guard) {
    const u32 state = at<u32>(guard, 0x19c);
    return state == 1 || state == 0;
}

u32 __fastcall Guard_IsState12(const u8 *guard) { return at<u32>(guard, 0x19c) == 0x12; }

u32 __fastcall Staff_IsState3At(const u8 *staff, void *, u32 area) {
    return at<u32>(staff, 0x19c) == 3 && at<u16>(staff, 0x208) == (u16)area;
}

s32 __cdecl Staff_WageFor(u32 type, u32 grade) {
    return (s32)(global<u32>(0x0078532c + (grade & 0xff) * 16) * global<u32>(0x00785380 + type * 4));
}

s32 __fastcall Staff_GetWage(const u8 *staff) {
    u32 type;
    switch (at<u8>(staff, 2)) {
    case 4: type = 1; break;
    case 5: type = 0; break;
    case 6: type = 2; break;
    case 7: type = 3; break;
    case 8: type = 4; break;
    default: type = 5;
    }
    return (s32)(global<u32>(0x00785380 + type * 4) * global<u32>(0x0078532c + (at<u32>(staff, 0x1e4) << 4)));
}

void __fastcall Staff_SetPatrolRegion(u8 *staff, void *, u32 bl, u32 tr) {
    at<u16>(staff, 0x20a) = (u16)bl;
    at<u16>(staff, 0x20c) = (u16)tr;
}

void __fastcall Staff_GetPatrolRegion(const u8 *staff, void *, u16 *bl, u16 *tr) {
    *bl = at<u16>(staff, 0x20a);
    *tr = at<u16>(staff, 0x20c);
}

void __fastcall Staff_ClearPatrolRegion(u8 *staff) {
    at<u16>(staff, 0x20a) = 1;
    at<u16>(staff, 0x20c) = 0x4000;
}

u32 __fastcall Staff_HasPatrolRegion(const u8 *staff) {
    return !(at<u16>(staff, 0x20a) == 1 && at<u16>(staff, 0x20c) == 0x4000);
}

u32 __fastcall Staff_InPatrolRegion(const u8 *staff, void *, u32 pos) {
    const u32 id = (pos & 0xffff) - 1, bl = (u32)at<u16>(staff, 0x20a) - 1, tr = (u32)at<u16>(staff, 0x20c) - 1;
    // Each part is at most 0x1ffffff, so the original's signed comparisons are these.
    return (id & 0x7f) >= (bl & 0x7f) && (id & 0x7f) <= (tr & 0x7f) && id >> 7 >= bl >> 7 && id >> 7 <= tr >> 7;
}

u8 *__fastcall StaffPool_ValidEntry(u8 *pool, void *, u32 index) {
    if (index >= 0x20) return nullptr;
    u8 *const entry = pool + index * 20;
    return at<u8>(entry, 0xa) ? entry : nullptr;
}

u8 *__fastcall StaffTraining_Ctor(u8 *training) {
    for (u32 i = 0; i != 5; ++i) at<u32>(training, i * 4) = 0;
    at<u8>(training, 0x14) = 0;
    return training;
}

u32 __fastcall StaffPool_FirstInvalid(const u8 *pool) {
    for (u32 i = 0; i != 0x20; ++i) {
        if (at<u8>(pool + i * 20, 0xa) == 0) return i;
    }
    return 0x20;
}

u16 *__fastcall Staff_NearestInList1da740(const u8 *object, void *, u16 *out) {
    u16 best = 0;
    u32 bestDistance = 0xffffffff;
    u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da740);
    while (handle) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const u32 distance = SquaredDistance(thing, object);
        if (distance < bestDistance) {
            bestDistance = distance;
            best = handle;
        }
        handle = at<u16>(thing, 0x210);
    }
    *out = best;
    return out;
}

u16 *__fastcall Guard_NearestFree(const u8 *object, void *, u16 *out, u32) {
    u16 best = 0;
    u32 bestDistance = 0xffffffff;
    u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da744);
    while (handle) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const u32 distance = SquaredDistance(thing, object);
        if (Guard_IsFree(thing) && distance < bestDistance) {
            bestDistance = distance;
            best = handle;
        }
        handle = at<u16>(thing, 0x210);
    }
    *out = best;
    return out;
}

s32 __fastcall Staff_WageForRecord(const u8 *record) { return Staff_WageFor(at<u32>(record, 0), at<u8>(record, 8)); }

void __fastcall Staff_Train(u8 *staff, void *, s32 amount) {
    if (at<s32>(staff, 0x1e4) == 4) return;
    Money_Debit(reinterpret_cast<u8 *>(GameTable7cfb90_Get1da726_5195d0(global<u8 *>(0x0080239c))), nullptr, amount);
    u8 *const park = reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c)));
    at<s32>(park, 0x1fa40) = at<s32>(park, 0x1fa40) + amount;
    const s32 grade = at<s32>(staff, 0x1e4);
    s32 cost;
    switch (at<u8>(staff, 2)) {
    case 4: cost = global<s32>(0x00785430 + grade * 8); break;
    case 5: cost = global<s32>(0x007853f4 + grade * 12); break;
    case 6: cost = global<s32>(0x007853a4 + grade * 16); break;
    case 7: cost = global<s32>(0x0078549c + grade * 8); break;
    case 8: cost = global<s32>(0x00785460 + grade * 12); break;
    default: cost = amount; break;
    }
    s32 points = amount / cost;  // sic: faults for a cost of 0 (and -0x80000000 / -1)
    if (points > 100) points = 100;
    if (points + at<u8>(staff, 0x1e8) > 0x63) {
        at<s32>(staff, 0x1e4) = at<s32>(staff, 0x1e4) + 1;
        at<u8>(staff, 0x1e8) = (u8)(at<u8>(staff, 0x1e8) + (u8)(points - 100));
        at<u32>(staff, 0x1f8) = 0x42c80000;  // 100.0f
    } else {
        at<u8>(staff, 0x1e8) = (u8)(at<u8>(staff, 0x1e8) + points);
    }
}

void __fastcall Staff_CountTypes(u8 *counts) {
    for (u32 k = 0; k < 5; ++k) at<u32>(counts, 0x280 + k * 4) = 0;
    at<u32>(counts, 0x299) = 0;
    at<u8>(counts, 0x29d) = 0;
    u16 handle;
    World_FirstThing(global<void *>(0x0080239c), nullptr, &handle);
    while (handle != 0) {
        const u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        u32 count = 0, limit = 0, flag = 0;
        switch (at<u8>(thing, 2)) {
        case 4: count = 0x284; limit = 0x007852ec; flag = 0x29a; break;
        case 5: count = 0x280; limit = 0x007852e8; flag = 0x299; break;
        case 6: count = 0x288; limit = 0x007852f0; flag = 0x29b; break;
        case 7: count = 0x28c; limit = 0x007852f4; flag = 0x29c; break;
        case 8: count = 0x290; limit = 0x007852f8; flag = 0x29d; break;
        }
        if (count != 0) {
            const s32 now = at<s32>(counts, count) + 1;
            at<s32>(counts, count) = now;
            if (now >= global<s32>(limit)) at<u8>(counts, flag) = 1;
        }
        u16 next;
        handle = *World_NextThing(global<void *>(0x0080239c), nullptr, &next, thing);
    }
}

namespace {

// The index of a staff type (byte +2) in Staff_TrainAll's amounts and counts: mechanic, handyman, entertainer, researcher, guard; -1 for another.
s32 TrainSlot(u8 type) {
    switch (type) {
    case 5: return 0;
    case 4: return 1;
    case 6: return 2;
    case 7: return 3;
    case 8: return 4;
    }
    return -1;
}

}  // namespace

void __fastcall Staff_TrainAll(const s32 *amounts) {
    s32 count[5] = {0, 0, 0, 0, 0};
    u16 handle;
    World_FirstThing(global<void *>(0x007cf83c), nullptr, &handle);
    while (handle != 0) {
        const u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const s32 slot = TrainSlot(at<u8>(thing, 2));
        if (slot >= 0) ++count[slot];
        u16 next;
        handle = *World_NextThing(global<void *>(0x007cf83c), nullptr, &next, thing);
    }
    World_FirstThing(global<void *>(0x007cf83c), nullptr, &handle);
    while (handle != 0) {
        u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        const s32 slot = TrainSlot(at<u8>(thing, 2));
        if (slot >= 0) Staff_Train(thing, nullptr, amounts[slot] / count[slot]);
        u16 next;
        handle = *World_NextThing(global<void *>(0x007cf83c), nullptr, &next, thing);
    }
}

u32 __fastcall Table_ScaledBySum(u8 *table, void *, s32 index) {
    const u32 total = (u32)Global7852f8_Times78548c();
    if (total == 0) return 0;
    const u32 share = (Things_SumChain() * 100u) / total;
    return (at<u32>(table, ((u32)index + 0x141) * 16) * share) / 100u;
}
}
