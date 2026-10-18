// The terrain model's normals scaled by the map cells under their vertices: reimplementation of 0x450b30 and 0x450d10. The terrain
// model is a tree of nodes (the map's +0x78 the root; a node's first child +0xc, its next sibling +8): +0 flags, +0x10 its matrix,
// +0x58 / +0x60 its vertices (as WorldVerts_TransformNode reads them), +0x5e (u16) its normals' count, +0x64 the normals (three floats
// each), +0x94 each normal's vertex (u16s, indices into the world vertex block 0x7aecd4).
#pragma once

#include "../../../re/types.h"

extern "C" {

// A 4x4 float matrix passed by value.
struct TerrainMatrix {
    float m[16];
};

// 0x450b30 (cdecl; node, a matrix by value; name unconfirmed): the matrix becomes the node's (+0x10) times it (Matrix_PreMul). Unless the
// node's flags have 0x200, its vertices go into the world vertex block through it (WorldVerts_TransformNode, the grid's size read first),
// and each normal whose vertex's cell (__ftol of a tenth of its x and z) is on the grid (0x7a0854 -> +0x6c: width +0x18, height +0x1c;
// unsigned) is scaled to the length [0x74cf30] (0.225) when the cell's dword in the table at [0x7a0884] has 0x400, else to 1, on the x87:
// s = (n0 n0 + n1 n1) + n2 n2, t = sqrt(s) stored as a float, k = length / t, then n0 k and n2 k with k stored as a float, k n1 with k
// as it was. Then the same for each child with the matrix.
void __cdecl Terrain_ScaleNormals(u8 *node, TerrainMatrix m);

// 0x450d10 (cdecl; name unconfirmed): for each cell of the grid (0x7a0854 -> +0x6c; rows +0x1c, columns +0x18, both read again each step),
// the grid's flags (dwords from +0x2c) get 0x400 when the 0x44-byte map cell there (the column and row each held in [0, 0x7a0314 - 1] /
// [0, 0x7a0374 - 1], below 0 first; [[0x7cf6ec]] + 0x294 + id * 0x44, id = (row << 7) + column + 1 cut to 16 bits) has flag 0x40 and not
// 0x80 (MapCell_HasFlags), else lose it (a word cleared). Then Terrain_ScaleNormals from the map's model (+0x78) with the identity, the
// scene's flags touched (SceneFlags_Touch on 0x8bd508) and 0x79f7cc = 0.
void __cdecl Terrain_ShadeNormals();
}
