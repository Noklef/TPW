#include "world.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "../core/funnytime.h"
#include "../core/gameheap.h"
#include "../core/initializers.h"
#include "objectdefs.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *const gObjectDefs = reinterpret_cast<u8 *>(0x007890a0);

// Ride number n (1 on) set in a bitset: bit (n - 1) & 7 of byte (n - 1) >> 3.
void set_ride(u32 bitset, u32 n) { global<u8>(bitset + ((n - 1) >> 3)) |= (u8)(1 << ((n - 1) & 7)); }

}  // namespace

extern "C" {

u8 *__fastcall World_Ctor(u8 *world) {
    using Ctor = void(__fastcall *)(void *);
    Class507800_Ctor(world);
    FunnyTime_Construct(world + 0x2a0);
    Init_4cf030(world + 0x2c4);
    u8 *const cells = world + 0x2d8;
    u8 *const tracks = cells + 0x110000;
    CRT_VectorCtorIteratorNoEh(cells, 0x44, 0x4000, reinterpret_cast<Ctor>(&MapCell_Ctor));
    CRT_VectorCtorIteratorNoEh(tracks, 0x28, 0x4000, reinterpret_cast<Ctor>(&TrackCell_Ctor));
    CRT_VectorCtorIteratorNoEh(cells + 0x1b1104, 0xa, 0x4000, reinterpret_cast<Ctor>(&RECell_Ctor));
    MemRanges_SetCells44((u32)(uintptr_t)cells, 0x44);
    MemRanges_SetCells28((u32)(uintptr_t)tracks, 0x28);
    CRT_VectorCtorIteratorNoEh(world + 0x1d943c, 0x20, 0x96, reinterpret_cast<Ctor>(&Empty_Ctor));
    at<u16>(world, 0x1da73c) = 0;
    at<u16>(world, 0x1da73e) = 0;
    at<u16>(world, 0x1da740) = 0;
    at<u16>(world, 0x1da744) = 0;
    at<u16>(world, 0x1da742) = 0;
    at<u16>(world, 0x1da746) = 0;
    at<u32>(world, 0x1da710) = 1;
    global<u32>(0x007cf56c) = 0;
    global<u32>(0x00801bbc) = 0x007cfb90;
    return world;
}

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

u8 __cdecl Game_Field1da710IsZero() { return Get1da710_51a280(global<u8 *>(0x0080239c)) == 0; }

u32 __fastcall GameTable7cfb90_Get1da718_519450(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da718));
}

u32 __fastcall GameTable7cfb90_Get1da71a_519490(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da71a));
}

u32 __fastcall GameTable7cfb90_Get1da71c_5194d0(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da71c));
}

u32 __fastcall GameTable7cfb90_Get1da720_519510(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da720));
}

u32 __fastcall GameTable7cfb90_Get1da722_519550(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da722));
}

u32 __fastcall GameTable7cfb90_Get1da724_519590(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da724));
}

u32 __fastcall GameTable7cfb90_Get1da726_5195d0(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da726));
}

u32 __fastcall GameTable7cfb90_Get1da736_519610(u8 *o) {
    return global<u32>(0x007cfb90 + 20 * at<u16>(o, 0x1da736));
}

u32 __fastcall World_BusStopBlocked(u8 *world) {
    // The debug print of the original is left out. The four cells are the bus stop position, the next one and the two on the row above (sic: the positions are cell numbers, 128 to a row).
    u16 cells[4];
    Map_BusStopPos(world + 0x2d8, nullptr, &cells[0], 0);
    u16 scratch[2] = {0, 0};
    cells[1] = *Map_BusStopPos(world + 0x2d8, nullptr, &scratch[0], 0) + 1;
    cells[2] = *Map_BusStopPos(world + 0x2d8, nullptr, &scratch[0], 0) - 0x100;
    cells[3] = *Map_BusStopPos(world + 0x2d8, nullptr, &scratch[0], 0) - 0xff;
    for (u32 i = 0; i < 4; i++) {
        const u16 handle = at<u16>(world, 0x2b8 + cells[i] * 0x44);
        if (handle == 0)
            continue;
        const u8 *const thing = reinterpret_cast<u8 *const *>(0x007cfb90)[handle * 5];
        if (at<u8>(thing, 2) == 1 && at<u32>(thing, 0x220) == 0x15)
            return 1;
    }
    return 0;
}
}
