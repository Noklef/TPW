// Runs of type 0x19 map cells: see cellrun.h.
#include "cellrun.h"

#include <cstdint>

#include "../core/accessors.h"
#include "dirstep.h"
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

// Two cells on in the direction (1 north, 4 east, 0x10 south, 0x40 west; others stay).
void move(u8 direction, s32 &x, s32 &y) {
    switch (direction) {
    case 0x01: y -= 2; break;
    case 0x04: x += 2; break;
    case 0x10: y += 2; break;
    case 0x40: x -= 2; break;
    default: break;
    }
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
        if (!done) move(direction, x, y);
        ++count;
    } while (!done);
}

// The track run from (x, y) shared by TraceA and TraceB (B: no move on reaching an end, and the cell `saved` reset).
void track_run(s32 x, s32 y, u8 direction, bool b, u16 saved, u32 done) {
    do {
        u8 *const cell = cell_at(x, y);
        if (MapCell_IsTypeBOr10(cell)) {
            if (MapCell_CornerNeighbours(cell)) {
                direction = static_cast<u8>(Nibble_Scale(direction)) ^ MapCell_Neighbours(cell);
                add_point(x, y);
                MapCell_SetOverlap(cell, nullptr, 1);
            } else if (MapCell_JunctionNeighbours(cell)) {
                MapCell_SetOverlap(cell, nullptr, 1);
            } else {
                MapCell_StraightNeighbours(cell);  // (its answer unused, sic)
                MapCell_SetOverlap(cell, nullptr, 0);
            }
            if (b && at<u16>(cell, 0) == saved) MapCell_SetOverlap(cell, nullptr, 0);
            if (!MapCell_CornerNeighbours(cell) && !(MapCell_Neighbours(cell) & direction)) done = 1;
            const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction, 2);
            if (next && MapCell_IsTypeDOr12ViaParent(next) && at<u16>(next, 0x10) == at<u16>(cell, 0x10)) {
                const u8 bits = MapCell_Neighbours(next);
                if (static_cast<u8>(Nibble_Scale(direction)) & bits) {
                    done = 1;
                    if (!b) move(direction, x, y);
                }
            }
        }
        if (!done) move(direction, x, y);
    } while (!done);
    add_point(x, y);
    global<s32>(0x0081ede4) = x;
    global<s32>(0x0081ede8) = y;
}

}  // namespace

extern "C" {

void __cdecl TrackRun_TraceA(const u8 *thing) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    const u8 direction = static_cast<u8>(Position_StepA(&x, &y, at<s32>(thing, 0x10)));
    global<u8>(0x00818c50) = direction;
    global<s32>(0x00820a8c) = 0;
    if (MapCell_IsTypeBOr10(cell_at(x, y))) add_point(x, y);
    track_run(x, y, direction, false, 0, 0);
}

void __cdecl TrackRun_TraceB(const u8 *thing) {
    const u32 t = (static_cast<u32>(thing[7]) << 7) + thing[5];
    s32 x = static_cast<s32>(t & 0x7f), y = static_cast<s32>(t >> 7);
    u16 saved = 0;
    const u8 direction = static_cast<u8>(Position_StepB(&x, &y, at<s32>(thing, 0x10)));
    global<u8>(0x00818c50) = direction;
    if (Map_InBounds(global<s32>(0x007639e0), global<s32>(0x007639e4)))
        saved = static_cast<u16>((static_cast<u32>(global<s32>(0x007639e4)) << 7) + static_cast<u32>(global<s32>(0x007639e0)) + 1);
    global<s32>(0x00820a8c) = 0;
    u8 *const cell = cell_at(x, y);
    if (MapCell_IsTypeBOr10(cell)) add_point(x, y);
    track_run(x, y, direction, true, saved, MapCell_IsTypeBDOr10Or12Or19(cell) ? 0 : 1);
}

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
