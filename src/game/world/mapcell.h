// The map's cell tables (gMapCells, 0x8023a0: 0x44-byte cells from +0, 0x28-byte ones from
// +0x110000, both named by a 1-based u16 handle) and the "World Struture" memory ranges at
// 0x7cdb30: reimplementation of 0x4d0a70-0x4d0b10; and the map's (gWorld + 0x2d8) bounds check,
// RE cells, fixed positions and block values, 0x4d8300-0x4d8ca0, and the cell constructor
// 0x4d8da0; the 0x28-byte cells' accessors 0x522700-0x5228a0, 0x535db0, 0x536300-0x536440,
// 0x539210, 0x53a510-0x53a530 and 0x53ad20-0x53aef0.
// A position is the u16 handle
// y * 128 + x + 1.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d0af0 (thiscall on a u16 handle): its 0x28-byte entry, gMapCells + 0x10ffd8 + handle * 0x28.
u8 *__fastcall MapHandle_Entry28(const u16 *handle);

// 0x4d0b10 (thiscall on a u16 handle): its 0x44-byte cell, gMapCells + (handle - 1) * 0x44.
u8 *__fastcall MapHandle_Cell44(const u16 *handle);

// 0x4d0a70 (thiscall on the ranges; index, a, b, name): entry index (8 bytes) = (a, b); the
// name is unused. The world gives (itself, its size); the cell tables below (start, end).
void __fastcall MemRanges_Set(u8 *ranges, void *, u32 index, u32 a, u32 b, const char *name);

// 0x4d0a90 / 0x4d0ac0 (cdecl; base, stride): ranges entry 2 / 3 (0x7cdb40 / 0x7cdb48) =
// (base, base + stride * 0x4000), and the stride to 0x7cdb50 / 0x7cdb54.
void __cdecl MemRanges_SetCells44(u32 base, u32 stride);
void __cdecl MemRanges_SetCells28(u32 base, u32 stride);

// 0x4d8300 (cdecl): whether x and y are both 0-127 (signed).
u32 __cdecl Map_InBounds(s32 x, s32 y);

// 0x4d8410 (thiscall on the map; pos, its low 16 bits): the RE cell (5 u16s) of pos - 1, from
// map +0x1b1104.
u8 *__fastcall Map_RECell(u8 *map, void *, u32 pos);

// 0x4cd720 (cdecl): over all 128 x 128 cells of the world's map ([0x80239c] + 0x2d8), those not of type 2
// or 7/0x1e count; of those, the ones whose RE cell has a signed word at +6 above 0 are counted too;
// returns the second count times 100 over the first (a signed division), or 0 when the first is 0.
s32 __cdecl Map_PercentWithRE();

// 0x4cd7f0 (cdecl): over all 128 x 128 cells, how many are of type 9, 0xa, 4 or 0x15 or pass MapHandle_IsTypeBDOr10Or12Or19 on their first word taken as a handle (sic).
s32 __cdecl Map_CountSpecialCells();

// 0x4d96f0 (thiscall on a cell id word; dx, dy, scale, pops 12): the cell the offset (dx, dy) times scale from the cell is, for the map's 128 x 128 (Map_InBounds), at
// world + 0x294 + 0x44 times (the id + dx + dy * 128, 16 bits) (the offset is not scaled there, sic); null when it falls outside.
u8 *__fastcall Map_StepCell(const u16 *cell, void *, s32 dx, s32 dy, s32 scale);

// 0x50c320: the same for the 0x28-byte entries: world + (the id + 0x6cde) * 0x28 (the world pointer at 0x80239c), null outside the map.
u8 *__fastcall Map_StepCell28(const u16 *cell, void *, s32 dx, s32 dy, s32 scale);

// 0x50c3e0 (thiscall on a cell id word; direction bit, scale, pops 8): Map_StepCell28 by the direction's pair (as Map_StepCellInDirection) and the scale; null for other directions.
u8 *__fastcall Map_StepCell28InDirection(const u16 *cell, void *, u32 direction, s32 scale);

// 0x4d8480 (thiscall on the world block; add, index, cell id, pops 12): to the five words of every cell within the radius (the byte at +0x1d910e of the index's 12-byte
// record, at world + index * 12, clamped to the map) of the cell (the words at world + 0x1b1104 + (id - 1) * 10), the five words of the record (at +0x1d9104) are added, each
// multiplied by +1 (add set) or -1 and divided (truncating) by the cell's distance, x + y + 1.
void __fastcall Map_SpreadValues(u8 *world, void *, u32 add, u32 index, u32 cell);

// 0x4d8440 / 0x4d8460 (thiscall; index, cell id, pops 8): Map_SpreadValues with add set / clear.
void __fastcall Map_SpreadAdd(u8 *world, void *, u32 index, u32 cell);
void __fastcall Map_SpreadRemove(u8 *world, void *, u32 index, u32 cell);

// 0x4d7bb0 (thiscall on the world block): clears its first 0x1b1104 bytes, gives the
// 128 x 128 cells their ids (the words at world + id * 0x44 - 0x44 and world + id * 0x28 + 0x10ffd8, the bytes 1 at +id * 0x44 - 0x42 and 2 at id * 0x28 + 0x10ffda) and
// the eight region records at +0x1d9104 (12 bytes: five words, the radius byte at +0xa) the values of gBalanceLevel's region effects.
void __fastcall World_InitBlock(u8 *world);

// 0x4de670 (cdecl; out word, cell id): the first of the four neighbours (steps 0x7cdba0, 0x7cdba8, 0x7cdbc8, 0x7cdbd0 of the cell at [0x7cf83c] + 0x294 + id * 0x44,
// by Map_StepCellInDirection) that is a type 3 cell but not 9 and has the direction (+0xd) 0x10, 1, 0x40, 4 respectively: its id word; else 0. Returns out.
u16 *__cdecl Map_FindFacingNeighbour(u16 *out, u32 cell);

// 0x4d97e0 (thiscall on a cell id word; direction bit, pops 4): Map_StepCell by the (dx, dy) pair of the direction (the pairs from 0x7cdba0, 0x7cdbd8, 0x7cdbc8, 0x7cdbb8,
// 0x7cdba8, 0x7cdbb0, 0x7cdbd0, 0x7cdbc0 for the bits 1 to 0x80) with scale 1; null for anything else (an assert is left out).
u8 *__fastcall Map_StepCellInDirection(const u16 *cell, void *, u32 direction);

// 0x4d8610, 0x4d8650, 0x4d8690, 0x4d86d0, 0x4d8710 (thiscall on the map, unused; &pos, b):
// the position of the A (b 0) or B one of gBalanceLevel's ticket booths (+0x55c), bus stops
// (+0x56c), entrances (+0x57c), park side crossings (+0x58c) and bus stop side crossings
// (+0x59c), each an A x, y then a B x, y. The two crossings' B takes A's y (sic). Return the
// pointer.
u16 *__fastcall Map_TicketBoothPos(const u8 *map, void *, u16 *out, u32 b);
u16 *__fastcall Map_BusStopPos(const u8 *map, void *, u16 *out, u32 b);
u16 *__fastcall Map_EntrancePos(const u8 *map, void *, u16 *out, u32 b);
u16 *__fastcall Map_CrossingParkSidePos(const u8 *map, void *, u16 *out, u32 b);
u16 *__fastcall Map_CrossingBSSidePos(const u8 *map, void *, u16 *out, u32 b);

// 0x4d8c60 (thiscall on the map; value, pos) / 0x4d8ca0 (pos): the dword of pos's 16x16 block
// in a 33-wide grid from map +0x1b0000, (y / 16) * 33 + x / 16, set (not for pos 0) or read.
void __fastcall Map_SetBlockValue(u8 *map, void *, u32 value, u32 pos);
u32 __fastcall Map_BlockValue(const u8 *map, void *, u32 pos);

// 0x4d8cd0 (thiscall on the map; bx, by, pops 8): the same dword by block coordinates, by * 33 + bx
// (neither is checked).
u32 __fastcall Map_BlockValueAt(const u8 *map, void *, u32 bx, u32 by);

// 0x4d8da0: a 0x44-byte map cell, zeroed but for u16 +0 = 0x423f and byte +2 = 1. Returns it.
u8 *__fastcall MapCell_Ctor(u8 *cell);

// 0x502460: an RE cell (5 u16 mpEffect, serialized at 0x502490) zeroed. Returns it.
u8 *__fastcall RECell_Ctor(u8 *cell);

// 0x502470 (other): whether the two RE cells' 10 bytes are the same.
u32 __fastcall RECell_Equals(const u8 *cell, void *, const u8 *other);

// 0x50c1d0: a 0x28-byte track cell (serialized at 0x50c200), zeroed but for byte +2 = 2.
// Returns it.
u8 *__fastcall TrackCell_Ctor(u8 *cell);

// 0x510870 (x, y): the map id of the cell, (y << 7) + x + 1 cut to 16 bits; -1 when Map_InBounds
// says it is off the map.
s32 __cdecl Map_IdAt(s32 x, s32 y);

// A 0x28-byte cell's fields as MapCellBase_Serialize (0x4d0b30) names them: +0xc mNeighbours and
// +0x22 mHoardingNeighbours (a bit for each of the 8 neighbours, the sides the even ones), +0xd
// mDirection (bits likewise), +0x20 mOverlapCounter (u16), +8 mType, +0xe mFlags (u16), +0x10
// mParentID (u16). Byte +2 is its kind, 2 a track cell's; u16 +0 its own handle. The world's
// cell for a handle is gWorld + 0x1102b0 + handle * 0x28 (the same as MapHandle_Entry28's).
// The neighbour methods (thiscall on the cell) work on mHoardingNeighbours for a track cell while
// 0x81b4cc is set (around the hoardings' placing, 0x532408 / 0x532814), else on mNeighbours.

// 0x522700 / 0x522730 (bits, the low 8): the neighbour bits set / cleared.
void __fastcall MapCell_AddNeighbours(u8 *cell, void *, u32 bits);
void __fastcall MapCell_RemoveNeighbours(u8 *cell, void *, u32 bits);

// 0x522770: the neighbour bits (in AL only; the rest of EAX is left as it was).
u8 __fastcall MapCell_Neighbours(const u8 *cell);

// 0x522790 / 0x522810: how many of the neighbour bits are set / of the 4 side ones.
u32 __fastcall MapCell_NeighbourCount(const u8 *cell);
u32 __fastcall MapCell_SideNeighbourCount(const u8 *cell);

// 0x5227e0 (value) / 0x5227f0 (bits): mDirection set / its bits cleared. 0x522850: mDirection
// (in AL only). 0x522860: how many of its 4 side bits are set.
void __fastcall MapCell_SetDirection(u8 *cell, void *, u32 value);
void __fastcall MapCell_ClearDirection(u8 *cell, void *, u32 bits);
u8 __fastcall MapCell_Direction(const u8 *cell);
u32 __fastcall MapCell_SideDirectionCount(const u8 *cell);

// 0x522890: mOverlapCounter decremented. 0x5228a0 (value): mOverlapCounter set. 0x5228b0: mOverlapCounter, sign-extended.
void __fastcall MapCell_DecOverlap(u8 *cell);
void __fastcall MapCell_SetOverlap(u8 *cell, void *, u32 value);
s32 __fastcall MapCell_Overlap(const u8 *cell);

// 0x535db0 (flags): 1 when any of mFlags' bits in flags (the low 16) is set. 0x5363b0: mFlags'
// bit 10.
u32 __fastcall MapCell_HasFlags(const u8 *cell, void *, u32 flags);
u32 __fastcall MapCell_Flag400(const u8 *cell);

// 0x536390, 0x536310, 0x536300, 0x536380, 0x536340, 0x536350, 0x5363e0, 0x5363a0: 1 when mType
// is 0, 1, 2, 4, 9, 0xa, 0x15, 0x1e. 0x536320: 3 or 9. 0x536360: 7 or 0x1e. 0x5363c0: 5, 7 or 2.
u32 __fastcall MapCell_IsType0(const u8 *cell);
u32 __fastcall MapCell_IsType1(const u8 *cell);
u32 __fastcall MapCell_IsType2(const u8 *cell);
u32 __fastcall MapCell_IsType4(const u8 *cell);
u32 __fastcall MapCell_IsType9(const u8 *cell);
u32 __fastcall MapCell_IsTypeA(const u8 *cell);
u32 __fastcall MapCell_IsType15(const u8 *cell);
u32 __fastcall MapCell_IsType1e(const u8 *cell);
u32 __fastcall MapCell_IsType3Or9(const u8 *cell);
u32 __fastcall MapCell_IsType7Or1e(const u8 *cell);
u32 __fastcall MapCell_IsType5Or7Or2(const u8 *cell);

// 0x53ad60: mType 0x19. 0x53ad70: 0xb or 0x10. 0x53ad90: 0xc or 0x11 (a cell with a parent).
// 0x53ad30: 0xb, 0xd, 0x10, 0x12 or 0x19.
u32 __fastcall MapCell_IsType19(const u8 *cell);
u32 __fastcall MapCell_IsTypeBOr10(const u8 *cell);
u32 __fastcall MapCell_IsTypeCOr11(const u8 *cell);
u32 __fastcall MapCell_IsTypeBDOr10Or12Or19(const u8 *cell);

// 0x53adb0: mType 0xd or 0x12, or for a 0xc / 0x11 cell its parent's (the world's cell).
u32 __fastcall MapCell_IsTypeDOr12ViaParent(const u8 *cell);

// 0x5363f0 (thiscall on a u16 handle): 0x53ad30 of its cell (MapHandle_Entry28), or for a 0xc /
// 0x11 cell of its parent (0 for none). 0x536440 (likewise): 0x53ad90 of its cell.
u32 __fastcall MapHandle_IsTypeBDOr10Or12Or19(const u16 *handle);
u32 __fastcall MapHandle_IsTypeCOr11(const u16 *handle);

// 0x536450 (thiscall on a u16 handle): whether its 0x28-byte entry (MapHandle_Entry28) is of type 0x19; for an entry of type 0xc or 0x11, whether the world's cell of its parent
// (the word at +0x10) is.
u32 __fastcall MapHandle_ParentIsType19(const u16 *handle);

// 0x539210 (u16 parent, passed as a dword): mParentID set.
void __fastcall MapCell_SetParent(u8 *cell, void *, u32 parent);

// 0x53a510 (value) / 0x53a520 / 0x53a530 (bits): mFlags set / read (in AX only) / bits cleared.
// 0x53ad20: 1 when any of mFlags' low 4 bits is set.
void __fastcall MapCell_SetFlags(u8 *cell, void *, u32 value);
u16 __fastcall MapCell_Flags(const u8 *cell);
void __fastcall MapCell_ClearFlags(u8 *cell, void *, u32 bits);
u32 __fastcall MapCell_HasLowFlags(const u8 *cell);

// 0x53ae00 / 0x53ae50: the neighbour bits when exactly 2 of the sides are set and they make a
// corner (one of 1 / 0x10 with one of 4 / 0x40) / a straight (1 and 0x10, or 4 and 0x40), else 0.
// 0x53ae90: the neighbour bits when more than 2 sides are set, else 0. All in AL only.
u8 __fastcall MapCell_CornerNeighbours(const u8 *cell);
u8 __fastcall MapCell_StraightNeighbours(const u8 *cell);
u8 __fastcall MapCell_JunctionNeighbours(const u8 *cell);

// 0x53aec0: the u16 +0x24, or for a 0xc / 0x11 cell its parent's (in AX only). 0x53aef0
// (value): the u16 +0x24 set.
u16 __fastcall MapCell_Word24(const u8 *cell);
void __fastcall MapCell_SetWord24(u8 *cell, void *, u32 value);
// 0x52a050 (cdecl; the path point {x, y, +0x10 direction byte}, the next x out, the next y out): 0 when the point is outside the map (Map_InBounds); otherwise the cell at the point
// (in the map at [0x7cf83c] + 0x294, 0x44 bytes each, the id (y << 7) + x + 1 cut to 16 bits) gets the neighbour bits of the direction (MapCell_AddNeighbours with Nibble_Scale),
// the next point is the point moved by the direction (1: y + 1, 0x10: y - 1, 4: x - 1, 0x40: x + 1, any other: not moved), 0 again when that is outside; else the cell there is
// returned, and gets the direction (MapCell_SetDirection) when Kinds_Compatible(cell, 3) holds.
u8 *__cdecl Cell_ExtendPath(const s32 *point, s32 *nextX, s32 *nextY);

// 0x4cd4e0 (cdecl; out, x, z): the word *out = (z << 7) + x + 1. Returns out.
u16 *__cdecl Cell_MakeHandle(u16 *out, u32 x, u32 z);

// 0x52c420 (cdecl; the grid {width, height, then 8-byte cells in rows of 20 from +8, the type first}): the byte table at 0x8186f0 (16 x 16, +y + 16 * x) is cleared and the width and height
// kept at 0x8187f0 / 0x8187f4; a cell of type 4, 9, 0x17 or 0xa gets 1 unless all four of its neighbours (left and up only inside the grid; right and below up to one past the size)
// are of those types too. Returns the table.
u8 *__cdecl MapGrid_MarkEdges(const u8 *grid);

// 0x52c5b0 (cdecl; the object whose grid is the pointer at +0x18): the dword table at 0x818800 (16 x 16, +y + 16 * x) is cleared, the width and height kept at 0x818c00 / 0x818c04, and the
// cells get 4 for type 1, 1 for type 4 or 0x17, 2 for type 9 and 3 for type 0xa. Returns the table.
u32 *__cdecl MapGrid_ClassifyTypes(const u8 *object);

// 0x42a340 (cdecl; x, y): the cell at (x, y) of the world behind the pointer at [0x7cf6ec] must be of type 9 (MapCell_IsType9); its parent cell (the word at +0x10 indexes the cells)
// has a child chain (the first handle at +0x24, each thing's next at +0xa) and the first thing of model 3 whose object definition has the dword at +0x118 clear is returned; 0 for none.
u8 *__cdecl Cell_FindRideableObject(s32 x, s32 y);

// 0x523070 (cdecl (&x, &y)): the cell at (x, y) (the 0x44-byte cell [0x7cf83c] + 0x294 + id * 0x44, id = (y << 7) + x + 1 cut to 16 bits) and its
// entry (MapHandle_Entry28); for an entry of type 0xc or 0x11 the parent's instead (the entry [0x7cf83c] + (its u16 +0x10 + 0x6cde) * 0x28, and its
// cell MapHandle_Cell44). Nothing unless the entry has flag 0x10 (MapCell_HasFlags). Then for the pairs at 0x7cdba0, 0x7cdbc8, 0x7cdba8 and 0x7cdbd0 in
// turn, the entry Map_StepCell28 reaches with the pair and a scale of 2, when there is one, has flag 0x10, its cell (MapHandle_Cell44) the same u16
// at +0x10 and a type of 0xb, 0xd, 0x10, 0x12 or 0x19 (MapCell_IsTypeBDOr10Or12Or19): y -= 2, x += 2, y += 2, x -= 2 respectively.
void __cdecl MapCell_PathNudge(s32 *x, s32 *y);

// 0x4d8750 (cdecl; x, y, direction, mode; name unconfirmed): 1 when a step from the cell (x, y) (bytes; the cell row * 128 + column + 1 of the world
// behind [0x7cf6ec]) in the direction (byte: 0 y - 1, 1 x + 1, 2 y + 1, 3 x - 1; past 3, sic, the cell of the x argument's low word) is blocked, else
// 0. Off the map: blocked. Into type 1 from a cell not of type 3 or 9: open; type 4 to type 4: open; into type 4: blocked; into 0x1e: open but in mode
// 2. In mode 2 a cell of flag 0x400 is open, and a ride cell (type 9) whose parent (+0x10) has an object (kind 3) among its things (+0x24, next +0xa,
// handles into 0x7cfb90) is blocked unless that ride is busy (Ride_IsBusy with its +0x20) and its definition's +0x118 is 0. Types 5, 7 and 2:
// blocked. Into type 3 or 9 (from 1, 3 or 9) or into 1 from 3 or 9: open when the target's neighbour bit for the way back (0x10, 0x40, 1, 4) is set.
// Otherwise: a target of type 0xb, 0xd, 0x10, 0x12 or 0x19 whose entry (its parent's for 0xc / 0x11) has none of the low flags is blocked; from
// other than 1, 3 or 9: open; mode 1 from 1 into other than 1, 3, 9 or 4: open; mode 2 from 1 into 0: open; from 0xa or 9 into 0 facing the way
// (+0xd: 4, 0x40, 0x10, 1 for directions 3, 1, 0, 2): open; else blocked.
u32 __cdecl MapCell_StepBlocked(u32 x, u32 y, u32 dir, s32 mode);
// 0x510510 (cdecl; two cell handles, mode; name unconfirmed): 1 when the step from the first cell (row * 128 + column + 1, the low word) to the
// second, a neighbour, is open (MapCell_StepBlocked in that mode), else 0. Not neighbours (sets the print-once flag 0x7cf38c) or the second argument
// -1 (sic, its whole dword): 0. Straight: the one step. Diagonal: round one corner (down-right: right then down; the others: up or down then across)
// or, failing that, the other.
u32 __cdecl MapCell_StepOpen(u32 from, u32 to, s32 mode);
}
