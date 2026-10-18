// The terrain model's nodes' cell bounds: reimplementation of 0x46eeb0 and 0x46f0c0. Each node (see normals.h for the tree) has a box
// (+0x78 one corner, +0x84 the other, three floats each); its cells go to +0x90 as four signed bytes, and its flags (+0) get 0x2000
// and, for a box off the grid, 0x4000000.
#pragma once

#include "../../../re/types.h"
#include "normals.h"

extern "C" {

// 0x46eeb0 (cdecl; node, grid, a matrix by value; name unconfirmed): the matrix becomes the node's (+0x10) times it (Matrix_PreMul).
// Unless the node's flags have 0x200, its box's corners go through it (Matrix_TransformPoint) and each axis is put in order (swapped
// only when ordered and the wrong way round); then on the x87, with the grid's origin (+8 x, +0xc z) and cell size (+0x10, +0x14):
// x0 = __ftol((x - origin) / size), less 1 when the corner's x is below 0 or NaN (sic: by the corner's sign, not the cell's), z0 likewise,
// x1 = __ftol((x - origin) / size - -0.999) and z1 likewise from the far corner. Flag 0x4000000 when x0 or z0 is below 0 or x1 / z1 is
// not below the grid's width / height (+0x18, +0x1c; signed); x0 and z0 held at -0x80 or more, x1 and z1 at 0x7f or less; flag
// 0x2000; +0x90 = x0 << 24 | x1 << 16 | z0 << 8 | z1 (each a byte). Then each child (+0xc, next +8) without flag 0x40000000 the same.
void __cdecl Terrain_NodeCells(u8 *node, const u8 *grid, TerrainMatrix m);

// 0x46f0c0 (cdecl; map; name unconfirmed): Terrain_NodeCells for the map's model (+0x78) on its grid (+0x6c) with the identity.
void __cdecl Terrain_ModelCells(const u8 *map);
}
