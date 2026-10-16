#include "flyingrides.h"

#include <cstdint>

#include "../core/tiny_d.h"
#include "../model/parts.h"
#include "../model/partstate.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *const *table() { return reinterpret_cast<u8 *const *>(0x008791f8); }

// The record for index, null for one outside 1-99 or an empty entry.
u8 *record(s32 index) { return index > 0 && index < 100 ? table()[index] : nullptr; }

u8 *slot(u8 *rec, u32 i) { return rec + 0x5c + i * 0xe4; }

// The ride part step names released: key (base + step) % count + 1 among the flag-0x80 parts.
void release(u8 *s, s32 step) {
    const s32 count = at<s32>(s, 0xdc);
    if (count == 0) return;
    const s32 key = (at<s32>(s, 0xd8) + step) % count + 1;
    if (at<u32>(s, 0xc0) == 0) return;
    u8 *const *rides = reinterpret_cast<u8 *const *>(0x007a4610);
    const u32 part = Model_FindPartByKey4(rides[at<u32>(s, 0xc0)], 0x80, (u32)key);
    if (part == 0xffffffff) return;
    PartStates_Release(at<u8 *>(rides[at<u32>(s, 0xc0)], 8), part);
}

// |x| as cdq; xor; sub has it (INT_MIN stays).
s32 magnitude(s32 x) { return x < 0 ? (s32)(0u - (u32)x) : x; }

}  // namespace

extern "C" {

u32 __cdecl FlyingRide_FirstSlotRide(s32 index) {
    u8 *const rec = record(index);
    return rec ? at<u32>(slot(rec, 0), 0xc0) : 0;
}

// The four fly-by bounds (0x7660bc, 0x7660c4, 0x7660b8, 0x7660c0): the globals Fn_52c6c0 copies, scaled by 3000 and offset by 0x2904 (the last two negatively).
void __cdecl FlyingRide_SetBounds() {
    u32 &a = *reinterpret_cast<u32 *>(0x007660bc), &b = *reinterpret_cast<u32 *>(0x007660c4), &c = *reinterpret_cast<u32 *>(0x007660b8), &d = *reinterpret_cast<u32 *>(0x007660c0);
    Fn_52c6c0(0x007660bc, 0x007660c4, 0x007660b8, 0x007660c0);
    a = a * 3000 + 0x2904;
    b = b * 3000 + 0x2904;
    c = c * 3000 - 0x2904;
    d = d * 3000 - 0x2904;
}

// The part a slot of the record at index (not checked: the table entry is used as it is) steps to.
void __cdecl FlyingRide_ReleaseSlotPart(s32 index, s32 slotIndex, s32 step) {
    release(slot(table()[index], (u32)slotIndex), step);
}

u32 __cdecl FlyingRide_StepInSlot(s32 index) {
    u8 *rec = record(index);
    if (!rec) return 0;
    u32 i = 0;
    while (at<s32>(slot(rec, i), 0x80) != 1) {
        if (++i >= 20) return 0;
    }
    if (at<s32>(slot(rec, i), 0x8c) >= 0) return 0;
    release(slot(rec, i), (s32)(0u - at<u32>(slot(rec, i), 0x8c)));
    ++at<s32>(slot(table()[index], i), 0x8c);
    --at<s32>(table()[index], 0x48);
    rec = table()[index];
    return at<u32>(slot(rec, i), 0x90 + (0u - at<u32>(slot(rec, i), 0x8c)) * 4);
}

u32 __cdecl FlyingRide_StepOutSlot(s32 index) {
    u8 *rec = record(index);
    if (!rec) return 0;
    u32 i = 0;
    while (at<s32>(slot(rec, i), 0x8c) <= 0) {
        if (++i >= 20) return 0;
    }
    release(slot(rec, i), magnitude(at<s32>(slot(rec, i), 0x8c)));
    s32 &step = at<s32>(slot(table()[index], i), 0x8c);
    step = step > 0 ? step - 1 : step + 1;
    --at<s32>(table()[index], 0x48);
    rec = table()[index];
    return at<u32>(slot(rec, i), 0x90 + (u32)magnitude(at<s32>(slot(rec, i), 0x8c)) * 4);
}

void __cdecl FlyingRide_SetCap(s32 index, s32 value) {
    u8 *const rec = record(index);
    if (!rec) return;
    const s32 cap = value / 64;
    at<s32>(rec, 0x10) = cap;
    for (u32 i = 0; i < 20; ++i) {
        s32 &v = at<s32>(slot(table()[index], i), 0x84);
        if (v > cap) v = cap;
    }
}

void __cdecl FlyingRide_SetTenth(s32 index, s32 value) {
    u8 *const rec = record(index);
    if (rec) at<s32>(rec, 0x50) = value / 10;
}

void __cdecl FlyingRide_SetOn(s32 index, u32 on) {
    u8 *const rec = record(index);
    if (!rec) return;
    if (on) {
        const u32 value = at<u32>(rec, 4);
        at<u32>(rec, 0x38) = 1;
        at<u32>(rec, 0) = value;
        return;
    }
    if (at<u32>(rec, 0x34) == 3) return;
    at<u32>(rec, 0) = 1;
    for (u32 i = 0; i < 20; ++i) {
        u8 *const s = slot(rec, i);
        if (at<u32>(s, 0x80) != 0) at<u32>(s, 0x84) = 0;
        if (at<u32>(s, 0x80) == 1 && at<s32>(s, 0x8c) > 0) at<s32>(s, 0x8c) = (s32)(0u - at<u32>(s, 0x8c));
        at<u32>(rec, 0x38) = 0;
    }
}

void __cdecl FlyingRide_Set4(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (!rec) return;
    at<u32>(rec, 4) = value;
    u8 *const again = table()[index];
    if (at<u32>(again, 0x38) != 0) at<u32>(again, 0) = value;
}

u32 __cdecl FlyingRide_Get3c(s32 index) {
    u8 *const rec = record(index);
    return rec ? at<u32>(rec, 0x3c) : 0;
}

u32 __cdecl FlyingRide_FirstSlotReady(s32 index) {
    u8 *const rec = record(index);
    return rec && at<u32>(slot(rec, 0), 0xb8) != 0 && at<u32>(slot(rec, 0), 0x80) == 1 ? 1 : 0;
}

u32 __cdecl FlyingRide_Take3c(s32 index) {
    u8 *const rec = record(index);
    if (!rec) return 0;
    if (at<u32>(rec, 0x38) == 0 && at<u32>(rec, 0x48) == 0 && at<s32>(rec, 0xc) <= 1) return 0;
    if (at<u32>(rec, 0x3c) == 0) return 0;
    at<u32>(rec, 0x3c) = 0;
    return 1;
}

void __cdecl FlyingRide_Set44(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x44) = value;
}

void __cdecl FlyingRide_Set20(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x20) = value;
}

void __cdecl FlyingRide_Set24(s32 index, u32 value) {
    u8 *const rec = record(index);
    if (rec) at<u32>(rec, 0x24) = value;
}

u32 __cdecl FlyingRide_Busy(s32 index) {
    u8 *const rec = record(index);
    if (!rec) return 0;
    return at<u32>(rec, 0x48) == 0 && at<u32>(rec, 0x3c) != 0 && at<u32>(rec, 0xc) == 1 ? 0 : 1;
}
}
