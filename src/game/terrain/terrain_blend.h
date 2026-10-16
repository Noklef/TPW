// Procedural terrain textures: reimplementation of the MMX tile blender.
// See tools/lnd.py for the file format and a Python reference of the same blend.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x55e710: 256-entry ramp, 0 below centre - (width+1)/2, 255 above centre + (width+1)/2.
void __cdecl Lnd_BuildRamp(u8 *table, s16 centre, s16 width);

// 0x55e780 (cdecl; control, x, y, channel, out): the original's own interface: bilinear upsampling of one channel of a control map over the tile (x, y) into the Array3D out (its
// size at +0 is the tile size; a size that is not 4 * 2^n only gets cells of 1 x 1; the cells are 4 x 4, the control map's pitch at +0 and data at +0xc). Returns 1.
u32 __cdecl Lnd_BuildWeightMap(const Array3D *control, s32 x, s32 y, s32 channel, const Array3D *out);
void __cdecl Lnd_BlendRow16(const u8 *a, const u8 *b, const u8 *wa, const u8 *tabA, const u8 *c, const u8 *d, const u8 *wc, const u8 *tabC, const u8 *mul, const u8 *mix, u8 *out);

// 0x55e780 (writes into a plain buffer instead of an Array3D): bilinear upsampling of one
// control channel over tile (x, y) to size * size bytes. y counts from the top of the
// control map. Returns false for a size the original cannot handle (not 4 * 2^n).
bool TerrainBlend_WeightMap(const Array3D *control, u32 x, u32 y, int channel, int size, u8 *out);

// 0x55f780 (__thiscall, 4 stack arguments): blends tile (x, y) of the current level into
// out (tile_size^2 pixels, bytes 0, R, G, B), then draws the overlay over it. y counts from
// the bottom, as the game passes it. scratch must hold tile_size^2 pixels.
void TerrainBlend_TileInto(const Lnd *lnd, u8 *out, u32 x, u32 y, const TerrainOverlay *overlay,
                           u8 *scratch);

// Hook entry for 0x55f780, using the game's scratch buffer (gTerrainBlendScratch 0x8798c8).
void __fastcall TerrainBlend_Tile(const Lnd *self, void *edx, u8 *out, u32 x, u32 y,
                                  const TerrainOverlay *overlay);
}
