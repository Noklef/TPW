// A map cell's model picked from its neighbours: see cellmodel.h.
#include "cellmodel.h"

#include <cstdint>

#include "../core/accessors.h"
#include "mapcell.h"
#include "random.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The table's model for the bits: the first row with exactly them, else the first with bits all among them; -1 for none.
u32 search(u32 table, s32 rows, u8 bits, s32 turn, s32 *angle) {
    for (s32 i = 0; i < rows; ++i) {
        const u32 row = table + i * 12;
        if (global<u8>(row + 8) == bits) {
            *angle = turn + global<s32>(row + 4);
            return global<u32>(row);
        }
    }
    for (s32 i = 0; i < rows; ++i) {
        const u32 row = table + i * 12;
        const u8 need = global<u8>(row + 8);
        if ((bits & need) == need) {
            *angle = global<s32>(row + 4) + turn;
            return global<u32>(row);
        }
    }
    return 0xffffffffu;
}

// A straight cell facing 0x40 or 0x10 turns by 180.
s32 turn_of(const u8 *cell, u8 direction) {
    return MapCell_StraightNeighbours(cell) && (direction == 0x40 || direction == 0x10) ? 0xb4 : 0;
}

}  // namespace

extern "C" {

u32 __fastcall MapCell_PickModel(const u8 *cell, void *, s32 *angle) {
    u8 bits = MapCell_Neighbours(cell);
    const s32 type = at<s32>(cell, 8);
    const u8 direction = MapCell_Direction(cell);
    *angle = 0;
    const u32 size = static_cast<u32>(type < 0 ? -type : type);
    switch (size) {
    case 4: case 5: case 7: case 9: case 0xa: case 0xc: case 0x11: return 8;
    default: break;
    }
    u32 model;
    switch (size) {
    case 3:
    case 0xb: model = search(0x00762f30, global<s32>(0x00700c18), bits, turn_of(cell, direction), angle); break;
    case 0x10: model = search(0x00762ff0, global<s32>(0x00700c1c), bits, turn_of(cell, direction), angle); break;
    case 0x19: {
        const u32 index = static_cast<u32>(at<u16>(cell, 0)) - 1;
        const s32 x = static_cast<s32>(index & 0x7f), y = static_cast<s32>(index >> 7);
        const s32 turn = turn_of(cell, direction);
        if ((x == global<s32>(0x007639f0) && y == global<s32>(0x007639f4)) || (x == global<s32>(0x007639f8) && y == global<s32>(0x007639fc)))
            bits |= static_cast<u8>(Nibble_Scale(bits));
        model = search(0x00762ff0, global<s32>(0x00700c1c), bits, turn, angle);
        break;
    }
    default: model = search(0x00763138, global<s32>(0x00700c24), bits, 0, angle); break;
    }

    if (!MapCell_CornerNeighbours(cell) && !MapCell_JunctionNeighbours(cell)) {
        const u16 flags = at<u16>(cell, 0xe);
        if (flags & 1) model += 9;
        if (flags & 2) model += 0xc;
        if (flags & 8) model += 0xf;
        if (flags & 4) model = (model & 0xffff0000u) + 0x12;
    }
    if (type == 0x10 || type == 0xb) {
        if (MapCell_CornerNeighbours(cell)) {
            const u8 d = MapCell_Direction(cell), b = MapCell_Neighbours(cell);
            if ((d == 0x40 && b == 0x50) || (d == 0x10 && b == 0x14) || (d == 1 && b == 0x41) || (d == 4 && b == 5)) model += 0x10;
        }
    }
    if (type == 0x10 && MapCell_JunctionNeighbours(cell)) {
        const u8 d = MapCell_Direction(cell);
        if (d == 0x14) *angle = 0x5a;
        if (d == 0x41) model += 0x10;
        if (d == 0x50) {
            model += 0x10;
            *angle = 0x10e;
        }
    }
    if (type != 0x19) return model;

    if (MapCell_CornerNeighbours(cell)) {
        const u8 d = MapCell_Direction(cell), b = MapCell_Neighbours(cell);
        if ((d == 1 && b == 0x41) || (d == 0x10 && b == 0x14) || (d == 4 && b == 5) || (d == 0x40 && (b == 0x50 || b == 0x14)) || (d == 1 && b == 0x50) ||
            (d == 4 && b == 0x41) || (d == 0x10 && b == 5)) {
            model += 0x10;
            if (at<u8>(cell, 0xe) & 0xf) model = 0x1b;
        } else if (at<u8>(cell, 0xe) & 0xf) {
            model = 0x1a;
        }
    }
    if (MapCell_StraightNeighbours(cell) && !(at<u8>(cell, 0xe) & 0xf)) {
        const s32 counter = global<s32>(0x00820aec);
        const u32 variant = counter < 0x12 ? 0 : counter < 0x18 ? 0x14 : counter < 0x1e ? 0x17 : static_cast<u32>(reinterpret_cast<uintptr_t>(angle));
        if (global<u32>(0x00820af0) != variant) model += variant;
        global<u32>(0x00820af0) = variant;
        global<u32>(0x00820aec) = static_cast<u32>(World_Rand(global<u8 *>(0x007cf83c))) % 30;
    }
    return (model & 0xffff) | 0x50000;
}

}  // extern "C"
