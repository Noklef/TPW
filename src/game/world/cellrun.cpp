// A run of type 0x19 map cells marked: see cellrun.h.
#include "cellrun.h"

#include <cstdint>

#include "../core/accessors.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *cell_at(s32 x, s32 y) { return global<u8 *>(0x007cf83c) + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x28 + 0x6cde * 0x28; }

}  // namespace

extern "C" {

void __cdecl CellRun_Mark(u32 set) {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    u8 direction = 4;
    global<u8>(0x00818c50) = 4;
    u32 done = 0, count = 0;
    do {
        u8 *const cell = cell_at(x, y);
        if (static_cast<u8>(set))
            MapCell_SetFlags(cell, nullptr, MapCell_Flags(cell) | 0x20u);
        else
            MapCell_ClearFlags(cell, nullptr, 0x20);
        if (at<u32>(cell, 8) == 0x19 && MapCell_CornerNeighbours(cell)) direction = static_cast<u8>(Nibble_Scale(direction)) ^ MapCell_Neighbours(cell);
        const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2);
        if (next && at<u32>(next, 8) != 0x19) done = 1;
        if (MapCell_NeighbourCount(cell) == 1 && count != 0) done = 1;
        if (!done) {
            switch (direction) {
            case 0x01: y -= 2; break;
            case 0x04: x += 2; break;
            case 0x10: y += 2; break;
            case 0x40: x -= 2; break;
            default: break;
            }
        }
        ++count;
    } while (!done);
}

}  // extern "C"
