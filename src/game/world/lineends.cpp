#include "lineends.h"

#include <cstdint>

#include "../core/accessors.h"
#include "cellmodel.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *World() { return global<u8 *>(0x007cf83c); }
// The offset of the world's 0x28-byte cell at (x, y): (id + 0x6cde) * 0x28, the id cut to a word.
u32 CellOffset(s32 x, s32 y) { return ((u32)(u16)(((u32)y << 7) + (u32)x + 1) + 0x6cde) * 0x28; }
u32 WithLowByte(u32 slot, u8 low) { return (slot & 0xffffff00) | low; }

// An end two steps out: with end flags, flags gives 1 and else 4 gives other; either then gets its model.
void ResetEnd(u8 *end, u32 flags, u32 other) {
    if (!end || !MapCell_HasLowFlags(end)) return;
    if (MapCell_HasFlags(end, nullptr, flags)) {
        MapCell_ClearFlags(end, nullptr, 0xf);
        MapCell_SetFlags(end, nullptr, 1);
    } else if (MapCell_HasFlags(end, nullptr, 4)) {
        MapCell_ClearFlags(end, nullptr, 0xf);
        MapCell_SetFlags(end, nullptr, other);
    } else {
        return;
    }
    MapCell_ApplyModel(end);
}

// The direction d (slot's low byte) cut from the cell and its reverse from the cell two steps that way.
void Cut(u8 *cell, u32 slot) {
    MapCell_RemoveNeighbours(cell, nullptr, slot);
    MapCell_ApplyModel(cell);
    if (MapCell_Overlap(cell)) MapCell_DecOverlap(cell);
    u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, slot, 2);
    if (!next) return;
    MapCell_RemoveNeighbours(next, nullptr, Nibble_Scale(slot));
    MapCell_ApplyModel(next);
}

}  // namespace

extern "C" {

void __cdecl TrackCell_ResetEnds(u8 *cell, s32 ax, s32 ay, s32 bx, s32 by) {
    u8 *const first = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, ax, ay, 2);
    u8 *const second = Map_StepCell28(reinterpret_cast<const u16 *>(cell), nullptr, bx, by, 2);
    Map_InBounds(global<s32>(0x007639e0), global<s32>(0x007639e4));
    Map_InBounds(global<s32>(0x007639e8), global<s32>(0x007639ec));
    // (the cell argument's slot when the direction is none of the four, sic: by then it holds the address of the second pair's stack
    // copy, here a local's)
    s32 copy[2] = {bx, by};
    u32 f = (u32)(uintptr_t)copy, g = f;
    switch (MapCell_Direction(cell)) {
    case 1:
    case 4:
        f = 2;
        g = 8;
        break;
    case 0x10:
    case 0x40:
        f = 8;
        g = 2;
        break;
    }
    MapCell_ClearFlags(cell, nullptr, 0xf);
    ResetEnd(first, f, g);
    ResetEnd(second, g, f);
    MapCell_ApplyModel(cell);
}

u8 *__cdecl LineEnd_Unlink(s32 x, s32 y, u32 dir) {
    u32 offset = CellOffset(x, y);
    u8 *cell = World() + offset;
    u8 d = MapCell_Direction(cell);
    u32 slot = WithLowByte((u32)x, d);   // (x's stack slot)
    if (x == global<s32>(0x007639e8) && y == global<s32>(0x007639ec)) {
        d = (u8)(MapCell_Neighbours(cell) & ~d);
        slot = WithLowByte(slot, d);
        if (d) Cut(cell, slot);
        return World() + offset;
    }
    if ((u8)dir) slot = WithLowByte(slot, d = (u8)dir);
    if (d) Cut(cell, slot);
    const s32 sx = global<s32>(0x007639e0), sy = global<s32>(0x007639e4);
    const bool beside = ((u32)x + 2 == (u32)sx && y == sy) || (x == sx && (u32)y + 2 == (u32)sy) || ((u32)x - 2 == (u32)sx && y == sy) ||
                        (x == sx && (u32)y - 2 == (u32)sy);
    if (!beside || !Map_InBounds(sx, sy)) return World() + offset;
    offset = CellOffset(global<s32>(0x007639e0), global<s32>(0x007639e4));
    cell = World() + offset;
    static const u32 kKinds[] = {0xd, 0x12};
    static const u32 kSides[] = {1, 4, 0x10, 0x40};
    for (const u32 kind : kKinds) {
        for (const u32 side : kSides) {
            const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, side, 2);
            if (at<u32>(next, 8) != kind) continue;   // (none not checked: the original reads +8 of a null too)
            MapCell_AddNeighbours(cell, nullptr, side);
            MapCell_ApplyModel(cell);
        }
    }
    return World() + offset;
}
}
