#include "mapcell.h"

#include <cstdint>

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
}
