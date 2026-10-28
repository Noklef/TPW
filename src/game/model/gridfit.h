// A model's frames fitted to a height grid: reimplementations of 0x469a80 and 0x469940 (cdecl, x87 at the game's 24-bit precision).
// The names are unconfirmed.
//
// A node (the mesh): +8 the next sibling, +0xc the first child, +0x60 its vertices in groups of 4 (x, y and z 0x10 apart, 0x30 a
// group). A frame (the node's state): +0 flags, +8 the next sibling, +0xc the first child, +0x10 its matrix, +0x58 the vertex count
// (u16), +0x60 its vertices (grouped as the node's), +0x78 / +0x84 the low and high corners of its box (x, y, z each).
// The grid (as render/heightgrid's): +8 / +0xc its origin's x and z, +0x10 / +0x14 a cell's width and depth, +0x18 / +0x1c its cells
// across and down (W, H), +0x28 the heights (floats, W + 1 a row), +0x2c a dword of flags a cell (bit 2: the cell's split runs the
// other way).
#pragma once

#include "../../../re/types.h"

extern "C" {

// A 4x4 matrix passed by value (16 floats on the stack).
struct GridFitMatrix {
    float m[16];
};

// 0x469a80 (node, frame, cursor, matrix by value, grid, place, hits, lift, best frame out, best count, flags, only, fine; 28 dwords,
// cdecl): the frame's matrix (+0x10) pre-multiplied into matrix (Matrix_PreMul). Unless the frame is skipped (flags bit 17 clear
// with flags bit 1 set, or with place set and the frame's bit 12 clear; flags bit 17 set with flags bit 2 set; frame bit 9 set;
// only given and not the frame), each vertex i of the count (place set with the frame's bit 17 clear: only i == **cursor when the
// frame's bit 12 is set, none when clear) is taken from the node's group into the matrix's space (Matrix_TransformPoint) and its x and z
// put on the grid: (p - origin) / cell (below zero or NaN: 0) split by modf into a whole cell and a fraction. With the frame's bit 17
// clear the whole cell (or the next, fraction 0.5 up) must lie on the grid (0 .. W and 0 .. H) or the function returns at once
// (children left out), and with place clear it is clamped below [0x7afe18] / [0x7b0460]. Near a grid line (|fraction| or
// |fraction - 1| below 0.1 fine, 0.01 not) place snaps the coordinate onto it. Place: y = lift-adjusted height (the line's
// interpolation, the cell's corner when not fine, the cell's triangle with the frame's bit 17 set), the point put back through the
// inverse matrix into the frame's vertex and its box. Not place (a contact test): a vertex with |y| under 100 (fine; 0.01 not) on a
// line (both, the frame's bit 17 clear and fewer misses than fine + 1) whose edge [0x7afe20] marks (flags bit 0: any) counts: the
// frame's bit 12 set, (*hits)++, **cursor = i and *cursor advanced. A best count above *best count makes it the best (frame into
// *best frame out). Then each child node with the frame's child, siblings in step (the node's chain decides the end), with the
// same arguments and the multiplied matrix.
void __cdecl Frame_FitToGrid(const u8 *node, u8 *frame, u16 **cursor, GridFitMatrix matrix, const u8 *grid, u32 place, s32 *hits,
                             float lift, u8 **bestFrame, s32 *bestCount, u32 flags, const u8 *only, u32 fine);

// 0x469940 (object, owner, lift as float bits, flags): with an owner holding a grid (+0x6c), the object's root's (+4) frames (+0x78)
// and nodes ((+0x9c)->+0x78) placed on the grid from the identity matrix (Frame_FitToGrid, place, the cursor the object's (+8)->+0x14,
// flags | 1 when the root's +0x30 has bit 8), then for each of the root's +0x44 (u16) frames (+0x70, 0xa0 bytes each) holding a
// sub-object (+0x98) the tree fitted again, fine, only that frame, the cursor the sub-object's +4; the root frame's bit 16 and the
// root's bit 18 (+0x30) set.
void __cdecl Model_FitToGrid(const u8 *object, const u8 *owner, u32 lift, u32 flags);
}
