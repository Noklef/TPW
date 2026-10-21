#include "queue.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../model/partstate.h"
#include "../world/ridelist.h"
#include "../world/things.h"
#include "../world/thingscan.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u8 *coaster_at(u32 index) { return reinterpret_cast<u8 **>(0x00790bd0)[index]; }
u8 *queue(u8 *coaster) { return coaster + 0xb0; }
u8 *boarding(u8 *coaster) { return coaster + 0xd0; }

// The slot after p: buffer + ((p - buffer) / 4 + 1) mod capacity (the byte difference
// shifted as signed, divided unsigned).
u32 *next(const u8 *ring, const u32 *p) {
    u32 *buffer = at<u32 *>(ring, 4);
    const u32 slot = (u32)((s32)((uintptr_t)p - (uintptr_t)buffer + 4) >> 2);
    return buffer + slot % at<u32>(ring, 0);
}

void push(u8 *ring, u32 peep) {
    if (!(at<u32>(ring, 0x10) + at<u32>(ring, 0xc) < at<u32>(ring, 0))) return;
    *at<u32 *>(ring, 0x18) = peep;
    at<u32 *>(ring, 0x18) = next(ring, at<u32 *>(ring, 0x18));
    ++at<u32>(ring, 0xc);
}

u32 take(u8 *ring) {
    u32 *head = at<u32 *>(ring, 0x14);
    const u32 peep = *head;
    at<u32 *>(ring, 0x14) = next(ring, head);
    --at<u32>(ring, 0xc);
    return peep;
}

}  // namespace

extern "C" {

void __cdecl Coaster_QueuePush(u32 index, u32 peep) { push(queue(coaster_at(index)), peep); }

void __cdecl CoasterQueue_Add(u8 *coaster, u32 peep) {
    u8 *ring = queue(coaster);
    if (at<u32>(ring, 0x10) + at<u32>(ring, 0xc) >= at<u32>(ring, 0)) ring = boarding(coaster);
    push(ring, peep);
}

s32 __cdecl Coaster_QueueRoom(u32 index) {
    const u8 *ring = queue(coaster_at(index));
    const s32 room = at<s32>(ring, 8) - at<s32>(ring, 0x10) - at<s32>(ring, 0xc);
    return room < 0 ? 0 : room;
}

u32 __cdecl Coaster_BoardingPop(u32 index) {
    u8 *ring = boarding(coaster_at(index));
    if (at<u32>(ring, 0xc) == 0 || at<u32 *>(ring, 0x14) == at<u32 *>(ring, 0x1c)) return 0;
    return take(ring);
}

void __cdecl CoasterQueue_Advance(u8 *coaster) {
    u8 *from = queue(coaster), *to = boarding(coaster);
    const s32 room = at<s32>(to, 0) - at<s32>(to, 0x10) - at<s32>(to, 0xc);
    const s32 waiting = at<s32>(from, 0x10) + at<s32>(from, 0xc);
    s32 n = room < waiting ? room : waiting;
    for (n = n < 0 ? 0 : n; n > 0; --n) {
        const u32 peep = at<u32>(from, 0xc) ? take(from) : 0;
        if (peep) push(to, peep);
    }
}

void __cdecl CoasterTrain_Unload(u8 *coaster, u8 *train, u32 side) {
    u8 *car = at<u8 *>(train, 0x28) + at<u32>(train, 8) * 0x60;
    for (u32 i = 0; i < at<u32>(train, 0xc); ++i, car += 0x60) {
        u32 *const riders = at<u32 *>(car, side ? 0x28 : 0x2c);
        u32 &count = at<u32>(car, side ? 0x30 : 0x34);
        for (u32 j = 0; j < count; ++j) {
            push(boarding(coaster), riders[j]);
            u8 *const ride = reinterpret_cast<u8 *const *>(0x007a4610)[at<u32>(car, 0xc)];
            PartStates_Release(at<u8 *>(ride, 8), at<const u32 *>(at<u8 *>(car, 4), 0x2c)[j]);
            --at<u32>(train, 0x10);
            riders[j] = 0;
        }
        count = 0;
    }
}

void __cdecl CoasterRing_Init(u8 *ring, u32 capacity) {
    at<u32>(ring, 0) = capacity;
    at<u32>(ring, 8) = 0;
    at<u32>(ring, 0xc) = 0;
    at<u32>(ring, 0x10) = 0;
    u32 *const buffer = static_cast<u32 *>(GlobalMem_Alloc(capacity * 4, 1));
    at<u32 *>(ring, 0x1c) = nullptr;
    at<u32 *>(ring, 4) = buffer;
    at<u32 *>(ring, 0x14) = buffer;
    at<u32 *>(ring, 0x18) = buffer;
}

void __cdecl CoasterRing_Free(u8 *ring) { GlobalMem_Free(at<u32 *>(ring, 4)); }

void __cdecl Coaster_RequeueWaitingPeeps(s32 id) {
    u8 *coaster = *reinterpret_cast<u8 **>((uintptr_t)0x00790fe0);
    while (coaster && at<s32>(coaster, 0x10) != id) coaster = at<u8 *>(coaster, 0x18);
    if (!coaster) return;
    u16 handle;
    const u16 target = *Thing_GetHandle(at<u8 *>(coaster, 0x28), nullptr, &handle);
    u32 first = 0, second = 0;   // (sic: uninitialised in the original when there is no record)
    if (const u8 *record = Rides_FindById(at<s32>(coaster, 0x24))) {
        first = Rides_GetElement(record, 0);
        second = Rides_GetElement(record, 1);
    }
    for (u8 *peep = Things_FindNextOfCategory(0x40, nullptr); peep; peep = Things_FindNextOfCategory(0x40, peep)) {
        if (at<u16>(peep, 0x1dc) != target || at<u32>(peep, 0x220) != 0x10) continue;
        const u16 self = *Thing_GetHandle(peep, nullptr, &handle);
        if (self == first || self == second) continue;
        bool riding = false;
        const u8 *train = at<u8 *>(coaster, 0x128);
        for (u32 t = 0; t < at<u32>(coaster, 0xf4) && !riding; ++t, train += 0x80) {
            const u8 *car = at<u8 *>(train, 0x28);
            for (u32 c = 0; c < at<u32>(train, 0xc) && !riding; ++c, car += 0x60) {
                for (u32 k = 0; k < at<u32>(car, 0x30) && !riding; ++k)
                    riding = at<u16>(at<u8 *>(car, 0x28), 4 * k) == self;
                for (u32 k = 0; k < at<u32>(car, 0x34) && !riding; ++k)
                    riding = at<u16>(at<u8 *>(car, 0x2c), 4 * k) == self;
            }
        }
        if (!riding) CoasterQueue_Add(coaster, self);
    }
}
}
