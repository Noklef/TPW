#include "world.h"

#include <cstdint>

#include "objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *const gObjectDefs = reinterpret_cast<u8 *>(0x007890a0);

// Ride number n (1 on) set in a bitset: bit (n - 1) & 7 of byte (n - 1) >> 3.
void set_ride(u32 bitset, u32 n) { global<u8>(bitset + ((n - 1) >> 3)) |= (u8)(1 << ((n - 1) & 7)); }

}  // namespace

extern "C" {

u32 __fastcall World_ArrivalVehicleNotSize1(const u8 *world) {
    const u16 current = at<u16>(world, 0x1da72a);
    return current != 0 && current != at<u16>(world, 0x1da72c) ? 1 : 0;
}

u32 __fastcall World_AddVisitorToDate(u8 *world) {
    u32 &count = at<u32>(world, 0x1da714);
    ++count;  // Debug_Printf(2, "Your park has received its %dth visitor!\n", count)
    return count;
}

void __fastcall World_MarkRideBuilt(u8 *, void *, u32 id) {
    const u32 n = at<u32>(ObjectDefs_Find(gObjectDefs, nullptr, (u16)id), 0x94);
    if (n != 0) set_ride(0x00802080, n);
}

u32 __fastcall World_HasRides(u8 *, void *, const u8 *wanted, u32 leave_out) {
    for (u32 i = 0; i < 0x20; ++i) global<u8>(0x007cfb70 + i) = 0;
    for (const u8 *def = ObjectDefs_First(gObjectDefs); def; def = ObjectDefs_Next(gObjectDefs)) {
        const u32 n = at<u32>(def, 0x94);
        if (n != 0 && (at<u32>(def, 0x98) == 0 || leave_out == 0)) set_ride(0x007cfb70, n);
    }
    for (u32 i = 0; i < 0x20; ++i) {
        for (s32 bit = 1; bit < 0x100; bit <<= 1) {
            if (((s8)wanted[i] & bit) == 1 && !((s8)global<u8>(0x007cfb70 + i) & bit)) return 0;  // sic
        }
    }
    return 1;
}

void *__fastcall Empty_Ctor(void *self) { return self; }

u8 *__fastcall Record999_Ctor(u8 *record) {
    at<u32>(record, 0) = 0;
    at<s32>(record, 4) = -999;
    at<u8>(record, 8) = 0;
    return record;
}
}
