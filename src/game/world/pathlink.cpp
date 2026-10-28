#include "pathlink.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/soundplay.h"
#include "cellmodel.h"
#include "mapcell.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *World() { return global<u8 *>(0x007cf83c); }
u32 Kind(const u8 *cell) { return at<u32>(cell, 8); }

// The cell a direction pair away (the pair of ints at table: 0x7cdba0...), scale 1.
u8 *Step(u8 *cell, u32 table) {
    return Map_StepCell(reinterpret_cast<const u16 *>(cell), nullptr, global<s32>(table), global<s32>(table + 4), 1);
}

u8 *StepDir(u8 *cell, u32 direction) { return Map_StepCellInDirection(reinterpret_cast<const u16 *>(cell), nullptr, direction); }

void Join(u8 *cell, u32 bit, u8 *next, u32 back) {
    MapCell_AddNeighbours(cell, nullptr, bit);
    MapCell_AddNeighbours(next, nullptr, back);
}

// __ftol (0x67a830): truncated, its low dword.
s32 ftol(double x) { return (s32)(long long)x; }

struct Side {
    u32 table, bit, back;   // the pair, the cell's bit and the next cell's
    u32 diagA, alongA, bitA, diagB, alongB, bitB;   // the next cell's diagonals: the pair, the direction along the side, its bit
};

// A side cell of mode 1.
void LinkSide(u8 *cell, const Side &s, bool models, s32 column) {
    u8 *const next = Step(cell, s.table);
    if (!next) return;
    if (Kind(next) == 1) {
        Join(cell, s.bit, next, s.back);
        u8 *diag = Step(next, s.diagA);
        if (diag && Kind(diag) == 1) {
            const u32 kind = Kind(StepDir(next, s.alongA));   // (not checked for none)
            if (kind != 3 && kind != 9) MapCell_AddNeighbours(next, nullptr, s.bitA);
        }
        diag = Step(next, s.diagB);
        if (diag && Kind(diag) == 1) {
            const u32 kind = Kind(StepDir(next, s.alongB));
            if (kind != 3 && kind != 9) MapCell_AddNeighbours(next, nullptr, s.bitB);
        }
        if (models) MapCell_ApplyPathModel(next);
    }
    if (Kind(next) == 0xa && (MapCell_Direction(next) & s.back)) {
        Join(cell, s.bit, next, s.back);
        if (models) MapCell_ApplyPathModel(next);
    }
    const u32 kind = Kind(next);
    if (kind == 9) {
        if (MapCell_Direction(next) & s.bit) {
            Join(cell, s.bit, next, s.back);
            if (models) MapCell_ApplyPathModel(next);
        }
    } else if (kind == 3) {
        if ((MapCell_Neighbours(next) & s.back) && models) {
            MapCell_ApplyPathModel(next);
            if (!global<u32>(0x00785a2c)) {
                const float scale = global<float>(0x00700d54);
                const s32 z = ftol((double)column * scale);
                const s32 x = ftol((double)column * scale);   // (the second local: the column too, sic)
                Sound_PlayAt(0, global<u32>(0x00803a2c), 0x8b, x, 0, z);
            }
        }
    }
}

// A diagonal cell of mode 1: joined when it and the two side cells between (not checked for none) are of kind 1.
void LinkDiagonal(u8 *cell, u32 table, u32 sideA, u32 sideB, u32 bit, u32 back, bool models) {
    u8 *const next = Step(cell, table);
    if (!next || Kind(next) != 1) return;
    if (Kind(Step(cell, sideA)) != 1 || Kind(Step(cell, sideB)) != 1) return;
    Join(cell, bit, next, back);
    if (models) MapCell_ApplyPathModel(next);
}

void LinkPath(u8 *cell, u32 models) {
    const s32 column = (s32)(((u32)at<u16>(cell, 0) - 1) & 0x7f);
    if (Kind(cell) == 3) {
        at<u32>(cell, 8) = 1;
        const u32 count = Counter7cdb98_Inc();
        Map_SetBlockValue(World() + 0x2d8, nullptr, count, at<u16>(cell, 0));   // (the high half the mode's: 0)
    }
    const bool on = (models & 0xff) != 0;
    static const Side kN = {0x007cdba0, 1, 0x10, 0x007cdbb8, 4, 8, 0x007cdbb0, 0x40, 0x20};
    static const Side kE = {0x007cdbc8, 4, 0x40, 0x007cdbc0, 1, 0x80, 0x007cdbb0, 0x10, 0x20};
    static const Side kS = {0x007cdba8, 0x10, 1, 0x007cdbd8, 4, 2, 0x007cdbc0, 0x40, 0x80};
    static const Side kW = {0x007cdbd0, 0x40, 4, 0x007cdbd8, 1, 2, 0x007cdbb8, 0x10, 8};
    LinkSide(cell, kN, on, column);
    LinkDiagonal(cell, 0x007cdbd8, 0x007cdba0, 0x007cdbc8, 2, 0x20, on);
    LinkSide(cell, kE, on, column);
    LinkDiagonal(cell, 0x007cdbb8, 0x007cdba8, 0x007cdbc8, 8, 0x80, on);
    LinkSide(cell, kS, on, column);
    LinkDiagonal(cell, 0x007cdbb0, 0x007cdba8, 0x007cdbd0, 0x20, 2, on);
    LinkSide(cell, kW, on, column);
    LinkDiagonal(cell, 0x007cdbc0, 0x007cdba0, 0x007cdbd0, 0x80, 8, on);
    // the sides with a cell of kind 3 or 9 that way drop the diagonals beside them (the direction in a stack slot's low byte; the callees
    // read only that)
    u8 d = 1;
    do {
        if (MapCell_Neighbours(cell) & d) {
            const u8 *const next = StepDir(cell, d);
            if (next && (Kind(next) == 3 || Kind(next) == 9)) {
                switch (d) {
                case 1: MapCell_RemoveNeighbours(cell, nullptr, 2); MapCell_RemoveNeighbours(cell, nullptr, 0x80); break;
                case 4: MapCell_RemoveNeighbours(cell, nullptr, 2); MapCell_RemoveNeighbours(cell, nullptr, 8); break;
                case 0x10: MapCell_RemoveNeighbours(cell, nullptr, 8); MapCell_RemoveNeighbours(cell, nullptr, 0x20); break;
                case 0x40: MapCell_RemoveNeighbours(cell, nullptr, 0x80); MapCell_RemoveNeighbours(cell, nullptr, 0x20); break;
                default: break;
                }
            }
        }
        d = (u8)Byte_ShiftLog2(d, 4);
    } while (d != 1);
}

// A drag's side: a cell of the kind mode there with under two neighbours and the owner 0x818698's or none is joined; a cell of kind 1
// then drops the two diagonals beside it.
void DragSide(u8 *cell, u32 mode, u32 table, u32 bit, u32 back, u32 dropA, u32 dropB) {
    u8 *const next = Step(cell, table);
    if (!next || Kind(next) != mode) return;
    if ((s32)MapCell_NeighbourCount(next) < 2) {
        const u16 owner = at<u16>(next, 0x10);
        if (owner == global<u16>(0x00818698) || owner == 0) Join(cell, bit, next, back);
    }
    if (Kind(cell) == 1) {
        MapCell_RemoveNeighbours(cell, nullptr, dropA);
        MapCell_RemoveNeighbours(cell, nullptr, dropB);
    }
}

void Drag(u8 *cell, u32 mode, s32 dx, s32 dy) {
    const u8 *const target = World() + 0x294 + (u32)at<u16>(global<u8 *>(0x008186ac), 0x36) * 0x44;
    if (global<u32>(0x00820aa4)) {
        static const u32 kTables[4] = {0x007cdba0, 0x007cdbc8, 0x007cdba8, 0x007cdbd0};
        static const u8 kFacing[4] = {0x10, 0x40, 1, 4};
        static const u32 kBits[4] = {1, 4, 0x10, 0x40};
        for (int i = 0; i < 4; ++i) {
            u8 *const next = Step(cell, kTables[i]);
            if (next && MapCell_Direction(next) == kFacing[i] && next == target) Join(cell, kBits[i], next, kFacing[i]);
        }
        const s32 x = global<s32>(0x00763ac0), y = global<s32>(0x00763ac4);
        if (x == 0x80 || y == 0x80) {
            dx = 0;
            dy = 0;
        } else {
            dx = x;
            dy = y;
        }
    }
    if (dy == 1) DragSide(cell, mode, 0x007cdba0, 1, 0x10, 2, 0x80);
    if (dx == -1) DragSide(cell, mode, 0x007cdbc8, 4, 0x40, 2, 8);
    if (dy == -1) DragSide(cell, mode, 0x007cdba8, 0x10, 1, 8, 0x20);
    if (dx == 1) DragSide(cell, mode, 0x007cdbd0, 0x40, 4, 0x80, 0x20);
}

}  // namespace

extern "C" {

void __fastcall MapCell_LinkPath(u8 *cell, void *, u32 mode, s32 dx, s32 dy, u32 models) {
    if (mode == 1) LinkPath(cell, models);
    else if (mode == 0 || mode == 3) Drag(cell, mode, dx, dy);
    global<s32>(0x00763ac0) = dx;
    global<s32>(0x00763ac4) = dy;
}
}
