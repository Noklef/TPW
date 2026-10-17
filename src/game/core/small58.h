// A wide string map lookup, a thing's move, the back of a queue and a holder's end of motion: reimplementation of 0x4f2b40, 0x50b6a0, 0x4de130 and 0x45bdd0.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4f2b40 (thiscall on the map; out, key, pops 8): the node WStringMap_LowerBound finds for the key when it is not the head (the dword at +4) and the key does not sort before the
// node's string, else the head. *out is set and out returned.
u32 *__fastcall WStringMap_Find(u8 *map, void *, u32 *out, const u8 *key);

// 0x50b6a0 (thiscall on the thing; x, y, pops 8): mX and mY (the words at +4 and +6) are set; when the cell (the high bytes of the new x and y, taken as column and row against
// the bytes at +5 and +7) changes the thing is first unlinked from the old cell's chain (Things_UnlinkFromChain on the cell id (row << 7) + column + 1 of [0x8023a0]) and
// afterwards put at the head of the new one's (Obj_ReplaceHandle24). The assert on the new cell is left out.
void __fastcall Thing_SetPos(u8 *thing, void *, u32 x, u32 y);

// 0x4de130 (thiscall on the object; out word, pops 4): the id at +0x3a when it is set; else the cell Object_NeighbourCellId gives is followed through Map_FindFacingNeighbour
// (up to 1000 steps, counted at +0x40, the last cell found kept at +0x3a). Not reaching the end of the chain in time gives 0 (a message is printed, left out).
void __fastcall Object_BackOfQueue(u8 *object, void *, u16 *out);

// 0x45bdd0 (cdecl; the holder): the part at +0x1c (null, or the pointer adjusted by the virtual base offset in the word at +4 of its vtable) leaves the renderer's slot list of
// [0x87a6d0] (tag 0x10 when flag 0x10 is set, else 0x20 when flag 8 is set, else 0x70); the flags lose 0x19.
void __cdecl Holder_Sleep(u8 *holder);

}  // extern "C"
