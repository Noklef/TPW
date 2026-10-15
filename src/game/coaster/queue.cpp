#include "queue.h"

#include <cstdint>

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

u32 __cdecl CoasterLink_Find(u32 key) {
    for (const u8 *link = *reinterpret_cast<u8 **>(0x00790fe0); link; link = at<const u8 *>(link, 0x18))
        if (at<u32>(link, 0x24) == key) return at<u32>(link, 0x14);
    return 0;
}

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
}
