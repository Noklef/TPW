#include "hoardinggrid.h"

#include <cstdint>

#include "../files/wrapsources.h"
#include "mapcell.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The world's 0x28-byte cell of an id.
const u8 *world_cell(u32 id) { return reinterpret_cast<const u8 *>(global<u32>(0x007cf83c) + (id + 0x6cde) * 0x28); }

}  // namespace

extern "C" {

void __cdecl HoardingGrid_Fill() {
    for (s32 y = 0; y < global<s32>(0x0081b1b0); y++) {
        for (s32 x = 0; x < global<s32>(0x0081d740); x++) {
            u8 *c = reinterpret_cast<u8 *>((uintptr_t)(0x0081eeb0 + (y >> 1) + (x >> 1) * 0x42 + (x >> 1)));
            *c = ' ';
            if (!Map_InBounds(x, y)) continue;
            const u8 *cell = world_cell((u16)((y << 7) + x + 1));
            if (MapCell_IsTypeCOr11(cell))
                cell = world_cell(*reinterpret_cast<const u16 *>(cell + 0x10));
            const u8 *cell44 = MapHandle_Cell44(reinterpret_cast<const u16 *>(cell));
            if (MapCell_IsType19(cell))
                *c = '@';
            else if (!MapCell_HasFlags(cell44, nullptr, 0x40) && *c != '@')  // (sic: never '@' here)
                *c = '#';
        }
    }
}

void __cdecl HoardingGrid_Flood(s32 row, s32 col, u32 pass, u32 fill) {
    for (; row >= 0 && row <= 0x41 && col >= 0 && col <= 0x41; col++) {
        u8 *const c = reinterpret_cast<u8 *>((uintptr_t)(0x0081d7c8 + row * 0x43 + col));
        if (*c != ' ' && *c != '@') return;
        *c = static_cast<u8>(fill);
        HoardingGrid_Flood(row - 1, col, pass, fill);
        HoardingGrid_Flood(row + 1, col, pass, fill);
        HoardingGrid_Flood(row, col - 1, pass, fill);
    }
}
}  // extern "C"

namespace {

u32 world() { return global<u32>(0x007cf83c); }
u8 *square(u32 id) { return reinterpret_cast<u8 *>(world() + ((id & 0xffff) + 0x6cde) * 0x28); }
u8 *square_at(s32 x, s32 y) { return square((u32)(y << 7) + x + 1); }
const u8 *cell44(u32 id) { return reinterpret_cast<const u8 *>(world() + id * 0x44 + 0x294); }
u8 &grid(u32 offset) { return global<u8>(0x0081d7c8 + offset); }

// One side's cell of CheckArea's walks: its run of type 7 / 0x1e (not 2) cells (two in a row set *bad), its type 0x19 count, its byte of
// the side's list (0 for a flag 0x40 cell that is not type 0x19, else 1; ones / zeros counted when given), and for a type 0x19 cell an
// end count when at is first or last, and the straight / other counts (straight when the straight neighbours are set and the neighbours
// are `straight`).
struct Side {
    s32 *run, *type19;
    u32 list, count;
    s32 *straight, *other;
};

void walk(u8 *sq, const Side &s, u8 *bad, s32 *ones, s32 *zeros, bool end, s32 *ends, u8 straight) {
    const u8 *cell = MapHandle_Cell44(reinterpret_cast<const u16 *>(sq));
    if (MapCell_IsType7Or1e(cell) && !MapCell_IsType2(cell)) {
        if (++*s.run > 1) {
            *bad = 1;
            *s.run = 0;
        }
    } else {
        *s.run = 0;
    }
    if (MapCell_IsType19(sq)) ++*s.type19;
    u32 &count = global<u32>(s.count);
    if (MapCell_HasFlags(cell, nullptr, 0x40) && !MapCell_IsType19(sq)) {
        global<u8>(s.list + count++) = 0;
        if (zeros) ++*zeros;
    } else {
        global<u8>(s.list + count++) = 1;
        if (ones) ++*ones;
    }
    if (MapCell_IsType19(sq)) {
        if (end) ++*ends;
        if (MapCell_StraightNeighbours(sq) && MapCell_Neighbours(sq) == straight)
            ++*s.straight;
        else
            ++*s.other;
    }
}

}  // namespace

extern "C" {

s32 __cdecl HoardingGrid_CheckArea(s32 x1, s32 y1, s32 x2, s32 y2, u8 charge) {
    const s32 minX = x1 < x2 ? x1 : x2, minY = y1 < y2 ? y1 : y2;
    const s32 maxX = x1 > x2 ? x1 : x2, maxY = y1 > y2 ? y1 : y2;
    static const u32 kLists[] = {0x0081ea78, 0x0081ec18, 0x0081eb98, 0x0081eb10};
    static const u32 kCounts[] = {0x0081b484, 0x0081aeec, 0x0081edec, 0x0081b73c, 0x0081af4c};
    for (const u32 list : kLists)
        for (u32 k = 0; k < 0x80; ++k) global<u8>(list + k) = 0xff;
    for (const u32 count : kCounts) global<u32>(count) = 0;
    u8 bad = 0, over = 0;
    if (!Map_InBounds(minX, minY) || !Map_InBounds(maxX, maxY) || maxX - minX <= 2 || maxY - minY <= 2) return 0;
    const u32 first = ((u32)(y1 << 7) + x1 + 1) & 0xffff;
    MapCell_HasFlags(cell44(first), nullptr, 0x40);   // (the answer unused)
    s32 runA = 0, runB = 0, ends = 0, ones = 0, zeros = 0;
    s32 straightTop = 0, straightBottom = 0, straightLeft = 0, straightRight = 0, otherTop = 0, otherBottom = 0, otherLeft = 0, otherRight = 0;
    s32 type19Top = 0, type19Bottom = 0, type19Left = 0, type19Right = 0;
    const s32 ax = global<s32>(0x007639f0), ay = global<s32>(0x007639f4), bx = global<s32>(0x007639f8), by = global<s32>(0x007639fc);
    for (s32 x = minX; x <= maxX; x += 2) {
        if (x == ax && (minY == ay || maxY == ay)) return 0;
        if (x == bx && (minY == by || maxY == by)) return 0;
        const bool end = x == minX || x == maxX;
        walk(square_at(x, minY), {&runA, &type19Top, 0x0081eb10, 0x0081b484, &straightTop, &otherTop}, &bad, &ones, &zeros, end, &ends, 0x11);
        walk(square_at(x, maxY), {&runB, &type19Bottom, 0x0081ec18, 0x0081edec, &straightBottom, &otherBottom}, &bad, &ones, &zeros, end, &ends, 0x11);
    }
    runA = runB = 0;
    for (s32 y = minY; y <= maxY; y += 2) {
        if ((minX == ax || maxX == ax) && y == ay) return 0;
        if ((minX == bx || maxX == bx) && y == by) return 0;
        const bool end = y == minY || y == maxY;
        walk(square_at(minX, y), {&runA, &type19Left, 0x0081eb98, 0x0081aeec, &straightLeft, &otherLeft}, &bad, nullptr, nullptr, end, &ends, 0x44);
        walk(square_at(maxX, y), {&runB, &type19Right, 0x0081ea78, 0x0081b73c, &straightRight, &otherRight}, &bad, nullptr, nullptr, end, &ends, 0x44);
    }
    // The corners: a side's other count past 1 counts one more straight cell at its start corner (x1, y1) or end corner (x2, y2), or,
    // when that corner is type 0x19, one fewer on the crossing side.
    const u8 *const start = square(first), *const finish = square((u32)(y2 << 7) + x2 + 1);
    s32 top = straightTop, bottom = straightBottom, left = straightLeft, right = straightRight;
    if (otherTop > 1) {
        ++top;
        if (*reinterpret_cast<const u32 *>(start + 8) == 0x19) {
            --top;
            --bottom;
        }
    }
    if (otherLeft > 1) {
        ++left;
        if (*reinterpret_cast<const u32 *>(start + 8) == 0x19) {
            --left;
            --right;
        }
    }
    if (otherBottom > 1) {
        ++bottom;
        if (*reinterpret_cast<const u32 *>(finish + 8) == 0x19) {
            --top;
            --bottom;
        }
    }
    if (otherRight > 1) {
        ++right;
        if (*reinterpret_cast<const u32 *>(finish + 8) == 0x19) {
            --left;
            --right;
        }
    }
    if (ends == 2 && !top && !bottom && !left && !right) return 0;
    if (bad) return 0;
    ones = zeros = 0;
    for (s32 y = minY; y <= maxY; ++y) {
        for (s32 x = minX; x <= maxX; ++x) {
            const u8 *sq = square_at(x, y);
            const u8 *const cell = MapHandle_Cell44(reinterpret_cast<const u16 *>(sq));
            if (MapCell_IsTypeCOr11(sq)) sq = square(*reinterpret_cast<const u16 *>(sq + 0x10));
            if (MapCell_HasFlags(cell, nullptr, 0x40) && !MapCell_IsType19(sq))
                ++zeros;
            else
                ++ones;
        }
    }
    if ((!ones || !zeros) && type19Top < 2 && type19Left < 2 && type19Bottom < 2 && type19Right < 2) return 0;
    for (u32 col = 0; col < 0x42; ++col) {   // (the grid at 0x81eeb0 copied in one cell down and right, a border of ' ' around)
        for (u32 row = 0; row < 0x42; ++row) grid(0x44 + col + 0x43 * row) = global<u8>(0x0081eeb0 + col + 0x43 * row);
        grid(col) = ' ';
        grid(0x43 * 0x41 + col) = ' ';
        grid(0x43 * col) = ' ';
        grid(0x43 * col + 0x41) = ' ';
    }
    for (s32 y = minY + 1; y <= maxY + 1; y += 2)
        for (s32 x = minX + 1; x <= maxX + 1; x += 2) grid((y >> 1) + 0x43 * (x >> 1)) = '#';
    if (grid(0x43 * 0x41) == ' ' || grid(0x43 * 0x41) == '@') {
        grid(0x43 * 0x41) = '#';
        HoardingGrid_Flood(0x40, 0, 0x20, 0x23);
        HoardingGrid_Flood(0x42, 0, 0x20, 0x23);
        HoardingGrid_Flood(0x41, -1, 0x20, 0x23);
        HoardingGrid_Flood(0x41, 1, 0x20, 0x23);
    }
    for (u32 col = 0; col < 0x40; ++col)
        for (u32 row = 0; row < 0x40; ++row)
            if (grid(0x44 + col + 0x43 * row) == ' ' || grid(0x44 + col + 0x43 * row) == '@') return 0;
    if (!charge) return 1;
    for (s32 y = minY + 2; y < maxY; ++y) {
        for (s32 x = minX + 2; x < maxX; ++x) {
            if (!MapCell_HasFlags(cell44(((u32)(y << 7) + 1 + x) & 0xffff), nullptr, 0x40)) continue;
            const u32 total = global<u32>(0x0081af4c) += global<u32>(0x0078560c);
            if (global<u32>(0x00785a2c)) {
                over = 0;
                continue;
            }
            const auto *source = reinterpret_cast<const TrackedSource *>((uintptr_t)GameTable7cfb90_Get1da726_5195d0(reinterpret_cast<u8 *>((uintptr_t)world())));
            if ((s32)(Source_Field0c(source) - total) < 0) over = 1;
        }
    }
    if (over) {
        global<u32>(0x00816d5c) = 1;
        return 0;
    }
    return 1;
}
}
