#include "registry.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../model/highlight.h"
#include "../world/objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u8 *coaster_at(u32 index) { return reinterpret_cast<u8 *const *>(0x00790bd0)[index]; }
const u8 *first_link() { return *reinterpret_cast<u8 *const *>(0x00790fe0); }

}  // namespace

extern "C" {

u32 __cdecl Coaster_Count() { return *reinterpret_cast<const u32 *>(0x00792214); }

u32 __cdecl CoasterLink_Nodes(u32 n, u32 *value) {
    const u8 *link = first_link();
    for (; n != 0; --n) link = at<const u8 *>(link, 0x18);
    if (value) *value = at<u32>(link, 0x28);
    return at<u32>(at<const u8 *>(at<const u8 *>(link, 0x54), 0x90), 4);
}

u32 __cdecl CoasterLink_Find(u32 key) {
    for (const u8 *link = first_link(); link; link = at<const u8 *>(link, 0x18))
        if (at<u32>(link, 0x24) == key) return at<u32>(link, 0x14);
    return 0;
}

void __cdecl Coaster_GetPair(u32 index, u32 *a, u32 *b) {
    const u8 *coaster = coaster_at(index);
    *a = at<u32>(coaster, 0x100);
    *b = at<u32>(coaster, 0x104);
}

void __cdecl Ride_EntryPosition(u32 index, const u32 *slot, float *out) {
    const u8 *ride = reinterpret_cast<u8 *const *>(0x007a4610)[index];
    const u8 *entries = at<const u8 *>(at<const u8 *>(at<const u8 *>(ride, 8), 0x28), 4);
    const u8 *object = at<const u8 *>(entries, *slot * 0x14 + 4);
    u32 *dst = reinterpret_cast<u32 *>(out);
    if (!object) {
        dst[2] = dst[1] = dst[0] = 0;
        return;
    }
    dst[0] = at<u32>(object, 0x30);
    dst[1] = at<u32>(object, 0x34);
    dst[2] = at<u32>(object, 0x38);
}

u8 *__cdecl CoasterLinks_FindTile(u32 x, u32 y, u32 *key) {
    const u8 *map = at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x007a0854), 0x6c);
    const u32 tile = at<u32>(map, 0x18) * y + x;
    u8 *found = CoasterLinks_FindKey(tile);
    if (key) *key = tile;
    return found;
}

u8 *__cdecl CoasterLinks_FindKey(u32 tile) {
    for (const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fe0); link; link = at<const u8 *>(link, 0x18))
        for (u8 *found = at<u8 *>(link, 0x4c); found; found = at<u8 *>(found, 0x28))
            if (!(at<u8>(found, 4) & 5) && at<u32>(found, 8) == tile) return found;
    return nullptr;
}

u32 __cdecl CoasterLink_EndMarked() {
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    if (!link || !(at<u8>(at<const u8 *>(link, 4), 0) & 2)) return 0;
    const u32 flags = at<u32>(link, 0x3c);
    if (flags & 1) return (flags & 2) && (at<u8>(at<const u8 *>(link, 0x60), 8) & 0x80);
    return (at<u8>(at<const u8 *>(link, 0x58), 8) & 0x80) != 0;
}

void __cdecl CoasterLink_CheckEnd() {
    if (!CoasterLink_EndMarked()) return;
    *reinterpret_cast<u32 *>(0x0079c664) = 1;
    *reinterpret_cast<u32 *>(0x0079c658) |= 0x40;
}

u8 *__cdecl CoasterLink_FindCarOnRide(u32 ride) {
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    if (!at<u32>(link, 0x54)) return nullptr;
    const u32 *rides = reinterpret_cast<const u32 *>(0x007a4610);
    for (const u8 *train = at<const u8 *>(link, 0x4c); train; train = at<const u8 *>(train, 0x28)) {
        if (at<u8>(train, 4) & 1) continue;
        for (u32 i = 0, n = at<u32>(train, 0x30); i < n; ++i) {
            u8 *car = at<u8 *>(train, 0x34 + 4 * i);
            if (rides[at<u32>(car, 0x70)] == ride) return car;
        }
    }
    return nullptr;
}

u32 __cdecl CoasterLink_BeginEnd() {
    u32 &flags = *reinterpret_cast<u32 *>(0x0079c658);
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    if (!(flags & 0x40) && link && (at<u8>(at<const u8 *>(link, 4), 0) & 2)) {
        *reinterpret_cast<u32 *>(0x0079c664) = 2;
        flags |= 0x40;
    }
    return 1;
}

u32 __cdecl Table7ae6a8_Bit18(u32 index) {
    return at<u32>(reinterpret_cast<const u8 *const *>(0x007ae6a8)[index], 4) >> 18 & 1;
}

u32 __cdecl Ride_Bit18(u32 index) {
    return at<u32>(reinterpret_cast<const u8 *const *>(0x007a4610)[index], 4) >> 18 & 1;
}

u8 *__cdecl CoasterLink_ById(u32 id) {
    for (u8 *link = *reinterpret_cast<u8 *const *>(0x00790fe0); link; link = at<u8 *>(link, 0x18))
        if (at<u32>(link, 0x10) == id) return link;
    return nullptr;
}

u32 __cdecl CoasterLink_IdValue(u32 id) {
    const u8 *link = CoasterLink_ById(id);
    if (!link || !(at<u32>(link, 0) & 0x800)) return 0;
    return at<u32>(at<const u8 *>(at<const u8 *>(link, 0x124), 0x34), 0xc);
}

void __cdecl CoasterLink_SetById(u32 id, u32 value) {
    if (u8 *link = CoasterLink_ById(id)) at<u32>(link, 0x28) = value;
}

void __cdecl CoasterLink_SetEnd() {
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    if (!link || !(at<u8>(at<const u8 *>(link, 4), 0) & 2)) return;
    *reinterpret_cast<u32 *>(0x0079c664) = 1;
    *reinterpret_cast<u32 *>(0x0079c658) |= 0x40;
}

void __cdecl CoasterLink_ClearEnd() {
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    if (!link || !(at<u8>(at<const u8 *>(link, 4), 0) & 2)) return;
    *reinterpret_cast<u32 *>(0x0079c664) = 0;
    *reinterpret_cast<u32 *>(0x0079c658) &= ~0x40u;
}

u8 *__cdecl Coaster_SelectedCar() { return *reinterpret_cast<u8 *const *>(0x0079c640); }

void __cdecl Coaster_SelectCar(u8 *car) { *reinterpret_cast<u8 **>(0x0079c640) = car; }

u8 *__cdecl Object_CoasterLink(const u8 *object) {
    return object ? CoasterLink_ById(at<u32>(object, 0x20)) : nullptr;
}

u32 __cdecl Object_LinkOpen(const u8 *object) {
    const u8 *link = Object_CoasterLink(object);
    return link && (at<u32>(link, 0x3c) & 3) == 1;
}

u32 __cdecl Object_LinkBare(const u8 *object) {
    const u8 *link = Object_CoasterLink(object);
    return link && !at<u32>(link, 0x140);
}

void __cdecl Coasters_GetState(u32 *a, u32 *b, u32 *c, u32 *d) {
    *a = *reinterpret_cast<const u32 *>(0x0079221c);
    *b = *reinterpret_cast<const u32 *>(0x00790fd8);
    *c = *reinterpret_cast<const u32 *>(0x0079104c);
    *d = *reinterpret_cast<const u32 *>(0x00791cf8);
}

u32 __cdecl CoasterEdit_Flag1() { return *reinterpret_cast<const u32 *>(0x0079c658) & 1; }

void __cdecl CoasterLink_SetCurrent(u8 *link) { *reinterpret_cast<u8 **>(0x00790fec) = link; }

u32 __cdecl CoasterEdit_Tools(u32 *out) {
    u32 local;
    if (!out) out = &local;
    *out = 0x1fc;
    if (!CoasterLink_EndMarked()) *out |= 1;
    const u8 *link = *reinterpret_cast<const u8 *const *>(0x00790fec);
    const u8 *type = at<const u8 *>(link, 4);
    if (at<u8>(type, 0) & 0x80) {
        *out |= 2;
        if (at<u32>(type, 0) & 0x100) *out |= 0x200;
    }
    *out &= (at<u32>(link, 0x3c) & 3) == 1 ? ~3u : ~0x40u;
    *reinterpret_cast<u32 *>(0x0079c6e0) = *out;
    return *out;
}

u32 __cdecl Object_LinkOtherOpen(const u8 *object) {
    if (!object) return 0;
    const u8 *link = CoasterLink_ById(at<u32>(object, 0x20));
    if (!link || at<u32>(link, 0x140)) return 0;
    const u32 flags = at<u32>(link, 0x3c);
    if (!(flags & 1) || (flags & 2)) return 0;
    return link != *reinterpret_cast<const u8 *const *>(0x00790fec);
}

void __cdecl Object_LinkParams(const u8 *object, u32 *a, u32 *b, u32 *c, u32 *d) {
    const u8 *link = Object_CoasterLink(object);
    *a = at<u32>(link, 0x10c);
    *b = at<u32>(link, 0x110);
    *c = at<u32>(link, 0x114);
    *d = at<u32>(link, 0x108);
}

void __cdecl Coasters_ResetUnit() { *reinterpret_cast<float *>(0x00792404) = 1.0f; }

void __cdecl Coasters_InitEpsilon() { *reinterpret_cast<float *>(0x007925e0) = 1.0f / 8388608.0f; }

void __cdecl Coasters_AllocTables(u32 nodes, u32 links) {
    *reinterpret_cast<u32 *>(0x00792400) = links;
    *reinterpret_cast<void **>(0x007925dc) = GlobalMem_Alloc(links * 0x1c, 1);
    *reinterpret_cast<void **>(0x00792408) = GlobalMem_Alloc(nodes * 0x10, 1);
}

void __cdecl Coaster_FlashCar(u8 *coaster, u32 which) {
    u8 *const *const rides = reinterpret_cast<u8 *const *>(0x007a4610);
    if (const u8 *const old = *reinterpret_cast<const u8 *const *>(coaster + 0x144)) {
        u8 *const ride = rides[*reinterpret_cast<const u32 *>(old + 0x70)];
        if (*reinterpret_cast<const u32 *>(ride + 4) & 0x4000000) Highlight_Unflash(ride);
        *reinterpret_cast<u8 **>(coaster + 0x144) = nullptr;
    }
    const bool other = *(coaster + 0x3c) & 2;
    u8 *car = nullptr;
    if ((u8)which & 1)
        car = *reinterpret_cast<u8 *const *>(coaster + (other ? 0x60 : 0x58));
    else if ((u8)which & 2)
        car = *reinterpret_cast<u8 *const *>(coaster + (other ? 0x64 : 0x54));
    if (!car) return;
    u8 *const ride = rides[*reinterpret_cast<const u32 *>(car + 0x70)];
    if (!(*reinterpret_cast<const u32 *>(ride + 4) & 0x4000000)) Highlight_Flash(ride);
    *reinterpret_cast<u8 **>(coaster + 0x144) = car;
}

u32 __cdecl Ride_Claim(u32 index, u32 type, u32 key, const u32 *desc, u32, u32 value, u32 a, u32 b) {
    if (!a) a = desc && desc[3] ? reinterpret_cast<const u32 *>(desc[3])[0] : 0;
    if (!b) b = desc && desc[3] ? reinterpret_cast<const u32 *>(desc[3])[1] : 0;
    u8 *const *const rides = reinterpret_cast<u8 *const *>(0x007a4610);
    const u8 *const *const types = reinterpret_cast<const u8 *const *>(0x007ae6a8);
    const u8 *def = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, (u16)at<u32>(types[type], 0xf8));
    u8 *ride;
    if (index) {
        ride = rides[index];
    } else {
        const u32 count = *reinterpret_cast<const u32 *>(0x007a3ebc) + *reinterpret_cast<const u32 *>(0x007a4084);
        u32 i = 1;
        for (;; ++i) {
            if (i > count) return 0;
            ride = rides[i];
            if (!ride) continue;
            u32 t = *reinterpret_cast<const u32 *>(0x007a439c);
            for (; t != 0; --t)
                if (reinterpret_cast<const u32 *>(types)[t] == at<u32>(ride, 0)) break;
            if (at<u32>(ride, 4) & 0x10000 || t != type || at<u32>(ride, 0xe4) != key) continue;
            if (!desc) break;
            if (at<u32>(ride, 0xe8) != desc[0] || at<u32>(ride, 0xec) != desc[1] ||
                at<u32>(at<const u8 *>(ride, 8), 0x10) != desc[2])
                continue;
            if (!desc[3]) break;
            if (def) {
                if (at<u32>(def, 0x18) == desc[3]) break;
            } else if (at<u32>(ride, 0xf0) == a && at<u32>(ride, 0xf4) == b) {
                break;
            }
        }
        index = i;
    }
    if (at<u32>(ride, 4) & 0x40000) CoasterLink_SetById(index, value);
    at<u32>(ride, 4) |= 0x10000;
    return index;
}
}
