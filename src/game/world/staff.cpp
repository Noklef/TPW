#include "staff.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/gameheap.h"
#include "../core/idmaps.h"
#include "../core/indexed.h"
#include "../core/initializers.h"
#include "../core/savegame.h"
#include "../core/timesig.h"
#include "../core/selectors.h"
#include "../core/textarg.h"
#include "../core/thunks.h"
#include "../core/vcall.h"
#include "../math/x87.h"
#include "../sprite/scriptref.h"
#include "analyser.h"
#include "analysercell.h"
#include "mapcell.h"
#include "messagecentre.h"
#include "money.h"
#include "mousemode.h"
#include "object.h"
#include "person.h"
#include "personscripts.h"
#include "random.h"
#include "research.h"
#include "ridelist.h"
#include "thingmsg.h"
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

u32 __fastcall StaffTraining_Serialize(u8 *training, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 5; ++i)
        if (!Save_Field(stream, save, training + i * 4, 4, 0x00761eb0))
            return 1;
    return Save_Field(stream, save, training + 0x14, 1, 0x00761e9c) ? 0 : 1;
}

u32 __fastcall StaffPool_FirstInvalid(const u8 *pool) {
    for (u32 i = 0; i != 0x20; ++i) {
        if (at<u8>(pool + i * 20, 0xa) == 0) return i;
    }
    return 0x20;
}

u32 __fastcall StaffPool_CountOfType(const u8 *pool, void *, u32 type) {
    u32 count = 0;
    for (u32 i = 0; i != 0x20; ++i) {
        if (at<u8>(pool + i * 20, 0xa) && at<u32>(pool + i * 20, 0) == type) ++count;
    }
    return count;
}

u8 *__fastcall StaffPoolEntry_Ctor(u8 *entry) {
    at<u32>(entry, 0xc) = 0;
    at<u32>(entry, 0) = 0;
    at<u8>(entry, 9) = 0;
    at<u8>(entry, 0xa) = 0;
    at<u8>(entry, 0xb) = 0;
    return entry;
}

void __fastcall StaffPoolEntry_FormatName(const u8 *entry, void *, u16 *to, u32 size) {
    const u32 kind = at<u32>(entry, 0);
    if (kind > 4) return;   // (Debug_Printf, empty in the retail build)
    TextFormat().format(global<void *>(0x0078baa8 + 4 * kind), to, size, at<u32>(entry, 4));
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

s32 __fastcall Staff_AgeDays(const u8 *staff) { return FunnyTime_DaysSince(reinterpret_cast<const FileTime *>(staff + 0x1f0)); }

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

u32 __fastcall StaffPoolEntry_Serialize(u8 *entry, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x0, 4, 0x0075a034}, {0x4, 4, 0x0075e6d8}, {0x8, 1, 0x0075e6cc},
                                       {0x9, 1, 0x0075e6c0}, {0xa, 1, 0x0075e6b8}, {0xb, 1, 0x0075e6ac}};
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Fields(stream, save, entry, kFields, 6) || TimeSig_Serialize(entry + 0xc, nullptr, stream, save, version, unused))
        return 1;
    return Save_Field(stream, save, entry + 0x10, 4, 0x0075e69c) ? 0 : 1;
}

u32 __fastcall Staff_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (Sprite_Serialize(staff, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (version < 0 || version > 1)
        return 0;
    static const SaveSpec kRest[] = {{0x20a, 2, 0x0075e370}, {0x20c, 2, 0x0075e360}, {0x1e8, 1, 0x0075e348}, {0x208, 2, 0x0075e33c},
                                     {0x19c, 4, 0x0075b5b0}, {0x200, 4, 0x0075d1e0}, {0x1f0, 8, 0x0075e330}};
    if (save != 0) {
        // the two floats are saved as the float of their __ftol's low byte
        float happiness = (float)(u8)(long long)at<float>(staff, 0x1f8);
        if (!Save_Field(stream, save, staff + 0x1e4, 4, 0x0075e398) || !Save_Field(stream, save, &happiness, 4, 0x0075b444) ||
            !Save_Field(stream, save, staff + 0x204, 4, 0x0075e38c))
            return 1;
        for (u32 i = 0; i < 0x21; ++i)
            if (!Save_Field(stream, save, staff + 0x1a0 + i * 2, 2, 0x0075e380))
                return 1;
        if (!Save_Fields(stream, save, staff, kRest, 7))
            return 1;
        float energy = (float)(u8)(long long)at<float>(staff, 0x1fc);
        return Save_Field(stream, save, &energy, 4, 0x0075b444) ? 0 : 1;
    }
    // (the load reads all but the first two fields through the readers, which announce and read the same way; the floats land only when whole)
    u32 value = 0;
    if (!Save_Field(stream, 0, staff + 0x1e4, 4, 0x0075e398) || !Save_Field(stream, 0, &value, 4, 0x0075b444))
        return 1;
    at<u32>(staff, 0x1f8) = value;
    if (!Save_Field(stream, 0, staff + 0x204, 4, 0x0075e38c))
        return 1;
    for (u32 i = 0; i < 0x21; ++i)
        if (!Save_Field(stream, 0, staff + 0x1a0 + i * 2, 2, 0x0075e380))
            return 1;
    if (!Save_Fields(stream, 0, staff, kRest, 7) || !Save_Field(stream, 0, &value, 4, 0x0075b444))
        return 1;
    at<u32>(staff, 0x1fc) = value;
    return 0;
}

u32 __fastcall StaffPool_Serialize(u8 *pool, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    if (version < 0 || version > 1)
        return 0;
    for (u32 i = 0; i < 0x20; ++i)
        if (StaffPoolEntry_Serialize(pool + i * 0x14, nullptr, stream, save, version, unused))
            return 1;
    for (u32 i = 0; i < 5; ++i)
        if (!Save_Field(stream, save, pool + 0x280 + i * 4, 4, 0x0075e7b8) || !Save_Field(stream, save, pool + 0x299 + i, 1, 0x0075e7a4))
            return 1;
    if (TimeSig_Serialize(pool + 0x294, nullptr, stream, save, version, unused))
        return 1;
    return Save_Field(stream, save, pool + 0x298, 1, 0x0075e788) ? 0 : 1;
}
}

extern "C" void __fastcall Staff_InitBase(u8 *staff) {
    at<u8>(staff, 0x1e8) = 0;
    FileTime now;
    const FileTime *t = FunnyTime_Now(global<u8 *>(0x0080239c) + 0x2a0, nullptr, &now, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                                      nullptr, nullptr);
    at<FileTime>(staff, 0x1f0) = *t;
    at<u16>(staff, 0x208) = 0;
    at<u32>(staff, 0x204) = 0;
    at<u32>(staff, 0x200) = 0;
    at<u16>(staff, 0x20a) = 1;
    at<u32>(staff, 0x1f8) = 0x42c80000;  // 100.0f
    at<u32>(staff, 0x1fc) = 0x42c80000;
    at<u16>(staff, 0x20c) = 0x4000;
    at<u16>(staff, 0xc2) = global<u16>(0x0075c7f0);
    at<u16>(staff, 0xc0) = at<s32>(staff, 0x1e4) < 3 ? global<u16>(0x0075c7f8) : global<u16>(0x0075c7fc);
    MessageCentre_AddThing(MessageCentre_Instance(), nullptr, staff, 0xc);
    ParkAnalyser_StampTime21c38(reinterpret_cast<u8 *>(GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c))));
}

extern "C" u8 *__fastcall Staff_CtorTyped(u8 *staff, void *, u32 cell, u32 kind, u32 type, u32) {
    MovingPerson_Ctor(staff, nullptr, cell, kind);
    at<u32>(staff, 0x1e4) = type;
    at<u32>(staff, 0x1f0) = 0;
    at<u32>(staff, 0x1f4) = 0;
    at<u32>(staff, 0x1f8) = 0;
    at<u32>(staff, 0x1fc) = 0;
    Staff_InitBase(staff);
    return staff;
}

extern "C" u8 *__fastcall Staff_Ctor(u8 *staff, void *, u32 cell, u32 kind) { return Staff_CtorTyped(staff, nullptr, cell, kind, 0, 0); }

namespace {

// A staff type's serializer: Staff_Serialize, then (a version of 0..1) the type's fields; 1 at the first that fails, else 0.
u32 StaffType_Serialize(u8 *staff, Stream *stream, u32 save, s32 version, u32 unused, const SaveSpec *spec, u32 count) {
    if (Staff_Serialize(staff, nullptr, stream, save, version, unused) != 0)
        return 1;
    if (version < 0 || version > 1)
        return 0;
    return Save_Fields(stream, save, staff, spec, count) ? 0 : 1;
}

}  // namespace

extern "C" {

u32 __fastcall Handyman_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x218, 2, 0x0075aabc}, {0x214, 4, 0x0075aaa4}, {0x21a, 2, 0x0075aa94}, {0x210, 2, 0x0075a4f4}};
    return StaffType_Serialize(staff, stream, save, version, unused, kFields, 4);
}

u32 __fastcall Guard_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x212, 2, 0x0075a8f0}, {0x214, 4, 0x0075a8d8}, {0x210, 2, 0x0075a4f4}};
    return StaffType_Serialize(staff, stream, save, version, unused, kFields, 3);
}

u32 __fastcall Mechanic_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x214, 4, 0x0075b168}, {0x218, 2, 0x0075b158}, {0x210, 2, 0x0075a4f4}};
    return StaffType_Serialize(staff, stream, save, version, unused, kFields, 3);
}

u32 __fastcall Entertainer_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x214, 4, 0x0075a4fc}, {0x210, 2, 0x0075a4f4}};
    return StaffType_Serialize(staff, stream, save, version, unused, kFields, 2);
}

u32 __fastcall Researcher_Serialize(u8 *staff, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    static const SaveSpec kFields[] = {{0x214, 4, 0x0075e168}, {0x210, 2, 0x0075a4f4}};
    return StaffType_Serialize(staff, stream, save, version, unused, kFields, 2);
}

u32 __fastcall Staff_GradeLevel(const u8 *staff) {
    // fild grade; fild progress; fmul -0.01 (a double); fsubp; fmul 20.0f; __ftol's low dword
    const X87Reg grade = X87Reg((double)at<s32>(staff, 0x1e4)) - X87Reg(X87_MulDouble((double)at<u8>(staff, 0x1e8), -0.01));
    return (u32)(long long)X87_MulDouble(grade.v, 20.0);
}

void __fastcall Staff_SetState(u8 *staff, void *, u32 state) {
    u8 *const ref = staff + 0xc;
    const auto tick = [] { return at<u32>(global<u8 *>(0x0080239c), 0x1da70c); };
    switch (state) {
    case 0:
        at<u32>(staff, 0x200) = at<u32>(staff, 0x19c) == 1 ? tick() : 0;
        at<u16>(staff, 0xc2) = global<u16>(0x0075c7f0);
        Wrap_Set4_4217f0_4fa460(staff);
        break;
    case 4:
        at<u16>(staff, 0xc2) = global<u16>(0x0075c7f2);
        Set4_4217f0(ref, nullptr, 9);
        break;
    case 1:
        Set4_4217f0(ref, nullptr, 9);
        break;
    case 2:
        Set4_4217f0(ref, nullptr, 1);
        break;
    case 3:
        Person_KillScript84(staff + 0x30);
        Obj_KillSlot(ref);
        break;
    case 5:
        at<u32>(staff, 0x1c) = 0;
        at<u16>(staff, 0xc2) = global<u16>(0x0075c7f0);
        Set4_4217f0(ref, nullptr, 0x13);
        break;
    case 6:
        at<u32>(staff, 0x200) = tick();
        Set4_4217f0(ref, nullptr, 0x14);
        break;
    case 7: {
        Obj_KillSlot(ref);
        Person_KillScript84(staff + 0x30);
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x14));
        u8 *const mode = block ? MMStaffDrop_Ctor(block, nullptr, at<u32>(staff, 0x24), at<u8>(staff, 0x20)) : nullptr;
        MouseModes_EnsureHolder();
        MouseMode_Set(mode);
        break;
    }
    default:   // (the original's debug print)
        break;
    }
    at<u32>(staff, 0x19c) = state;
}

void __fastcall Staff_Settle(u8 *staff) {
    if (!AnalyserRecord_ChecksCell(staff)) return;   // (inlined: not 4, 5, 7, 0xd or 0x12, and the cell plain)
    if (at<u8>(staff, 2) == 7 && at<u32>(staff, 0x19c) == 0x12) return;
    Staff_SetState(staff, nullptr, 0);
}

namespace {
// The holder made when missing; when the current mode answers 6 from its slot 9 (none: 0), a new CMMOnline base set (as 0x505c50 and
// 0x505790 have it inlined).
void drop_staff_mode() {
    MouseModes_EnsureHolder();
    u8 *const current = global<u8 *>(0x007b05d8);
    if ((current ? vcall<s32>(current, 0x24) : 0) == 6) {
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc(8));
        u8 *const mode = block ? MouseModeOnlineBase_Ctor(block) : nullptr;
        MouseModes_EnsureHolder();
        MouseMode_Set(mode);
    }
}
}  // namespace

void __fastcall Staff_PickUp(u8 *staff) {
    drop_staff_mode();
    u16 handle;
    Fn_509ae0(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(global<u8 *>(0x0080239c))), nullptr,
              *Thing_GetHandle(staff, nullptr, &handle));
    Staff_SetState(staff, nullptr, 7);
}

u16 *__fastcall Handyman_FindJob(u8 *staff, void *, u16 *out) {
    u16 word;
    u32 best = 0xffffffff;
    u16 found = 0;
    for (u16 handle = at<u16>(global<u8 *>(0x0080239c), 0x1da746); handle;) {
        u8 *const object = global<u8 *>(0x007cfb90 + 20 * handle);
        if (at<u8>(object, 0x32) & 1) {
            bool free = *Obj_GetTrackedHandle(object, nullptr, &word) == 0;
            if (!free) {
                const u16 *const mine = Thing_GetHandle(staff, nullptr, &word);
                u16 tracked;
                free = *Obj_GetTrackedHandle(object, nullptr, &tracked) == *mine;
            }
            if (free) {
                const u32 o = (u16)((at<u8>(object, 7) << 7) + at<u8>(object, 5) + 1) - 1;
                const u32 s = (u16)((at<u8>(staff, 7) << 7) + at<u8>(staff, 5) + 1) - 1;
                const s32 dx = (s32)(o & 0x7f) - (s32)(s & 0x7f), dy = (s32)(o >> 7) - (s32)(s >> 7);
                const u32 d2 = (u32)(dx * dx) + (u32)(dy * dy);
                bool near = Get64_4e03d0(object) && d2 < best;
                if (!near && Object_InPoorCondition(object)) {
                    const u32 radius = global<u32>(0x007853f0 + 12 * at<u32>(staff, 0x1e4));
                    near = d2 < radius * radius;
                }
                if (near && Sprite_SetDestinationCell(staff, nullptr, at<u16>(object, 0x36))) {
                    best = d2;
                    found = handle;
                }
            }
        }
        handle = at<u16>(object, 0xc);
    }
    if (found) Obj_StampAt5e(global<u8 *>(0x007cfb90 + 20 * found), nullptr, *Thing_GetHandle(staff, nullptr, &word));
    *out = found;
    return out;
}

void __fastcall Mechanic_SetState(u8 *mechanic, void *, u32 state) {
    if (state == 0xc) {
        Set4_4217f0(mechanic + 0xc, nullptr, 9);
    } else if (state == 0xd) {
        Set4_4217f0(mechanic + 0xc, nullptr, 0x12);
        const u8 *const target = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(mechanic, 0x218));
        const u32 scale = global<u32>(0x0078542c + 8 * at<u32>(mechanic, 0x1e4));
        if (Is19c_Eq2_4e03e0(target)) {
            const u32 rate = target[0x50];
            at<u32>(mechanic, 0x214) = at<u32>(Object_GetDef(target), 0x1fc + (rate << 6)) * scale;
        } else {
            const u32 condition = (u8)(long long)at<float>(target, 0x44);   // (__ftol's low byte)
            at<s32>(mechanic, 0x214) = (s32)((0x64 - condition) * scale) / 100;
        }
    } else {
        Staff_SetState(mechanic, nullptr, state);
    }
    at<u32>(mechanic, 0x19c) = state;
}

void __fastcall Handyman_SetState(u8 *handyman, void *, u32 state) {
    if (state == 8 || state == 0xa) {
        at<u16>(handyman, 0xc2) = global<u16>(0x0075c7f2);
        Set4_4217f0(handyman + 0xc, nullptr, 9);
    } else if (state == 9 || state == 0xb) {
        at<u32>(handyman, 0x214) = global<u32>(global<u32>(0x0080239c) + 0x1da70c);
        Set4_4217f0(handyman + 0xc, nullptr, 0x11);
    } else {
        Staff_SetState(handyman, nullptr, state);
    }
    at<u32>(handyman, 0x19c) = state;
}

void __fastcall Handyman_WalkToJob(u8 *handyman) {
    const u32 walked = Sprite_WalkNav(handyman);
    if (walked == 0) {
        // (the debug check of an empty thing table left out)
        u8 *const job = global<u8 *>(0x007cfb90 + 20 * (u32)at<u16>(handyman, 0x21a));
        if (at<u16>(job, 0x36) == (u16)(((u32)handyman[7] << 7) + handyman[5] + 1)) {
            u16 handle, tracked;
            const u16 *const mine = Thing_GetHandle(handyman, nullptr, &handle);
            Handyman_SetState(handyman, nullptr, *Obj_GetTrackedHandle(job, nullptr, &tracked) == *mine ? 0xb : 0);
            return;
        }
        Handyman_SetState(handyman, nullptr, 0);
        return;
    }
    if (walked == 2) Handyman_SetState(handyman, nullptr, 0);
}

// The staff's spread record moved with them: Sprite_RecordMapId's last cell against the one they stand on now; the map takes the record
// back from the last and spreads it round the new one (only the low words count; the original passes stack dwords).
static void moveSpread(u8 *person, u32 record) {
    u16 last;
    Sprite_RecordMapId(person, nullptr, &last);
    const u16 cell = (u16)(((u32)person[7] << 7) + person[5] + 1);
    if (last != cell) {
        Map_SpreadRemove(global<u8 *>(0x008023a0), nullptr, record, last);
        Map_SpreadAdd(global<u8 *>(0x008023a0), nullptr, record, cell);
    }
    Person_TickSpeed(person);
}

void __fastcall Entertainer_Update(u8 *entertainer) { moveSpread(entertainer, 0); }

void __fastcall Guard_Update(u8 *guard) { moveSpread(guard, 3); }

void __fastcall Researcher_SetState(u8 *researcher, void *, u32 state) {
    if (state == 0xf) {
        at<u32>(researcher, 0x214) = global<u32>(global<u32>(0x0080239c) + 0x1da70c);
        Set4_4217f0(researcher + 0xc, nullptr, 0xa);
    } else {
        Staff_SetState(researcher, nullptr, state);
    }
    at<u32>(researcher, 0x19c) = state;
}

void __fastcall Staff_HeadForEntrance(u8 *staff) {
    const u32 roll = static_cast<u32>(World_Rand(global<u8 *>(0x0080239c)));
    if (!(roll & 7)) {
        const u32 value = (roll & 0xff) + at<u32>(staff, 0x1c) - 0x80;
        at<u32>(staff, 0x1c) = value > 0x7ff ? 0x7ffu : value;   // (unsigned: below 0 too)
    }
    if (Get1da710_51a280(global<u8 *>(0x0080239c))) {
        GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c));   // (its answer handed in ECX, which the count does not read)
        if (!Things_CountKind1OnPlainCell())
            Obj_ClearIndexed28(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(global<u8 *>(0x0080239c))), nullptr,
                               Byte2_ToIndex(staff));
    }
    const u32 index = Byte2_ToIndex(staff);
    if (Obj_GetIndexed28(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da718_519450(global<u8 *>(0x0080239c))), nullptr, index))
        return;
    // (the cell is the low word of the dword slot that held the pushed ECX: its high word the staff pointer's)
    const u16 cell = static_cast<u16>((global<u32>(0x007855c0) << 7) + global<u32>(0x007855bc) + 1);
    const u32 arg = (static_cast<u32>(reinterpret_cast<uintptr_t>(staff)) & 0xffff0000u) | cell;
    Staff_SetState(staff, nullptr, Sprite_SetDestinationCell(staff, nullptr, arg) ? 1u : 0u);   // (0: after the original's debug print)
}

void __fastcall Staff_LeaveRestArea(u8 *staff) {
    u16 handle;
    u32 msg[4] = {0x00700868, 0, *Thing_GetHandle(staff, nullptr, &handle), 0};   // (the original leaves the word above the handle as it was)
    Object_PostMessage(reinterpret_cast<u8 *>(msg));
    const u16 area = *reinterpret_cast<u16 *>(staff + 0x208);
    const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + area * 20u);
    u8 *const record = Rides_FindById(*reinterpret_cast<const s32 *>(thing + 0x24));
    Rides_SetElement(record, 0, Rides_GetElement(record, 0) - 1);
    *reinterpret_cast<u16 *>(staff + 0x208) = 0;
    ScriptRef_Start(staff + 0xc, nullptr, 3, *reinterpret_cast<u32 *>(staff + 0x24), *reinterpret_cast<u32 *>(staff + 0x20));
    Staff_SetState(staff, nullptr, 0);
}

namespace {
const StaffWalkEnv kGameWalkEnv = {&Sprite_WalkNav, &Sprite_SpawnParticle, &Thing_Delete};
const StaffWalkEnv *g_walk = &kGameWalkEnv;
}  // namespace

void Staff_SetWalkEnv(const StaffWalkEnv *env) { g_walk = env ? env : &kGameWalkEnv; }

void __fastcall Staff_WalkToRestArea(u8 *staff) {
    const u32 walked = g_walk->walkNav(staff);
    if (walked == 0) {
        // (Debug_Printf of whether slot 0's thing is null: empty)
        const u16 area = *reinterpret_cast<const u16 *>(staff + 0x208);
        const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + area * 20u);
        u8 *const record = Rides_FindById(*reinterpret_cast<const s32 *>(thing + 0x24));
        Rides_SetElement(record, 0, Rides_GetElement(record, 0) + 1);
        Staff_SetState(staff, nullptr, 3);
        u16 handle;
        u32 msg[4] = {0x00700868, 0, *Thing_GetHandle(staff, nullptr, &handle), 1};   // (the original leaves the word above the handle as it was)
        Object_PostMessage(reinterpret_cast<u8 *>(msg));
    } else if (walked == 2) {
        const X87Reg rest = X87Reg(-(double)*reinterpret_cast<const s32 *>((uintptr_t)0x00785328)) + *reinterpret_cast<const float *>(staff + 0x1f8);
        float &energy = *reinterpret_cast<float *>(staff + 0x1f8);
        if (rest.v > 100.0) energy = 100.0f;
        else if (!(rest.v >= 0.0)) energy = 0.0f;
        else energy = (float)rest;
        Staff_SetState(staff, nullptr, 0);
    }
}

void __fastcall Staff_Detach(u8 *staff) { Person_Detach(staff); }

void __fastcall Mechanic_Detach(u8 *mechanic) {
    Things_Unlink1da73e(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c), nullptr, mechanic);
    const u16 handle = at<u16>(mechanic, 0x218);
    if (handle) {
        u8 *const object = *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + handle * 20u);
        u16 tracked;
        Obj_GetTrackedHandle(object, nullptr, &tracked);   // (compared with the mechanic's handle only in the original's debug print)
        Fn_4e0210(object);
    }
    Staff_Detach(mechanic);
}

void __fastcall Handyman_Detach(u8 *handyman) {
    Things_Unlink1da73c(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c), nullptr, handyman);
    const u16 handle = at<u16>(handyman, 0x21a);
    if (handle) Fn_4e0210(*reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + handle * 20u));
    Staff_Detach(handyman);
}

void __fastcall Entertainer_Detach(u8 *entertainer) {
    Things_Unlink1da740(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c), nullptr, entertainer);
    u8 *const map = *reinterpret_cast<u8 **>((uintptr_t)0x008023a0);
    u16 cell;
    Map_SpreadRemove(map, nullptr, 0, *Sprite_RecordMapId(entertainer, nullptr, &cell));
    Staff_Detach(entertainer);
}

void __fastcall Mechanic_HandleMessage(u8 *mechanic, void *, u8 *message) {
    if (vcall<u32>(message, 0) == 0xa && at<u16>(mechanic, 0x218) == at<u16>(message, 8)) {
        at<u16>(mechanic, 0x218) = 0;
        Mechanic_SetState(mechanic, nullptr, 0);
    }
    Staff_HandleMessage(mechanic, nullptr, message);
}

void __fastcall Handyman_HandleMessage(u8 *handyman, void *, u8 *message) {
    if (vcall<u32>(message, 0) == 0xa && at<u16>(handyman, 0x21a) == at<u16>(message, 8)) {
        const u32 state = at<u32>(handyman, 0x19c);
        at<u16>(handyman, 0x21a) = 0;
        if (state == 0xb) Set4_4217f0(handyman + 0xc, nullptr, 3);
        Handyman_SetState(handyman, nullptr, 0);
    }
    Staff_HandleMessage(handyman, nullptr, message);
}

void __fastcall Guard_HandleMessage(u8 *guard, void *, u8 *message) {
    if (vcall<u32>(message, 0) == 0x1b && at<u16>(message, 8) == at<u16>(guard, 0x212)) at<u16>(guard, 0x212) = 0;
    Staff_HandleMessage(guard, nullptr, message);
}

void __fastcall Staff_HandleMessageDefault(u8 *staff, void *, u8 *message) { Staff_HandleMessage(staff, nullptr, message); }

void __fastcall Researcher_Detach(u8 *researcher) {
    u8 **const game = reinterpret_cast<u8 **>((uintptr_t)0x0080239c);   // (reloaded for each call, as the original does)
    Things_Unlink1da742(*game, nullptr, researcher);
    Staff_Detach(researcher);
    if (GameTable7cfb90_Get1da722_519550(*game))
        ResearchLab_SyncFlag(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da722_519550(*game)));
}

namespace {
s32 handler_wage(const u8 *staff) {   // (as 0x504c70 has it: the grade's whole dword)
    const s32 factor = *reinterpret_cast<const s32 *>((uintptr_t)0x00785380 + Byte2_ToIndex(staff) * 4u);
    return factor * *reinterpret_cast<const s32 *>((uintptr_t)0x0078532c + (*reinterpret_cast<const u32 *>(staff + 0x1e4) << 4));
}

// The wage debited from the park's money and added to the paid total (the wage computed again), as 0x504c70 and 0x505790 have it.
void pay_wage(const u8 *staff) {
    u8 **const game = reinterpret_cast<u8 **>((uintptr_t)0x0080239c);
    Money_Debit(reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(*game)), nullptr, handler_wage(staff));
    s32 *const paid = reinterpret_cast<s32 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*game) + 0x1f7f0);
    *paid += handler_wage(staff);
}
}  // namespace

void __fastcall Staff_Dismiss(u8 *staff) {
    drop_staff_mode();
    pay_wage(staff);
    g_walk->spawnParticle(staff, nullptr, 0x13);
    g_walk->destroy(staff);
}

void __fastcall Staff_HandleMessage(u8 *staff, void *, u8 *message) {
    const u32 kind = vcall<u32>(message, 0);
    if (kind == 0xc) {
        pay_wage(staff);
        return;
    }
    if (kind != 0xa || *reinterpret_cast<u16 *>(staff + 0x208) != *reinterpret_cast<const u16 *>(message + 8)) return;
    const u32 state = *reinterpret_cast<const u32 *>(staff + 0x19c);
    if (state == 3) {
        Staff_LeaveRestArea(staff);
        u16 area = static_cast<u16>(reinterpret_cast<uintptr_t>(message));   // (the out word is the original's argument slot)
        Things_NearestRestArea(staff, nullptr, &area);
        if (!area) return;
        const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + area * 20u);
        if (Sprite_SetDestinationCell(staff, nullptr, *reinterpret_cast<const u16 *>(thing + 0x36)))
            *reinterpret_cast<u16 *>(staff + 0x208) = area;   // (either way after the original's debug print)
    } else if (state == 2) {
        *reinterpret_cast<u16 *>(staff + 0x208) = 0;
        Staff_SetState(staff, nullptr, 0);
    }
}

namespace {
s32 __fastcall game_sumA(u8 *record, void *, u32 n) { return (s32)ParkAnalyser_CountWith2c(record, nullptr, n); }
s32 __fastcall game_sumB(u8 *record, void *, u32 n) { return (s32)ParkAnalyser_CountType1Or3Or9(record, nullptr, n); }
void __fastcall game_sendGuard(u8 *guard, void *, u32 handle) {
    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>((uintptr_t)0x004d6260)(guard, nullptr, handle);
}
const StaffAdviceEnv kGameAdviceEnv = {&game_sumA, &game_sumB, &game_sendGuard};
const StaffAdviceEnv *g_advice = &kGameAdviceEnv;
const u32 kKindHeads[5] = {0x1da73c, 0x1da73e, 0x1da740, 0x1da744, 0x1da742};

// The chain at the world's head word: more than 3 members and the average of the low bytes of their truncated floats at field below 15.
bool chain_low(u32 head, u32 field) {
    u32 count = 0, sum = 0;
    for (u16 h = *reinterpret_cast<const u16 *>(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c) + head); h;) {
        ++count;
        const u8 *const thing = *reinterpret_cast<u8 **>((uintptr_t)0x007cfb90 + h * 20u);
        sum += (u32)(s32)(long long)X87Reg(*reinterpret_cast<const float *>(thing + field)).v & 0xff;   // (__ftol 0x67a830)
        h = *reinterpret_cast<const u16 *>(thing + 0x210);
    }
    return count > 3 && sum / count < 15;
}
}  // namespace

void Staff_SetAdviceEnv(const StaffAdviceEnv *e) { g_advice = e ? e : &kGameAdviceEnv; }

u32 __fastcall StaffAdvice_KindLow(const u8 *self, void *, u32 kind) {
    if (*reinterpret_cast<const u32 *>(self + 0x64)) return 1;
    if (kind > 4) return 0;
    if (kind == 0) {
        u8 *const record = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c)));
        const float a = (float)g_advice->sumA(record, nullptr, 1);   // (fild, fstp m32)
        const X87Reg ratio = a / X87Reg((double)g_advice->sumB(record, nullptr, 1));   // (fild, fdivr m32)
        if (ratio.v > (double)0.2f) return 1;   // (fcomp [0x7008dc]: not when below, equal or unordered)
    }
    if (chain_low(kKindHeads[kind], 0x1fc)) return 1;
    return chain_low(kKindHeads[kind], 0x1f8) ? 1 : 0;
}

void __fastcall StaffAdvice_Update(u8 *self, void *, u32 kind) {
    FileTime out;
    const FileTime *const age = FunnyTime_Elapsed(**reinterpret_cast<u8 ***>((uintptr_t)0x007cf6ec) + 0x2a0, nullptr, &out);
    if ((s32)((long long)((unsigned long long)age->high << 32 | age->low) / 25920000000000LL) < 0x18) return;   // (_alldiv by 30 days)
    u32 *const state = reinterpret_cast<u32 *>(self + 0x24 + 12 * kind);
    u32 msg[4];
    if (StaffAdvice_KindLow(self, nullptr, kind)) {
        const u32 now = state[0];
        if (now > 4) return;
        state[0] = now == 0 ? 1 : now == 4 ? 4 : now + 1;
        if (now) state[1] = 1;
        Object_PostMessage(ThingMsg17_Ctor(reinterpret_cast<u8 *>(msg), nullptr, kind, now ? 2 : 0));
        return;
    }
    if (state[0] == 1) Object_PostMessage(ThingMsg17_Ctor(reinterpret_cast<u8 *>(msg), nullptr, kind, 3));
    state[0] = 0;
    state[1] = 0;
}

void __fastcall StaffAdvice_Tick(u8 *self) {
    Staff_TrainAll(reinterpret_cast<const s32 *>(self + 0xc));
    if (!at<u32>(self, 0x64)) {
        if (Get1da710_51a280(global<u8 *>(0x0080239c))) return;   // (the original's debug print)
        u8 *const analyser = reinterpret_cast<u8 *>((uintptr_t)GameTable7cfb90_Get1da720_519510(global<u8 *>(0x0080239c)));
        if (ParkAnalyser_CountKind1OnPlainCell(analyser, nullptr, 5)) return;   // (the same print)
    }
    static const u32 kHeads[5] = {0x1da73c, 0x1da73e, 0x1da740, 0x1da744, 0x1da742};
    for (u32 kind = 0; kind < 5; ++kind) {
        const u8 *const world = global<u8 *>(0x0080239c);
        if (!at<u16>(world, kHeads[kind])) continue;
        u32 *const state = reinterpret_cast<u32 *>(self + 0x24 + 12 * kind);
        const u32 month = at<u32>(world, 0x1da70c);
        if (state[2] == month) continue;
        state[2] = month;
        if (state[1]) state[1] = 0;
        else StaffAdvice_Update(self, nullptr, kind);
    }
}

void __fastcall StaffHQ_HandleMessage(u8 *self, void *, u8 *message) {
    const u32 type = (*reinterpret_cast<u32(__fastcall *const *const *)(u8 *)>(message))[0](message);
    if (type == 0xc) {
        StaffAdvice_Tick(self);
        return;
    }
    if (type != 0xe) return;
    // (sic: the handle's argument slot keeps the message pointer's high word, as the original's stack slot does)
    const u32 handle = at<u16>(message, 8) | ((u32)(uintptr_t)message & 0xffff0000u);
    const u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    const u32 pos = (((u32)thing[7] << 7) + thing[5] + 1) & 0xffff;
    if (at<s16>(Map_RECell(global<u8 *>(0x008023a0), nullptr, pos), 6) <= 0) return;
    u16 guard = 0;
    Guard_NearestFree(self, nullptr, &guard, pos);
    if (!guard) return;
    g_advice->sendGuard(global<u8 *>(0x007cfb90 + 20 * (u32)guard), nullptr, handle);
}
}
