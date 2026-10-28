#include "registry.h"

#include <cstdint>
#include <cstring>

#include "../core/actionrec.h"
#include "../core/globalmem.h"
#include "../core/idtable.h"
#include "../math/x87.h"
#include "../model/highlight.h"
#include "../model/holder.h"
#include "../model/parts.h"
#include "../motion/animator.h"
#include "../terrain/land.h"
#include "../ui/dialog7ca1c8.h"
#include "../world/objectdefs.h"
#include "../world/queuepieces.h"
#include "../world/ridestate.h"
#include "../world/trackline.h"
#include "../world/cellmessage.h"
#include "cursor.h"
#include "cars.h"
#include "queue.h"
#include "train.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const CoasterLinksEnv kGameEnv = {reinterpret_cast<u32(__cdecl *)(u32, u32, const u32 *, u32, u32, u32, u32, u32, s32, u32)>((uintptr_t)0x00463060),
                                  &Model_FindPartByKey4, &CoasterLink_AddEntry, &CoasterLink_AddCar, &Car_SetChannelRates,
                                  &CoasterEdit_SetBounds, &Coaster_FlashCar, &GlobalMem_Alloc, &IdTable_Add, &GlobalMem_Free,
                                  &CoasterRing_Init, &Coaster_SetState, &Coaster_SetMix, &CoasterLink_SetUpCars, &Coaster_SetupCars,
                                  &CoasterLink_AddFixedPieces};
const CoasterLinksEnv *g_env = &kGameEnv;

const u8 *coaster_at(u32 index) { return reinterpret_cast<u8 *const *>(0x00790bd0)[index]; }
const u8 *first_link() { return *reinterpret_cast<u8 *const *>(0x00790fe0); }

}  // namespace

void CoasterLinks_SetEnv(const CoasterLinksEnv *env) { g_env = env ? env : &kGameEnv; }

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

u8 *__cdecl CoasterLinks_FindAtFocus(s32 *x, s32 *z, u32 *key) {
    const float *focus = reinterpret_cast<const float *>(0x007a1c70);
    const s32 cx = (s32)(long long)(X87Reg(focus[0]) * 0.100000001f).v;  // 0x6fdedc, __ftol
    const s32 cz = (s32)(long long)(X87Reg(focus[2]) * 0.100000001f).v;
    const u8 *map = at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x007a0854), 0x6c);
    const u32 tile = at<u32>(map, 0x18) * cz + cx;
    u8 *found = CoasterLinks_FindKey(tile);
    if (x) *x = cx;
    if (z) *z = cz;
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

void __cdecl CoasterEdit_SetBounds() {
    const u32 flags = *reinterpret_cast<const u32 *>(0x0079c658);
    const u8 *const link = *reinterpret_cast<u8 *const *>(0x00790fec);
    const u8 *const selection = *reinterpret_cast<u8 *const *>(0x0079c6a4);
    const u8 *piece;
    if (at<u8>(link, 0x3c) & 2) {
        if (flags & 0x100) {
            piece = at<const u8 *>(selection, 0x68);
            if (at<u32>(piece, 8) & 0x10000) piece = at<const u8 *>(selection, 0x64);
        } else {
            piece = at<const u8 *>(link, 0x60);
        }
    } else if (flags & 0x100) {
        piece = at<const u8 *>(selection, 0x68);
    } else {
        piece = at<const u8 *>(link, 0x58);
        while (piece && (at<u32>(piece, 8) & 0x2000)) piece = at<const u8 *>(piece, 0x68);
    }
    const u8 *const cell = *reinterpret_cast<u8 *const *>(reinterpret_cast<uintptr_t>(piece) + 0x100);  // (null: faults)
    const u32 x = at<u32>(cell, 0xc), z = at<u32>(cell, 0x10);
    const u8 *const type = at<const u8 *>(link, 4) + (flags & 0x40 ? 0x1c8 : 0xa8);
    const u32 reach = (u32)(long long)X87_Sqrt(X87Reg((double)at<u32>(type, 0xa0))).v + 1;  // (__ftol)
    s32 xLow = (s32)(x - reach), zLow = (s32)(z - reach), xHigh = (s32)(reach + x), zHigh = (s32)(reach + z);
    const s32 origin = *reinterpret_cast<const s32 *>(0x007a01bc);
    if (xLow < origin) xLow = origin;
    if (zLow < origin) zLow = origin;  // sic: the x origin
    const s32 xEnd = (s32)((u32)origin + *reinterpret_cast<const u32 *>(0x007a0314));
    if (xHigh >= xEnd) xHigh = (s32)((u32)xEnd - 1);
    const s32 zEnd = (s32)(*reinterpret_cast<const u32 *>(0x007a029c) + *reinterpret_cast<const u32 *>(0x007a0374));
    if (zHigh >= zEnd) zHigh = (s32)((u32)zEnd - 1);
    *reinterpret_cast<s32 *>(0x00791cf8) = zHigh;
    *reinterpret_cast<s32 *>(0x0079104c) = xHigh;
    *reinterpret_cast<s32 *>(0x0079221c) = xLow;
    *reinterpret_cast<s32 *>(0x00790fd8) = zLow;
}

void __cdecl CoasterEdit_SetTool(u32 tool) {
    u8 **const current = reinterpret_cast<u8 **>(0x00790fec);
    u32 &flags = *reinterpret_cast<u32 *>(0x0079c658);
    u32 &mode = *reinterpret_cast<u32 *>(0x0079c664);
    u32 &selected = *reinterpret_cast<u32 *>(0x0079c644);
    if (tool != 2) {
        ActionRec_Write(reinterpret_cast<ActionRec *>(0x00785a08), nullptr, 0x37);
        at<u32>(*current, 0) &= ~0x2000u;
        if (selected == 2 && mode == 2 && *current && (*at<const u8 *>(*current, 4) & 2)) flags &= ~0x40u;
        mode = 0;
        selected = tool;
        CoasterEdit_SetBounds();
        return;
    }
    ActionRec_Write(reinterpret_cast<ActionRec *>(0x00785a08), nullptr, 0x38);
    at<u32>(*current, 0) &= ~0x2000u;
    const u8 *const link = *current;
    const bool typed = link && (*at<const u8 *>(link, 4) & 2);
    bool flagged = false;
    if (typed) {
        const u32 state = at<u32>(link, 0x3c);
        if (!(state & 1))
            flagged = at<u8>(at<const u8 *>(link, 0x58), 8) & 0x80;
        else if (state & 2)
            flagged = at<u8>(at<const u8 *>(link, 0x60), 8) & 0x80;
    }
    if (flagged) {
        mode = 1;
        flags |= 0x40;
    } else if (!(flags & 0x40) && typed) {
        mode = 2;
        flags |= 0x40;
    }
    selected = 2;
    CoasterEdit_SetBounds();
}

void __cdecl CoasterEdit_Tool1() { CoasterEdit_SetTool(1); }

void __cdecl CoasterEdit_Tool2() { CoasterEdit_SetTool(2); }

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

void __cdecl Coasters_InitTables() {
    Coasters_AllocTables(*reinterpret_cast<const u32 *>(0x00790fd0), *reinterpret_cast<const u32 *>(0x00791cfc));
    IdTable_Init(reinterpret_cast<u32 *>(0x00790b04), reinterpret_cast<u32 *>(0x00790b00),
                 reinterpret_cast<u32 *>(0x00791ce0));
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

u8 *__cdecl CoasterLink_AddEntry(s32 x, s32 y, u32 flags, u32, u32 value, const float *position) {
    u8 *const link = *reinterpret_cast<u8 **>(0x00790fec);
    u8 *const entry = static_cast<u8 *>(GlobalMem_Alloc(0x48, 1));
    if (flags & 4) {
        at<u8 *>(link, 0x68) = entry;
    } else {
        if (!at<u8 *>(link, 0x4c)) at<u8 *>(link, 0x4c) = entry;
        if (u8 *const last = at<u8 *>(link, 0x50)) at<u8 *>(last, 0x28) = entry;
        at<u8 *>(entry, 0x2c) = at<u8 *>(link, 0x50);
        const u32 index = at<u32>(link, 0x40);
        at<u8 *>(link, 0x50) = entry;
        at<u32>(entry, 0) = index;
        ++at<u32>(link, 0x40);
    }
    at<u32>(entry, 4) = flags;
    if (position) {
        const u32 *const bits = reinterpret_cast<const u32 *>(position);   // (copied as dwords)
        at<u32>(entry, 0x1c) = bits[0];
        at<u32>(entry, 0x20) = bits[1];
        at<u32>(entry, 0x24) = bits[2];
        s32 cx = (s32)(long long)(X87Reg(position[0]) * 0.100000001f).v;  // 0x6fdedc, __ftol
        s32 cz = (s32)(long long)(X87Reg(position[2]) * 0.100000001f).v;
        if (cx < 0) cx = 0;
        if (cz < 0) cz = 0;
        if (cx >= *reinterpret_cast<const s32 *>(0x007a0314)) cx = *reinterpret_cast<const s32 *>(0x007a0314) - 1;
        if (cz >= *reinterpret_cast<const s32 *>(0x007a0374)) cz = *reinterpret_cast<const s32 *>(0x007a0374) - 1;
        at<s32>(entry, 0xc) = cx;
        at<s32>(entry, 0x10) = cz;
    } else {
        at<s32>(entry, 0xc) = x;
        at<float>(entry, 0x1c) = (float)X87_MulInt((u32)x, 10.0f);   // fild qword (x, 0); 0x6fde7c
        at<s32>(entry, 0x10) = y;
        at<float>(entry, 0x24) = (float)X87_MulInt((u32)y, 10.0f);
        at<float>(entry, 0x20) = (float)Terrain_FootprintHeight(x, y, nullptr, 1, 1, 0, 0);
        const u8 *const grid = at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x007a0854), 0x6c);
        at<u32>(entry, 8) = at<u32>(grid, 0x18) * y + x;
    }
    at<u8 *>(entry, 0x14) = *reinterpret_cast<u8 **>(0x00790fec);
    at<u32>(entry, 0x18) = value;
    return entry;
}

void __cdecl CoasterEdit_RefreshTools() {
    CoasterEdit_Tools(nullptr);
    void *const owner = reinterpret_cast<void *>(static_cast<uintptr_t>(0x007b51f0));
    Dialog7ca1c8_SetChecksIfUpThis(owner, nullptr, *reinterpret_cast<u32 *>(0x0079c6e0));
    Dialog7ca1c8_SelectButtonIfUpThis(owner, nullptr, *reinterpret_cast<u32 *>(0x0079c638));
}

void __cdecl CoasterLink_RemoveEntry(u8 *link, u8 *entry, u32 keep) {
    if (!(entry[4] & 1) && !keep) TrackCell_ClearAt(at<s32>(entry, 0xc), at<s32>(entry, 0x10));
    if (at<u8 *>(entry, 0x2c)) at<u8 *>(at<u8 *>(entry, 0x2c), 0x28) = at<u8 *>(entry, 0x28);
    if (at<u8 *>(entry, 0x28)) at<u8 *>(at<u8 *>(entry, 0x28), 0x2c) = at<u8 *>(entry, 0x2c);
    if (at<u8 *>(link, 0x4c) == entry) at<u8 *>(link, 0x4c) = at<u8 *>(entry, 0x28);
    if (at<u8 *>(link, 0x50) == entry) at<u8 *>(link, 0x50) = at<u8 *>(entry, 0x2c);
    --at<u32>(link, 0x40);
    u8 *next = at<u8 *>(entry, 0x28);
    if (next) {
        const u8 *const before = at<u8 *>(next, 0x2c);
        u32 index = before ? at<u32>(before, 0) + 1 : 0;
        do {
            at<u32>(next, 0) = index++;
            next = at<u8 *>(next, 0x28);
        } while (next);
    }
    GlobalMem_Free(entry);
    Flags_SetChanged(reinterpret_cast<u32 *>(link), 1);
}

void __cdecl CoasterLink_FreeEntries(const u8 *link) {
    for (u8 *entry = at<u8 *>(link, 0x4c); entry;) {
        if (!(entry[4] & 1)) {
            if (*at<u8 *>(entry, 0x18) & 4)
                for (u32 i = 0; i < at<u32>(entry, 0x30); ++i) Car_Free(at<u8 *>(entry, 0x34 + i * 4), at<u8 *>(entry, 0x18));
            TrackCell_ClearAt(at<s32>(entry, 0xc), at<s32>(entry, 0x10));
        } else {
            u8 *const car = at<u8 *>(entry, 0x34);
            Car_FreeCoupling(car, reinterpret_cast<u32 *>(car + 0x78));
        }
        u8 *const next = at<u8 *>(entry, 0x28);
        GlobalMem_Free(entry);
        entry = next;
    }
}

void __cdecl CoasterLink_SetUpCars(const u8 *train, u8 *link, u32 quiet) {
    const u32 segments = at<u32>(train, 0x2f4);
    const u32 total = segments * at<u32>(train, 0x2e8);
    at<void *>(link, 0x128) = GlobalMem_Alloc(at<u32>(train, 0x2e8) << 7, 1);
    at<void *>(link, 0x12c) = GlobalMem_Alloc(at<u32>(train, 0x2e8) << 2, 1);
    u8 *segment = static_cast<u8 *>(GlobalMem_Alloc(total * 0x60, 1));
    at<u8 *>(link, 0x130) = segment;
    u8 *car = at<u8 *>(link, 0x128);
    u8 *slots = static_cast<u8 *>(GlobalMem_Alloc(at<u32>(train, 0x318) << 3, 1));
    at<u8 *>(link, 0x148) = slots;
    u8 *const *const holders = reinterpret_cast<u8 *const *>(0x007a4610);
    u8 *holder = nullptr;   // (the link argument's slot, kept from segment to segment)
    for (u32 i = 0; i < at<u32>(train, 0x2e8); ++i, car += 0x80) {
        at<const u8 *>(car, 4) = train + 0x2f0;
        at<u32>(car, 0) = 0;
        at<u32>(car, 0xc) = 0;
        at<u8 *>(car, 0x28) = segment;
        at<u32>(car, 0x2c) = 0;
        at<u32>(car, 0x30) = 0;
        at<u32>(car, 0x74) = 0;
        at<u32>(car, 0x78) = 0;
        for (u32 j = 0; j < segments; ++j, segment += 0x60) {
            const u8 *const type = at<u8 *>(train, j == 0 ? 0x2fc : j == segments - 1 ? 0x304 : 0x300);
            at<u32>(segment, 0) = 0;
            at<u32>(segment, 8) = j;
            at<const u8 *>(segment, 4) = type;
            at<u32>(segment, 0x10) = 0;
            at<u32>(segment, 0x14) = 0;
            at<u32>(segment, 0x18) = 0;
            at<u32>(segment, 0x1c) = 0;
            at<u8 *>(segment, 0x20) = slots;
            at<u8 *>(segment, 0x24) = slots += at<u32>(at<u8 *>(segment, 4), 0x30) * 4;
            slots += at<u32>(at<u8 *>(segment, 4), 0x30) * 4;
            at<u8 *>(segment, 0x28) = at<u8 *>(segment, 0x20);
            at<u8 *>(segment, 0x2c) = at<u8 *>(segment, 0x24);
            if (!quiet) {
                const u32 kind = at<u32>(type, 0xc);
                if (kind) {
                    const u32 ride = g_env->createRide(kind, 0x101, nullptr, 0, 0, 0, 0, 0, -1, 0);
                    at<u32>(segment, 0xc) = ride;
                    u8 *const h = holders[ride];
                    if (at<u32>(h, 4) & 0x4000) Obj_RemoveFromRect(at<u8 *>(h, 8), at<u32>(h, 0xc0), at<u32>(h, 0xc4));
                    else Holder_Sleep(at<u8 *>(h, 8));
                    holder = holders[at<u32>(segment, 0xc)];
                } else {
                    holder = nullptr;
                }
            }
            if (holder) Animator_SetPlaying(holder);
        }
        at<u8 *>(car, 0x34) = at<u8 *>(car, 0x28) + (at<u32>(train, 0x2fc) == at<u32>(train, 0x300) ? 0 : 0x60);
    }
}

// A link destroyed (CoasterLink_Destroy's and CoasterLinks_DestroyAll's, inlined in both originals).
static void DestroyLink(u8 *link) {
    u8 **const head = reinterpret_cast<u8 **>(0x00790fe0);
    IdTable_Remove(reinterpret_cast<u32 *>(0x00790b04), reinterpret_cast<u32 *>(0x00790b00), reinterpret_cast<u32 *>(0x00791ce0),
                   reinterpret_cast<u32 *>(0x00790bd0), at<u32>(link, 0x14));
    --*reinterpret_cast<u32 *>(0x00792214);
    const u8 *const train = at<const u8 *>(link, 4);
    u8 *car = at<u8 *>(link, 0x128);   // (read before the slots are freed)
    GlobalMem_Free(at<void *>(link, 0x148));
    at<u32>(link, 0x148) = 0;
    if (at<u32>(link, 0x128)) {
        for (u32 i = 0; i < at<u32>(train, 0x2e8); ++i, car += 0x80) {
            if (car[0] & 2) CoasterCar_Sleep(link, car);
            const u8 *const shape = at<const u8 *>(car, 4);
            u8 *segment = at<u8 *>(car, 0x28) + 0xc;
            for (u32 j = 0; j < at<u32>(shape, 4); ++j, segment += 0x60) {
                if (!at<u32>(segment, 0)) continue;
                Ride_Sleep(at<u32>(segment, 0));
                QueuePiece_Remove(at<u32>(segment, 0));   // (Ride_Destroy 0x463c80, through the queue pieces' env)
            }
        }
    }
    GlobalMem_Free(at<void *>(link, 0x130));
    GlobalMem_Free(at<void *>(link, 0x128));
    GlobalMem_Free(at<void *>(link, 0x12c));
    Owner_DisposeChildren(link);
    CoasterLink_FreeEntries(link);
    if (*head == link) {
        *head = at<u8 *>(link, 0x18);
    } else {
        u8 *prev = nullptr;
        for (u8 *node = *head; node; node = at<u8 *>(node, 0x18)) {
            if (node == link) at<u8 *>(prev, 0x18) = at<u8 *>(node, 0x18);   // (the walk goes on past it)
            prev = node;
        }
    }
    CoasterRing_Free(link + 0xb0);
    CoasterRing_Free(link + 0xd0);
    GlobalMem_Free(link);
}

void __cdecl CoasterLink_Destroy(u32 id) {
    u8 **const head = reinterpret_cast<u8 **>(0x00790fe0);
    u8 *link = *head;
    while (link && at<u32>(link, 0x10) != id) link = at<u8 *>(link, 0x18);
    if (!link) return;
    DestroyLink(link);
}

void __cdecl CoasterLinks_DestroyAll() {
    u8 **const head = reinterpret_cast<u8 **>(0x00790fe0);
    for (u8 *link = *head; link;) {
        u8 *const next = at<u8 *>(link, 0x18);   // (read first)
        DestroyLink(link);
        link = next;
    }
    *head = nullptr;
}
u8 *__cdecl CoasterLink_AddCar(u8 *train, u32 a, u32 b, float height, u32 flags, const u8 *desc, u32 half, u32 slot, u32 claim) {
    u8 *const link = *reinterpret_cast<u8 *const *>(0x00790fec) + 0x3c;   // (kept from +0x3c)
    u8 *const piece = static_cast<u8 *>(GlobalMem_Alloc(0x104, 1));
    const u32 spare = flags & 2;
    if (spare) at<u32>(piece, 8) |= 0x2000;
    const bool wide = desc && (at<u32>(desc, 0) & 0x4000);
    if (!wide && !spare) at<u8 *>(link, 0x20) = piece;
    if (link[0] & 2) {
        u8 *const prev = at<u8 *>(link, 0x24);
        at<u32>(prev, 8) &= 0xfffaffffu;
        at<u32>(piece, 8) |= 0x50000;
        at<u8 *>(piece, 0x68) = at<u8 *>(link, 0x24);
        at<u8 *>(piece, 0x64) = at<u8 *>(at<u8 *>(link, 0x24), 0x64);
        at<u8 *>(at<u8 *>(piece, 0x68), 0x64) = piece;
        at<u8 *>(at<u8 *>(piece, 0x64), 0x68) = piece;
        at<u8 *>(link, 0x24) = piece;
    } else if (spare) {
        at<u8 *>(link, 0x30) = piece;
    } else {
        if (!at<u8 *>(link, 0x18)) at<u8 *>(link, 0x18) = piece;
        if (at<u8 *>(link, 0x1c)) at<u8 *>(at<u8 *>(link, 0x1c), 0x64) = piece;
        at<u8 *>(piece, 0x68) = at<u8 *>(link, 0x1c);
        at<u8 *>(link, 0x1c) = piece;
        at<u8 *>(link, 0x24) = piece;
    }
    if (!spare) {
        at<u32>(piece, 0) = at<u32>(link, 8);
        at<float>(piece, 0x6c) = (float)(double)at<u32>(link, 8);   // fild qword (count, 0)
    }
    if (link[0] & 2) TrackPieces_Renumber(piece);
    if (wide) at<u32>(piece, 8) |= half ? 0x60 : 0xe0;
    if (at<u8>(train, 4) & 1) {
        at<u32>(piece, 0x4c) = a;
        at<u32>(piece, 0x54) = b;
        at<float>(piece, 0x50) = height;
        at<u32>(piece, 0x78) = 1;
        at<u8 *>(piece, 0x7c) = piece + 0xc;
        u8 *const seats = static_cast<u8 *>(GlobalMem_Alloc(0x58, 1));
        at<u8 *>(piece, 0x80) = seats;
        at<u32>(piece, 0x84) = 1;
        at<u8 *>(piece, 0x88) = at<u8 *>(piece, 0x7c);
        at<u8 *>(piece, 0x8c) = seats;
        at<u8 *>(piece, 0x90) = piece + 0x78;
        at<u8 *>(at<u8 *>(piece, 0x7c), 0x10) = piece;
        at<u8 *>(at<u8 *>(at<u8 *>(piece, 0x90), 8), 0x10) = piece;
    } else {
        at<u32>(piece, 0xf8) = wide && half ? 0x3f000000u : 0;   // 0.5f
        *reinterpret_cast<u32 *>(0x00791cec) = at<u32>(train, 0x10);
        *reinterpret_cast<u32 *>(0x00791050) = at<u32>(desc, 0xb4);
        *reinterpret_cast<u32 *>(0x00791054) = at<u32>(desc, 0xb8);
        *reinterpret_cast<u32 *>(0x00791ce8) = at<u32>(train, 0xc);
        *reinterpret_cast<u32 *>(0x00791cf0) = 0;
        *reinterpret_cast<u32 *>(0x00791cf4) = 0x00791050;
        const u32 *const key = reinterpret_cast<const u32 *>(0x00791ce8);
        at<u32>(piece, 0x70) = (flags & 1) ? Ride_Claim(claim, at<u32>(desc, 0x14), 0x3fa, key, 0, 0, 0, 0)
                                           : g_env->createRide(at<u32>(desc, 0x14), 0x3fa, key, 0, 0, 0, 0, 0, -1, 0);
        u8 *const *const rides = reinterpret_cast<u8 *const *>(0x007a4610);
        at<u32>(rides[at<u32>(piece, 0x70)], 4) |= 0x80000;
        at<u8 *>(rides[at<u32>(piece, 0x70)], 0xd4) = piece;
        u8 *const ride = rides[at<u32>(piece, 0x70)];
        u8 *layer = at<u8 *>(ride, 0x10);
        const u32 *const banks = reinterpret_cast<const u32 *>(0x0074ca70);     // 3, 0xa, 2, 9, 0xa
        const u32 *const entries = reinterpret_cast<const u32 *>(0x0074ca88);   // 0, 0, 0, 0, 1
        for (u32 i = 0; i < 5; ++i, layer += 0x38) {
            if (!(at<u32>(desc, 0) & (0x20u << i))) continue;
            if (!(ride[(banks[i] + 2) * 0xc] & 3)) continue;
            Layer_Play(ride, banks[i], entries[i], layer, 0, 1.0f);
        }
        at<u32>(ride, 4) |= 0x1000;
        u8 *const holder = at<u8 *>(rides[at<u32>(piece, 0x70)], 0x10);
        X87Reg v = X87Reg(at<float>(piece, 0xe8)) * at<float>(desc, 0xd8) + at<float>(desc, 0xdc);
        if (!(v.v >= (double)at<float>(desc, 0xd0))) v = X87Reg(at<float>(desc, 0xd0));   // (below or unordered)
        if (v.v > (double)at<float>(desc, 0xd4)) v = X87Reg(at<float>(desc, 0xd4));
        at<float>(holder, 0x20) = (float)(X87Reg(at<float>(holder, 0x1c)) * v);
        Car_SetupCoupling(piece, reinterpret_cast<u32 *>(piece + 0x78), desc + 0x30, desc);
        if (at<u8>(desc, 0) & 8) Car_SetupCoupling(piece, reinterpret_cast<u32 *>(piece + 0x84), desc + 0x44, desc);
        if (!(flags & 6)) at<u32>(link, 0) |= 4;
    }
    if (flags & 8) at<u32>(piece, 8) |= 0x200;
    if (flags & 0x10) at<u32>(piece, 8) |= 0x400;
    if (flags & 0x20) at<u32>(piece, 8) |= 0x1800;
    if (flags & 0x40) at<u32>(piece, 8) |= 0x80000;
    if (flags & 0x80) at<u32>(piece, 8) |= 0x40000;
    if (flags & 0x100) at<u32>(piece, 8) |= 0x100000;
    if ((flags & 0x200) && !(flags & 1))
        Map_SetRideCell(at<u32>(*reinterpret_cast<u8 *const *>(0x00790fec), 0x28), at<s32>(train, 0xc), at<s32>(train, 0x10),
                        at<u32>(piece, 0x70), flags & 0x10);
    if (!spare) {
        Train_InsertCar(*reinterpret_cast<u8 *const *>(0x00790fec), piece, train, slot, height, nullptr, 0, flags & 1, 0, flags & 4);
        const u8 *const owner = at<const u8 *>(train, 0x18);
        if (!(flags & 4) && owner && !(at<u32>(owner, 0) & 0x4000)) {
            u32 bits;
            memcpy(&bits, &height, 4);
            at<u32>(*reinterpret_cast<u8 *const *>(0x00790fec), 0x118) = X87_FloatThrough(bits);   // fld / fstp
        }
        if (++at<u32>(link, 8) >= 2) at<float>(link, 0xc) = (float)(X87Reg(at<float>(link, 0xc)) - -1.0f);   // 0x6fde9c
    }
    if (!(flags & 1)) {
        at<u32>(piece, 8) |= 0x200000;
        if (at<u8 *>(piece, 0x68)) at<u32>(at<u8 *>(piece, 0x68), 8) |= 0x200000;
    }
    return piece;
}

void __cdecl CoasterLink_AddFixedPieces(u8 *link, u32, u32, float, u32 flag) {
    const CoasterLinksEnv *const env = g_env;
    const u8 *const type = at<const u8 *>(link, 4);
    const bool twin = at<u8>(type, 0) & 0x20;
    const s32 count = twin ? 3 : 2, first = twin ? 1 : 2;
    const auto holder = [link]() { return reinterpret_cast<const u8 *const *>(0x007a4610)[at<u32>(link, 0x10)]; };
    if (!at<u32>(link, 0x40)) {
        for (s32 i = 0; i < count; ++i) {
            const u32 part = env->findPart(holder(), 0x400000, (u32)(first + i));
            if (part == 0xffffffff) continue;
            const u8 *const h = holder();
            const u8 *const obj = at<const u8 *>(at<const u8 *>(at<const u8 *>(at<const u8 *>(h, 8), 0x28), 4), part * 0x14 + 4);
            float pos[3];
            if (obj) {
                at<u32>(pos, 0) = at<u32>(obj, 0x30);
                at<u32>(pos, 4) = at<u32>(obj, 0x34);
                at<u32>(pos, 8) = at<u32>(obj, 0x38);
            } else {
                at<u32>(pos, 0) = at<u32>(pos, 4) = at<u32>(pos, 8) = 0;
            }
            if ((at<u8>(type, 0) & 0x20) && (i == 0 || i == 2)) {
                const u8 *const frame = at<const u8 *>(at<const u8 *>(at<const u8 *>(h, 8), 4), 0x78);
                pos[1] = static_cast<float>(pos[1] - X87Reg(at<float>(frame, 0x44)));   // (fld the frame's, fsubr y)
                const s32 cz = (s32)(long long)(X87Reg(pos[2]) * 0.100000001f).v;   // 0x6fdedc, __ftol
                const s32 cx = (s32)(long long)(X87Reg(pos[0]) * 0.100000001f).v;
                const u8 *const desc = type + 0xa8;
                u8 *const entry = env->addEntry(cx, cz, 0x12, 0, (u32)(uintptr_t)desc, nullptr);
                u32 flags = i == 0 ? 0x274 : 0x28c;
                if (flag) flags |= 1;
                u8 *const car = env->addCar(entry, 0, 0, pos[1], flags, desc, 0, 0, 0);
                env->setChannelRates(at<u32>(*reinterpret_cast<u8 *const *>(0x00790fec), 4), car);
            } else {
                u8 *const entry = env->addEntry(0, 0, 0x13, 0, 0, pos);
                u32 flags = flag ? 1 : 0;
                if (at<u8>(type, 0) & 0x20) flags |= 0x3c;
                env->addCar(entry, at<u32>(entry, 0x1c), at<u32>(entry, 0x24), at<float>(entry, 0x20), flags, nullptr, 0, 0, 0);
            }
        }
        float pos[3] = {0, 0, 0};
        u8 *const entry = env->addEntry(0, 0, 7, 0, 0, pos);
        env->addCar(entry, at<u32>(entry, 0x1c), at<u32>(entry, 0x24), at<float>(entry, 0x20), 2, nullptr, 0, 0, 0);
    }
    env->setBounds();
    env->flashCar(link, 1);
    at<u32>(link, 0) |= 0x1001;   // (after the empty Debug_Printf)
}

u32 __cdecl CoasterLink_Create(u32 typeId, u32 ride, u32 height, u32 a3, u32 loaded, u32 a5) {
    const CoasterLinksEnv *const env = g_env;
    u8 *const link = static_cast<u8 *>(env->alloc(0x14c, 1));
    u32 id = 0;   // (loaded: the original's is the caller's ECX, sic)
    if (!loaded) {
        id = env->idAdd(reinterpret_cast<u32 *>(0x00790b04), reinterpret_cast<u32 *>(0x00790b00), reinterpret_cast<u32 *>(0x00791ce0), 0xff,
                        reinterpret_cast<u32 *>(0x00790bd0), (u32)(uintptr_t)link, -1);
        if (!id) {
            env->freeBlock(link);
            return 0;
        }
        at<u32>(link, 0x14) = id;
    }
    u8 *&head = *reinterpret_cast<u8 **>(0x00790fe0);
    at<u8 *>(link, 0x18) = head;
    head = link;
    const u8 *type = nullptr;   // (none: the reads below fault, sic)
    const u32 n = *reinterpret_cast<const u32 *>(0x00790fe8);
    for (u32 i = 0; i < n; ++i) {
        const u8 *const t = reinterpret_cast<const u8 *const *>(0x00790b08)[i];
        if (at<u32>(t, 4) == typeId) {
            type = t;
            break;
        }
    }
    at<u32>(link, 0xc) = typeId;
    at<const u8 *>(link, 4) = type;
    at<u32>(link, 0x10) = ride;
    at<u32>(link, 0x1c) = height;
    at<u32>(link, 0x20) = a3;
    at<u32>(link, 0x118) = at<u32>(type, 0xa4);
    at<u32>(link, 0x11c) = 0;
    at<u32>(link, 0xac) = 0x40400000;   // 3.0f
    env->ringInit(link + 0xb0, at<u32>(type, 0x2f8));
    env->ringInit(link + 0xd0, at<u32>(type, 0x318) * 2);
    if (at<u8>(type, 0) & 8) at<u32>(link, 0x3c) |= 8;
    if (at<u8>(type, 0) & 4) at<void *>(link, 0x13c) = env->alloc(at<u32>(type, 0x20) * 4, 1);
    at<u32>(link, 0x28) = a5;
    *reinterpret_cast<u8 **>(0x00790fec) = link;
    env->setState(link, 0x11);
    env->setMix(link, 0x3c, loaded);
    if (!loaded) {
        env->setUpCars(type, link, 0);
        env->setupCars(link, 0, at<u32>(type, 0x2f4), 1);
        const u8 *const holder = reinterpret_cast<const u8 *const *>(0x007a4610)[ride];
        const u8 *const frame = at<const u8 *>(at<const u8 *>(at<const u8 *>(holder, 8), 4), 0x78);
        const u32 bits = X87_FloatThrough(at<u32>(frame, 0x44));   // fld / fstp
        float through;
        std::memcpy(&through, &bits, 4);
        env->addFixed(link, height, a3, through, 0);
    }
    ++*reinterpret_cast<u32 *>(0x00792214);
    *reinterpret_cast<u8 **>(0x00790fec) = nullptr;
    return id;
}
}
