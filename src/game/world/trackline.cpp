#include "trackline.h"

#include <cstdint>

#include "../core/accessors.h"
#include "cellkind.h"
#include "cellmodel.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *World() { return global<u8 *>(0x007cf83c); }
// The world's 0x28-byte cell at (x, y): (id + 0x6cde) * 0x28, the id cut to a word.
u8 *CellAt(s32 x, s32 y) { return World() + (((((u32)y << 7) + (u32)x + 1) & 0xffff) + 0x6cde) * 0x28; }

// The map cell of a 0x28-byte cell and the map cells south, south-east and east of it (Map_StepCell, not checked for none).
struct Block {
    u8 *cells[4];
};

Block BlockOf(const u8 *cell) {
    Block b;
    b.cells[0] = MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));
    static const u32 kTables[3] = {0x007cdba8, 0x007cdbb8, 0x007cdbc8};
    for (int i = 0; i < 3; ++i)
        b.cells[i + 1] = Map_StepCell(reinterpret_cast<const u16 *>(b.cells[0]), nullptr, global<s32>(kTables[i]), global<s32>(kTables[i] + 4), 1);
    return b;
}

void Unmark(const Block &b) {
    for (u8 *const c : b.cells) {
        MapCell_ClearFlags(c, nullptr, 0x40);
        MapCell_ApplyPathModel(c);
    }
}

// A cell beside the line: unless it has flag 0x20, its block loses flag 0x40 (with models) when any of the four has it.
void Beside(const u8 *cell) {
    if (MapCell_HasFlags(cell, nullptr, 0x20)) return;
    const Block b = BlockOf(cell);
    if (MapCell_HasFlags(b.cells[0], nullptr, 0x40) || MapCell_HasFlags(b.cells[1], nullptr, 0x40) ||
        MapCell_HasFlags(b.cells[2], nullptr, 0x40) || MapCell_HasFlags(b.cells[3], nullptr, 0x40))
        Unmark(b);
}

}  // namespace

extern "C" {

void __cdecl TrackLine_Prune() {
    s32 x = global<s32>(0x007639f0), y = global<s32>(0x007639f4);
    bool done = false;
    s32 steps = 0;
    u8 facing = 4;   // (a stack slot's low byte; its other bytes unset)
    global<u8>(0x00818c50) = 4;
    bool northEnded = false;   // (set once, never reset)
    do {
        u8 *const cell = CellAt(x, y);
        if (at<u32>(cell, 8) == 0x19) {
            const u8 *side;
            s32 cursor;
            switch (facing) {   // (the jump tables 0x5319b8 / 0x5319a4)
            case 1:
                cursor = x;
                do {
                    cursor += 2;
                    side = CellAt(cursor, y);
                    Beside(side);
                } while (!MapCell_HasFlags(side, nullptr, 0x20));
                break;
            case 0x10:
                cursor = x;
                do {
                    cursor -= 2;
                    side = CellAt(cursor, y);
                    Beside(side);
                } while (!MapCell_HasFlags(side, nullptr, 0x20));
                break;
            case 4:
                cursor = y;
                do {
                    cursor += 2;
                    side = CellAt(x, cursor);
                    Beside(side);
                } while (!MapCell_HasFlags(side, nullptr, 0x20));
                break;
            case 0x40:
                cursor = y - 2;
                for (;;) {
                    side = CellAt(x, cursor);
                    Beside(side);
                    cursor -= 2;
                    if (cursor < 0) northEnded = true;
                    if (MapCell_HasFlags(side, nullptr, 0x20) || northEnded) break;
                }
                break;
            default:
                break;
            }
            if (MapCell_CornerNeighbours(cell)) facing = (u8)(MapCell_Neighbours(cell) ^ (u8)Nibble_Scale(facing));
        }
        const u8 *const next = Map_StepCell28InDirection(reinterpret_cast<const u16 *>(cell), nullptr, facing, 2);
        if (next && at<u32>(next, 8) != 0x19) done = true;
        if (MapCell_NeighbourCount(cell) == 1 && steps) done = true;
        if (!done) {
            switch (facing) {   // (the jump tables 0x531a0c / 0x5319f8)
            case 1: y -= 2; break;
            case 4: x += 2; break;
            case 0x10: y += 2; break;
            case 0x40: x -= 2; break;
            default: break;
            }
        }
        ++steps;
    } while (!done);
    for (s32 row = 0; row < (s32)global<u32>(0x0081b1b0) - 1; ++row) {
        for (s32 column = 0; column < (s32)global<u32>(0x0081d740) - 1; ++column) {
            u8 *cell = CellAt(column, row);
            if (MapCell_IsTypeCOr11(cell)) cell = World() + ((u32)at<u16>(cell, 0x10) + 0x6cde) * 0x28;
            if (!MapCell_IsType19(cell) || MapCell_HasFlags(cell, nullptr, 0x20)) continue;
            const Block b = BlockOf(cell);
            if (at<u32>(b.cells[0], 8) != 0x15) TrackCell_Remove(cell, nullptr, 0, 0, 0);
            else TrackCell_ClearBlock(cell);
            MapCell_SetOverlap(cell, nullptr, 0);
            Unmark(b);
        }
    }
}

void __cdecl TrackCell_ClearAt(s32 x, s32 y) {
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x >= global<s32>(0x007a0314)) x = global<s32>(0x007a0314) - 1;
    if (y >= global<s32>(0x007a0374)) y = global<s32>(0x007a0374) - 1;
    if (!Map_InBounds(x, y)) return;
    u8 *const cell = reinterpret_cast<u8 *>((uintptr_t)World() + ((((u32)y << 7) + (u32)x + 1) & 0xffff) * 0x44 + 0x294);
    if (!cell) return;
    u8 *const entry = MapHandle_Entry28(reinterpret_cast<const u16 *>(cell));
    MapCell_Clear(cell, nullptr, 0, 0);
    TrackCell_Remove(entry, nullptr, 0, 0, 1);
}
}
