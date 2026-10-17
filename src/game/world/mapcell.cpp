#include "mapcell.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../files/file.h"
#include "object.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u16 *store_pos(u16 *out, u32 x, u32 y) {
    *out = (u16)((y << 7) + x + 1);
    return out;
}

// A 0x28-byte cell's neighbour bits: mHoardingNeighbours for a track cell while 0x81b4cc is set.
u8 &neighbours(const u8 *cell) {
    return global<u8>(0x0081b4cc) != 0 && cell[2] == 2 ? at<u8>(cell, 0x22) : at<u8>(cell, 0xc);
}

// The world's 0x28-byte cell for a handle (wrapping as the originals' 32-bit sums do).
const u8 *world_cell(u32 handle) { return reinterpret_cast<const u8 *>(global<u32>(0x007cf83c) + 0x1102b0 + handle * 0x28); }

u32 type_in(const u8 *cell, u32 a, u32 b) {
    const u32 type = at<u32>(cell, 8);
    return type == a || type == b ? 1 : 0;
}

// How many of the bits in mask are set in byte, counted a bit at a time as the originals do.
u32 count(u8 byte, u32 mask) {
    u32 n = 0;
    for (u32 bit = 1; bit < 0x100; bit <<= 1) {
        if ((mask & bit) && (byte & bit)) ++n;
    }
    return n;
}

// A gBalanceLevel pair of positions at a: A x, A y, B x, B y.
u16 *balance_pos(u16 *out, u32 b, u32 a) {
    return b == 0 ? store_pos(out, global<u32>(a), global<u32>(a + 4)) : store_pos(out, global<u32>(a + 8), global<u32>(a + 0xc));
}

// As balance_pos, but B with A's y (sic).
u16 *balance_pos_a_y(u16 *out, u32 b, u32 a) {
    return store_pos(out, global<u32>(b == 0 ? a : a + 8), global<u32>(a + 4));
}

// The type of the cell at (x, y) of a map grid {width, height, cells of 8 bytes in rows of 20 from +8}.
s32 GridType(const u8 *grid, s32 x, s32 y) { return at<s32>(grid, 8 + 8 * (x + y * 20)); }

bool IsEdgeType(s32 type) { return type == 4 || type == 9 || type == 0x17 || type == 10; }

}  // namespace

namespace {

u8 *CellAt(s32 x, s32 y) {
    const u32 id = (((u32)y << 7) + (u32)x + 1) & 0xffff;
    return *reinterpret_cast<u8 **>(0x007cf83c) + 0x294 + id * 0x44;
}

}  // namespace

extern "C" {

u8 *__fastcall MapHandle_Entry28(const u16 *handle) {
    return global<u8 *>(0x008023a0) + 0x10ffd8 + (u32)*handle * 0x28;
}

u8 *__fastcall MapHandle_Cell44(const u16 *handle) { return global<u8 *>(0x008023a0) + (u32)*handle * 0x44 - 0x44; }

void __fastcall MemRanges_Set(u8 *ranges, void *, u32 index, u32 a, u32 b, const char *) {
    at<u32>(ranges, index * 8) = a;
    at<u32>(ranges, index * 8 + 4) = b;
}

void __cdecl MemRanges_SetCells44(u32 base, u32 stride) {
    global<u32>(0x007cdb40) = base;
    global<u32>(0x007cdb50) = stride;
    global<u32>(0x007cdb44) = (stride << 14) + base;
}

void __cdecl MemRanges_SetCells28(u32 base, u32 stride) {
    global<u32>(0x007cdb48) = base;
    global<u32>(0x007cdb54) = stride;
    global<u32>(0x007cdb4c) = (stride << 14) + base;
}

u32 __cdecl Map_InBounds(s32 x, s32 y) { return x >= 0 && y >= 0 && x < 0x80 && y < 0x80; }

u8 *__fastcall Map_RECell(u8 *map, void *, u32 pos) { return map + 0x1b1104 + ((pos & 0xffff) - 1) * 10; }

u16 *__fastcall Map_TicketBoothPos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x0078559c); }
u16 *__fastcall Map_BusStopPos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x007855ac); }
u16 *__fastcall Map_EntrancePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos(out, b, 0x007855bc); }
u16 *__fastcall Map_CrossingParkSidePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos_a_y(out, b, 0x007855cc); }
u16 *__fastcall Map_CrossingBSSidePos(const u8 *, void *, u16 *out, u32 b) { return balance_pos_a_y(out, b, 0x007855dc); }

// pos - 1 (y * 128 + x) to its block: (y / 16) * 33 + x / 16 (the column by an arithmetic shift).
static u32 block_of(u32 pos) {
    const u32 i = (pos & 0xffff) - 1;
    return (i >> 11) * 33 + ((u32)((s32)i >> 4) & 7);
}

void __fastcall Map_SetBlockValue(u8 *map, void *, u32 value, u32 pos) {
    if ((u16)pos == 0) return;
    at<u32>(map, 0x1b0000 + block_of(pos) * 4) = value;
}

u32 __fastcall Map_BlockValue(const u8 *map, void *, u32 pos) { return at<u32>(map, 0x1b0000 + block_of(pos) * 4); }

u8 *__fastcall MapCell_Ctor(u8 *cell) {
    for (u32 i = 0; i < 0x44; i += 4) at<u32>(cell, i) = 0;
    at<u8>(cell, 2) = 1;
    at<u16>(cell, 0) = 0x423f;
    return cell;
}

u8 *__fastcall RECell_Ctor(u8 *cell) {
    at<u32>(cell, 0) = 0;
    at<u32>(cell, 4) = 0;
    at<u16>(cell, 8) = 0;
    return cell;
}

s32 __cdecl Map_IdAt(s32 x, s32 y) {
    if (!Map_InBounds(x, y)) return -1;
    return (s32)(((u32)y << 7) + (u32)x + 1 & 0xffff);
}

u8 *__fastcall TrackCell_Ctor(u8 *cell) {
    for (u32 i = 0; i < 0x28; i += 4) at<u32>(cell, i) = 0;
    at<u8>(cell, 2) = 2;
    return cell;
}

u32 __fastcall RECell_Equals(const u8 *cell, void *, const u8 *other) {
    for (u32 i = 0; i != 10; ++i) {
        if (cell[i] != other[i]) return 0;
    }
    return 1;
}

void __fastcall MapCell_AddNeighbours(u8 *cell, void *, u32 bits) { neighbours(cell) |= (u8)bits; }

void __fastcall MapCell_RemoveNeighbours(u8 *cell, void *, u32 bits) { neighbours(cell) &= (u8)~bits; }

u8 __fastcall MapCell_Neighbours(const u8 *cell) { return neighbours(cell); }

u32 __fastcall MapCell_NeighbourCount(const u8 *cell) { return count(neighbours(cell), 0xff); }

u32 __fastcall MapCell_SideNeighbourCount(const u8 *cell) { return count(neighbours(cell), 0x55); }

void __fastcall MapCell_SetDirection(u8 *cell, void *, u32 value) { cell[0xd] = (u8)value; }

void __fastcall MapCell_ClearDirection(u8 *cell, void *, u32 bits) { cell[0xd] &= (u8)~bits; }

u8 __fastcall MapCell_Direction(const u8 *cell) { return cell[0xd]; }

u32 __fastcall MapCell_SideDirectionCount(const u8 *cell) { return count(cell[0xd], 0x55); }

void __fastcall MapCell_DecOverlap(u8 *cell) { --at<u16>(cell, 0x20); }

void __fastcall MapCell_SetOverlap(u8 *cell, void *, u32 value) { at<u16>(cell, 0x20) = (u16)value; }

s32 __fastcall MapCell_Overlap(const u8 *cell) { return at<s16>(cell, 0x20); }

u32 __fastcall MapCell_HasFlags(const u8 *cell, void *, u32 flags) { return (at<u16>(cell, 0xe) & (u16)flags) != 0 ? 1 : 0; }

u32 __fastcall MapCell_Flag400(const u8 *cell) { return at<u16>(cell, 0xe) >> 10 & 1; }

u32 __fastcall Map_BlockValueAt(const u8 *map, void *, u32 bx, u32 by) {
    return at<u32>(map, 0x1b0000 + (by * 33 + bx) * 4);
}

s32 __cdecl Map_PercentWithRE() {
    u8 *map = at<u8 *>(reinterpret_cast<const void *>(0x0080239c), 0) + 0x2d8;
    s32 total = 0, counted = 0;
    for (s32 x = 0x7f; x >= 0; x--) {
        for (s32 y = 0x7f; y >= 0; y--) {
            const u32 id = ((u32)y << 7) + (u32)x + 1;
            const u8 *cell = map + (id & 0xffff) * 17 * 4 - 0x44;
            if (MapCell_IsType2(cell) || MapCell_IsType7Or1e(cell))
                continue;
            total++;
            if (at<s16>(Map_RECell(map, nullptr, id & 0xffff), 6) > 0)
                counted++;
        }
    }
    return total ? counted * 100 / total : 0;
}

void __fastcall Map_SpreadValues(u8 *world, void *, u32 add, u32 index, u32 cell) {
    const u8 *source = world + index * 12;
    const s32 radius = (u16)source[0x1d910e];
    const u32 at_cell = (cell & 0xffff) - 1;
    const s32 cx = (s32)(at_cell & 0x7f), cy = (s32)(at_cell >> 7);
    const s32 xmin = cx - radius < 0 ? 0 : cx - radius, ymin = cy - radius < 0 ? 0 : cy - radius;
    const s32 xmax = cx + radius > 0x7f ? 0x7f : cx + radius, ymax = cy + radius > 0x7f ? 0x7f : cy + radius;
    const s32 sign = add != 0 ? 1 : -1;
    for (s32 x = xmin; x <= xmax; ++x) {
        for (s32 y = ymin; y <= ymax; ++y) {
            const u32 id = (u16)((y << 7) + 1 + x);
            s16 *to = reinterpret_cast<s16 *>(world + 0x1b1104 + (id - 1) * 10);
            const s16 *from = reinterpret_cast<const s16 *>(source + 0x1d9104);
            const s32 dy = cy - y < 0 ? y - cy : cy - y, dx = cx - x < 0 ? x - cx : cx - x;
            for (s32 i = 0; i < 5; ++i)
                to[i] = (s16)(to[i] + (s16)((sign * (s32)from[i]) / (dx + dy + 1)));
        }
    }
}

void __fastcall World_InitBlock(u8 *world) {
    for (u32 offset = 0; offset < 0x1b1104; offset += 4)  // (0x110000, 0xa0000 and 0x1104 bytes in three runs)
        *reinterpret_cast<u32 *>(world + offset) = 0;
    for (s32 x = 0; x < 0x80; ++x) {
        for (s32 y = 0; y < 0x80; ++y) {
            const u16 id = (u16)((y << 7) + 1 + x);
            *reinterpret_cast<u16 *>(world + (u32)id * 0x44 - 0x44) = id;
            *reinterpret_cast<u16 *>(world + (u32)id * 0x28 + 0x10ffd8) = id;
            world[(u32)id * 0x44 - 0x42] = 1;
            world[(u32)id * 0x28 + 0x10ffda] = 2;
        }
    }
    // (the eight region effects of the level balance: a radius dword and five values, 0x18 bytes each, from 0x785610)
    for (u32 k = 0; k < 8; ++k)
        world[0x1d910e + 12 * k] = *reinterpret_cast<const u8 *>(0x00785610 + 0x18 * k);
    for (u32 k = 0; k < 8; ++k) {
        u16 *to = reinterpret_cast<u16 *>(world + 0x1d9104 + 12 * k);
        for (u32 j = 0; j < 5; ++j)
            to[j] = *reinterpret_cast<const u16 *>(0x00785614 + 0x18 * k + 4 * j);
    }
}

static u32 DirectionTable(u32 direction);

u8 *__fastcall Map_StepCell28InDirection(const u16 *cell, void *, u32 direction, s32 scale) {
    const u32 table = DirectionTable(direction);
    if (!table) return nullptr;
    return Map_StepCell28(cell, nullptr, at<s32>(reinterpret_cast<const void *>(table), 0), at<s32>(reinterpret_cast<const void *>(table), 4), scale);
}

u8 *__fastcall Map_StepCell28(const u16 *cell, void *, s32 dx, s32 dy, s32 scale) {
    const u32 index = (u32)*cell - 1;
    const s32 x = (s32)(index & 0x7f) + dx * scale;
    const s32 y = (s32)(index >> 7) + dy * scale;
    if (!Map_InBounds(x, y)) return nullptr;
    const u32 id = ((u32)(dy * scale) << 7) + *cell + (u32)(dx * scale);  // (the scaled offset, unlike Map_StepCell)
    return at<u8 *>(reinterpret_cast<const void *>(0x0080239c), 0) + ((id & 0xffff) + 0x6cde) * 0x28;
}

void __fastcall Map_SpreadAdd(u8 *world, void *, u32 index, u32 cell) { Map_SpreadValues(world, nullptr, 1, index, cell); }

void __fastcall Map_SpreadRemove(u8 *world, void *, u32 index, u32 cell) { Map_SpreadValues(world, nullptr, 0, index, cell); }

u8 *__fastcall Map_StepCell(const u16 *cell, void *, s32 dx, s32 dy, s32 scale) {
    const u32 index = (u32)*cell - 1;
    const s32 x = (s32)(index & 0x7f) + dx * scale;
    const s32 y = (s32)(index >> 7) + dy * scale;
    if (!Map_InBounds(x, y)) return nullptr;
    const u32 id = ((u32)dy << 7) + *cell + (u32)dx;
    return at<u8 *>(reinterpret_cast<const void *>(0x0080239c), 0) + 0x294 + (id & 0xffff) * 0x44;
}

static u32 DirectionTable(u32 direction) {
    u32 table = 0;
    switch (direction & 0xff) {
    case 0x01: table = 0x007cdba0; break;
    case 0x02: table = 0x007cdbd8; break;
    case 0x04: table = 0x007cdbc8; break;
    case 0x08: table = 0x007cdbb8; break;
    case 0x10: table = 0x007cdba8; break;
    case 0x20: table = 0x007cdbb0; break;
    case 0x40: table = 0x007cdbd0; break;
    case 0x80: table = 0x007cdbc0; break;
    default: return 0;  // (the original's Debug_Printf assert is left out)
    }
    return table;
}

u8 *__fastcall Map_StepCellInDirection(const u16 *cell, void *, u32 direction) {
    const u32 table = DirectionTable(direction);
    if (!table) return nullptr;
    return Map_StepCell(cell, nullptr, at<s32>(reinterpret_cast<const void *>(table), 0), at<s32>(reinterpret_cast<const void *>(table), 4), 1);
}

u16 *__cdecl Map_FindFacingNeighbour(u16 *out, u32 cell) {
    const u8 *here = at<u8 *>(reinterpret_cast<const void *>(0x007cf83c), 0) + 0x294 + (cell & 0xffff) * 0x44;
    // the neighbour in a direction (1, 0x10, 4, 0x40) that is a type 3 cell (not 9) facing the opposite one (0x10, 1, 0x40, 4): its id
    static const u32 directions[4][2] = {{0x01, 0x10}, {0x10, 0x01}, {0x04, 0x40}, {0x40, 0x04}};
    for (const auto &d : directions) {
        const u8 *next = Map_StepCellInDirection(reinterpret_cast<const u16 *>(here), nullptr, d[0]);
        if (next && MapCell_IsType3Or9(next) && !MapCell_IsType9(next) && MapCell_Direction(next) == d[1]) {
            *out = at<u16>(next, 0);
            return out;
        }
    }
    *out = 0;
    return out;
}

s32 __cdecl Map_CountSpecialCells() {
    const u8 *map = at<u8 *>(reinterpret_cast<const void *>(0x0080239c), 0) + 0x2d8;
    s32 count = 0;
    for (s32 x = 0x7f; x >= 0; x--) {
        for (s32 y = 0x7f; y >= 0; y--) {
            const u32 id = ((u32)y << 7) + (u32)x + 1;
            const u8 *cell = map + (id & 0xffff) * 17 * 4 - 0x44;
            // (sic) the fourth test takes the cell as a handle: its first word
            if (MapCell_IsType9(cell) || MapCell_IsTypeA(cell) || MapCell_IsType4(cell) || MapHandle_IsTypeBDOr10Or12Or19(reinterpret_cast<const u16 *>(cell)) ||
                MapCell_IsType15(cell))
                count++;
        }
    }
    return count;
}

u32 __fastcall MapCell_IsType0(const u8 *cell) { return at<u32>(cell, 8) == 0 ? 1 : 0; }
u32 __fastcall MapCell_IsType1(const u8 *cell) { return at<u32>(cell, 8) == 1 ? 1 : 0; }
u32 __fastcall MapCell_IsType2(const u8 *cell) { return at<u32>(cell, 8) == 2 ? 1 : 0; }
u32 __fastcall MapCell_IsType4(const u8 *cell) { return at<u32>(cell, 8) == 4 ? 1 : 0; }
u32 __fastcall MapCell_IsType9(const u8 *cell) { return at<u32>(cell, 8) == 9 ? 1 : 0; }
u32 __fastcall MapCell_IsTypeA(const u8 *cell) { return at<u32>(cell, 8) == 0xa ? 1 : 0; }
u32 __fastcall MapCell_IsType15(const u8 *cell) { return at<u32>(cell, 8) == 0x15 ? 1 : 0; }
u32 __fastcall MapCell_IsType1e(const u8 *cell) { return at<u32>(cell, 8) == 0x1e ? 1 : 0; }

u32 __fastcall MapCell_IsType3Or9(const u8 *cell) {
    const u32 type = at<u32>(cell, 8);
    return type == 3 || type == 9 ? 1 : 0;
}

u32 __fastcall MapCell_IsType7Or1e(const u8 *cell) {
    const u32 type = at<u32>(cell, 8);
    return type == 7 || type == 0x1e ? 1 : 0;
}

u32 __fastcall MapCell_IsType5Or7Or2(const u8 *cell) {
    const u32 type = at<u32>(cell, 8);
    return type == 5 || type == 7 || type == 2 ? 1 : 0;
}

u32 __fastcall MapCell_IsType19(const u8 *cell) { return at<u32>(cell, 8) == 0x19 ? 1 : 0; }
u32 __fastcall MapCell_IsTypeBOr10(const u8 *cell) { return type_in(cell, 0xb, 0x10); }
u32 __fastcall MapCell_IsTypeCOr11(const u8 *cell) { return type_in(cell, 0xc, 0x11); }

u32 __fastcall MapCell_IsTypeBDOr10Or12Or19(const u8 *cell) {
    const u32 type = at<u32>(cell, 8);
    return type == 0xb || type == 0xd || type == 0x10 || type == 0x12 || type == 0x19 ? 1 : 0;
}

u32 __fastcall MapCell_IsTypeDOr12ViaParent(const u8 *cell) {
    if (MapCell_IsTypeCOr11(cell)) {
        const u8 *parent = world_cell(at<u16>(cell, 0x10));
        if (parent != nullptr && type_in(parent, 0xd, 0x12)) return 1;
    }
    return type_in(cell, 0xd, 0x12);
}

u32 __fastcall MapHandle_IsTypeBDOr10Or12Or19(const u16 *handle) {
    const u8 *cell = MapHandle_Entry28(handle);
    if (!MapCell_IsTypeCOr11(cell)) return MapCell_IsTypeBDOr10Or12Or19(cell);
    const u16 parent = at<u16>(cell, 0x10);
    return parent != 0 ? MapCell_IsTypeBDOr10Or12Or19(world_cell(parent)) : 0;
}

u32 __fastcall MapHandle_IsTypeCOr11(const u16 *handle) { return MapCell_IsTypeCOr11(MapHandle_Entry28(handle)); }

u32 __fastcall MapHandle_ParentIsType19(const u16 *handle) {
    const u8 *cell = MapHandle_Entry28(handle);
    if (MapCell_IsTypeCOr11(cell)) cell = world_cell(at<u16>(cell, 0x10));
    return MapCell_IsType19(cell);
}

void __fastcall MapCell_SetParent(u8 *cell, void *, u32 parent) { at<u16>(cell, 0x10) = (u16)parent; }

void __fastcall MapCell_SetFlags(u8 *cell, void *, u32 value) { at<u16>(cell, 0xe) = (u16)value; }

u16 __fastcall MapCell_Flags(const u8 *cell) { return at<u16>(cell, 0xe); }

void __fastcall MapCell_ClearFlags(u8 *cell, void *, u32 bits) { at<u16>(cell, 0xe) &= (u16)~bits; }

u32 __fastcall MapCell_HasLowFlags(const u8 *cell) { return (cell[0xe] & 0xf) != 0 ? 1 : 0; }

u8 __fastcall MapCell_CornerNeighbours(const u8 *cell) {
    const u8 bits = MapCell_Neighbours(cell);
    if (MapCell_SideNeighbourCount(cell) != 2) return 0;
    return (bits & 0x11) && (bits & 0x44) ? bits : 0;
}

u8 __fastcall MapCell_StraightNeighbours(const u8 *cell) {
    const u8 bits = MapCell_Neighbours(cell);
    if (MapCell_SideNeighbourCount(cell) != 2) return 0;
    return ((bits & 1) && (bits & 0x10)) || ((bits & 4) && (bits & 0x40)) ? bits : 0;
}

u8 __fastcall MapCell_JunctionNeighbours(const u8 *cell) {
    const u8 bits = MapCell_Neighbours(cell);
    return MapCell_SideNeighbourCount(cell) <= 2 ? 0 : bits;
}

u16 __fastcall MapCell_Word24(const u8 *cell) {
    if (!MapCell_IsTypeCOr11(cell)) return at<u16>(cell, 0x24);
    return at<u16>(world_cell(at<u16>(cell, 0x10)), 0x24);
}

void __fastcall MapCell_SetWord24(u8 *cell, void *, u32 value) { at<u16>(cell, 0x24) = (u16)value; }
u8 *__cdecl Cell_ExtendPath(const s32 *point, s32 *nextX, s32 *nextY) {
    if (!Map_InBounds(point[0], point[1]))
        return nullptr;
    const u8 direction = at<u8>(point, 0x10);
    MapCell_AddNeighbours(CellAt(point[0], point[1]), nullptr, Nibble_Scale(direction));
    *nextX = point[0];
    const s32 y = point[1];
    *nextY = y;
    switch (direction) {
    case 1: *nextY = y + 1; break;
    case 0x10: *nextY = y - 1; break;
    case 4: *nextX = *nextX - 1; break;
    case 0x40: *nextX = *nextX + 1; break;
    default: break;
    }
    if (!Map_InBounds(*nextX, *nextY))
        return nullptr;
    u8 *cell = CellAt(*nextX, *nextY);
    if (Kinds_Compatible(cell, nullptr, 3))
        MapCell_SetDirection(cell, nullptr, direction);
    return cell;
}

u16 *__cdecl Cell_MakeHandle(u16 *out, u32 x, u32 z) {
    *out = (u16)((z << 7) + x + 1);
    return out;
}

u8 *__cdecl MapGrid_MarkEdges(const u8 *grid) {
    u8 *const table = reinterpret_cast<u8 *>(0x008186f0);
    const s32 width = at<s32>(grid, 0), height = at<s32>(grid, 4);
    global<s32>(0x008187f0) = width;
    global<s32>(0x008187f4) = height;
    for (u32 i = 0; i < 0x100; i++)
        table[i] = 0;
    for (s32 y = 0; y < at<s32>(grid, 4); y++) {
        for (s32 x = 0; x < at<s32>(grid, 0); x++) {
            if (!IsEdgeType(GridType(grid, x, y)))
                continue;
            s32 around = 0;
            if (y - 1 >= 0 && y - 1 <= at<s32>(grid, 4) && IsEdgeType(GridType(grid, x, y - 1)))
                around = 1;
            if (x - 1 >= 0 && x - 1 <= at<s32>(grid, 0) && IsEdgeType(GridType(grid, x - 1, y)))
                around++;
            if (x + 1 >= 0 && x + 1 <= at<s32>(grid, 0) && IsEdgeType(GridType(grid, x + 1, y)))
                around++;
            if (y + 1 >= 0 && y + 1 <= at<s32>(grid, 4) && IsEdgeType(GridType(grid, x, y + 1)))
                around++;
            if (around != 4)
                table[y + 16 * x] = 1;
        }
    }
    return table;
}

u32 *__cdecl MapGrid_ClassifyTypes(const u8 *object) {
    u32 *const table = reinterpret_cast<u32 *>(0x00818800);
    const u8 *const grid = at<const u8 *>(object, 0x18);
    global<s32>(0x00818c00) = at<s32>(grid, 0);
    global<s32>(0x00818c04) = at<s32>(grid, 4);
    for (u32 i = 0; i < 0x100; i++)
        table[i] = 0;
    for (s32 y = 0; y < at<s32>(grid, 4); y++) {
        for (s32 x = 0; x < at<s32>(grid, 0); x++) {
            switch (GridType(grid, x, y)) {
            case 1: table[y + 16 * x] = 4; break;
            case 4:
            case 0x17: table[y + 16 * x] = 1; break;
            case 9: table[y + 16 * x] = 2; break;
            case 10: table[y + 16 * x] = 3; break;
            default: break;
            }
        }
    }
    return table;
}

u8 *__cdecl Cell_FindRideableObject(s32 x, s32 y) {
    u8 *const world = global<u8 *>(global<u32>(0x007cf6ec));
    const u32 id = (u32)(((y << 7) + x + 1) & 0xffff);
    const u8 *const cell = world + 0x294 + id * 0x44;
    if (!MapCell_IsType9(cell))
        return nullptr;
    const u8 *const parent = world + 0x294 + (u32)at<u16>(cell, 0x10) * 0x44;
    if (!parent)
        return nullptr;
    u16 handle = at<u16>(parent, 0x24);
    if (handle == 0)
        return nullptr;
    for (;;) {  // (the debug assert on the thing table is left out)
        u8 *const thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (thing[2] == 3 && at<u32>(Object_GetDef(thing), 0x118) == 0)
            return thing;
        handle = at<u16>(thing, 0xa);
        if (handle == 0)
            return nullptr;
    }
}
}
