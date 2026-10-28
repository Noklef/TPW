// A ride's queue line traced from its entrance: see queuetrace.h.
#include "queuetrace.h"

#include <cstdint>

#include "../core/accessors.h"
#include "cellmodel.h"
#include "cellrun.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *map_cell(u32 id) { return global<u8 *>(0x007cf83c) + id * 0x44 + 0x294; }
u8 *map_cell(s32 x, s32 y) { return map_cell((((u32)y << 7) + (u32)x + 1) & 0xffff); }

// A step in the direction (1 north, 4 east, 0x10 south, 0x40 west; others stay); back: the other way.
void step(u8 direction, s32 &x, s32 &y, s32 sign) {
    switch (direction) {
    case 0x01: y -= sign; break;
    case 0x04: x += sign; break;
    case 0x10: y += sign; break;
    case 0x40: x -= sign; break;
    default: break;
    }
}

}  // namespace

extern "C" {

u8 __cdecl RideQueue_Trace(const u8 *thing) {
    const u32 handle = at<u16>(thing, 0x36);
    u8 stop = 0, result = 0, moved = 0;
    global<s32>(0x00820a8c) = 0;
    s32 x = (s32)((handle - 1) & 0x7f), y = (s32)((handle - 1) >> 7);
    u8 *cell = map_cell(x, y);
    u8 direction = MapCell_Neighbours(cell);
    u32 pair = 0;
    switch (direction) {
    case 0x01: pair = 0x007cdba0; break;
    case 0x04: pair = 0x007cdbc8; break;
    case 0x10: pair = 0x007cdba8; break;
    case 0x40: pair = 0x007cdbd0; break;
    default: break;
    }
    if (pair) {
        const u8 *const next = Map_StepCell(reinterpret_cast<const u16 *>(cell), nullptr, global<s32>(pair), global<s32>(pair + 4), 1);
        if (next) {
            s32 nx = x, ny = y;
            step(direction, nx, ny, 1);
            if (MapCell_IsType3Or9(next) && at<u16>(next, 0x10) != at<u16>(cell, 0x10)) {
                x = nx, y = ny;
                moved = 1;
            } else {
                PairTable_Add((u32)nx, (u32)ny);
            }
        }
    }
    if (!moved) {
        if (direction && Map_InBounds(x, y)) {
            for (;;) {
                cell = map_cell(x, y);
                if (MapCell_IsType3Or9(cell)) {
                    if (MapCell_CornerNeighbours(cell)) {
                        const u8 back = (u8)Nibble_Scale(direction);
                        direction = MapCell_Neighbours(cell) ^ back;
                        PairTable_Add((u32)x, (u32)y);
                        MapCell_SetOverlap(cell, nullptr, 1);
                    } else {
                        MapCell_SetOverlap(cell, nullptr, 0);
                    }
                } else {
                    stop = 1;
                    result = direction;
                }
                if (!MapCell_IsType9(cell)) {
                    if (MapCell_NeighbourCount(cell) == 1) stop = 1;
                    if (MapCell_IsType1(cell)) {
                        const s32 sx = x, sy = y;
                        step(direction, x, y, -1);
                        u8 *const behind = map_cell(x, y);
                        if (MapCell_IsType9(behind)) {
                            x = sx, y = sy;
                        } else {
                            MapCell_RemoveNeighbours(behind, nullptr, direction);
                            MapCell_ApplyPathModel(behind);
                        }
                        MapCell_RemoveNeighbours(cell, nullptr, Nibble_Scale(direction));
                        MapCell_ApplyPathModel(cell);
                        stop = 1;
                    }
                }
                if (stop) break;
                step(direction, x, y, 1);
            }
        } else {
            const u8 cell_direction = MapCell_Direction(map_cell(handle));
            u8 facing = (u8)(uintptr_t)thing;   // (sic: an angle other than the four)
            switch (at<s32>(thing, 0x10)) {
            case 0: facing = 0x01; break;
            case 90: facing = 0x04; break;
            case 180: facing = 0x10; break;
            case 270: facing = 0x40; break;
            default: break;
            }
            step((u8)Byte_ShiftLog2(cell_direction, facing), x, y, 1);
        }
    }
    PairTable_Add((u32)x, (u32)y);
    global<s32>(0x0081ede8) = y;
    global<s32>(0x0081ede4) = x;
    return result;
}

}  // extern "C"
