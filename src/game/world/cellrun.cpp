// Runs of type 0x19 map cells: see cellrun.h.
#include "cellrun.h"

#include <cstdint>

#include "../core/accessors.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *cell_at(s32 x, s32 y) { return global<u8 *>(0x007cf83c) + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x28 + 0x6cde * 0x28; }

// The point list: (x, y) pairs at 0x81b740, the count at 0x820a8c.
void add_point(s32 x, s32 y) {
    s32 &count = global<s32>(0x00820a8c);
    if (count > 0x400) return;
    global<s32>(0x0081b740 + count * 8) = x;
    global<s32>(0x0081b744 + count * 8) = y;
    ++count;
}

// The run from the cell at 0x7639f0 / 0x7639f4 (left in x and y where it stops): visit(cell) on each cell, then turned() after a turn at it.
template <typename Visit, typename Turned> void run(s32 &x, s32 &y, Visit visit, Turned turned) {
    u8 direction = 4;
    u32 done = 0, count = 0;
    do {
        u8 *const cell = cell_at(x, y);
        visit(cell);
        if (at<u32>(cell, 8) == 0x19 && MapCell_CornerNeighbours(cell)) {
            direction = static_cast<u8>(Nibble_Scale(direction)) ^ MapCell_Neighbours(cell);
            turned();
        }
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

}  // namespace

extern "C" {

void __cdecl CellRun_Mark(u32 set) {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    global<u8>(0x00818c50) = 4;
    run(
        x, y,
        [set](u8 *cell) {
            if (static_cast<u8>(set))
                MapCell_SetFlags(cell, nullptr, MapCell_Flags(cell) | 0x20u);
            else
                MapCell_ClearFlags(cell, nullptr, 0x20);
        },
        [] {});
}

void __cdecl CellRun_Corners() {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    global<u8>(0x00818c50) = 4;
    global<s32>(0x0081b740) = x;
    global<s32>(0x0081b744) = y;
    global<s32>(0x00820a8c) = 1;
    run(x, y, [](u8 *) {}, [&x, &y] { add_point(x, y); });
    add_point(x, y);
    global<s32>(0x0081ede4) = x;
    global<s32>(0x0081ede8) = y;
}

}  // extern "C"
