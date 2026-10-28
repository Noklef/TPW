// A track's line of cells walked and the cells off it removed (name invented, unconfirmed): the world's 0x28-byte cells ([0x7cf83c] +
// 0x1102b0 + id * 0x28) and their 0x44-byte map cells. Reimplementation of 0x5311c0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5311c0 (cdecl, no arguments; name invented, unconfirmed): the byte 0x818c50 = 4. From the cell at (0x7639f0, 0x7639f4), facing 4 (a
// direction kept in a stack slot's low byte; the callees read only that), a walk two cells at a time: on a cell of kind 0x19, the cells
// two, four... to one side (facing 1: east, 4: south, 0x10: west, 0x40: north) up to the first with flag 0x20 (+0xe) each have their map
// cell (MapHandle_Cell44) and the map cells south, south-east and east of it (Map_StepCell, not checked for none) lose flag 0x40 and get
// their models (MapCell_ApplyPathModel) when any of the four has it; the north walk also stops once its row has gone negative, and for
// good (the flag is never reset, sic). A corner (MapCell_CornerNeighbours) then faces its bits but the reverse of the facing
// (Nibble_Scale). The walk ends when the cell two steps on (Map_StepCell28InDirection) exists and is not of kind 0x19, or (from the
// second step) the cell has one neighbour; else it steps on (facing 1: north, 4: east, 0x10: south, 0x40: west). Last, every cell of
// the map but its last row and column ([0x81b1b0] rows, [0x81d740] columns), or for one of kind 0xc / 0x11 its owner's, of kind 0x19
// without flag 0x20 is removed (TrackCell_Remove (0, 0, 0), or TrackCell_ClearBlock when its map cell is of kind 0x15), its overlap
// counter 0 (MapCell_SetOverlap), and its map cell and those south, south-east and east lose flag 0x40 and get their models.
void __cdecl TrackLine_Prune();

// 0x452ca0 (cdecl (x, y); name invented, unconfirmed): x and y held in [0, [0x7a0314] - 1] / [0, [0x7a0374] - 1] (below 0 first, so a
// size of 0 or less gives -1); inside the map (Map_InBounds) the map cell there ([0x7cf83c] + 0x294 + id * 0x44) is cleared
// (MapCell_Clear (0, 0)) and its track cell (MapHandle_Entry28, taken first) removed (TrackCell_Remove (0, 0, models 1)).
void __cdecl TrackCell_ClearAt(s32 x, s32 y);
}
