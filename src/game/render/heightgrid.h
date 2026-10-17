// A grid of heights sampled at a point: reimplementation of 0x57c190 (thiscall, x87 at the game's 24-bit precision; the engine's grid
// at [0x879d60], sampled at 0x57c10c and 0x57c12b). The names are unconfirmed.
//
// The sampler (this): +0 the grid, +8 / +0xc its cells across and down (nx, nz), +0x10 / +0x14 the cells per unit across and down.
// The grid: +8 / +0xc its origin's x and z, +0x10 / +0x14 a cell's width and depth, +0x28 the heights (floats, nx a row).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x57c190 (x, z; pops 8): the cell (i, j) = __ftol((x - origin) * cells per unit) (low halves, unchecked); the next column and row
// clamped to 0..nx and 0..nz; with h00 = heights[j * nx + i], h10 the next column's and h01 the next row's, returns
// ((z - (j * depth + oz)) * per unit z) * (h01 - h00) + ((x - (i * width + ox)) * per unit x) * (h10 - h00) + h00 in st0: the plane
// through the cell's corner and its two neighbours (the index arithmetic wraps as dwords; an out-of-range cell reads outside).
double __fastcall HeightGrid_HeightAt(const u8 *sampler, void *, float x, float z);
}
