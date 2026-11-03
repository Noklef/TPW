// The tiles of a rectangle of a grid of 64-unit cells: reimplementation of 0x577ae0.
#pragma once

#include "../../../re/types.h"

// What TileGrid_Draw calls (the port by default; the bench runs it as a recorder).
struct TileGridEnv {
    u32(__fastcall *drawQuad)(u8 *object, void *, u8 *spec, u32 mask);  // Batch_DrawQuad (0x565f90)
};
// The call goes to env (nullptr: the port).
void TileGrid_SetEnv(const TileGridEnv *env);

extern "C" {

// 0x577ae0 (thiscall, on the object at 0x87aee8; name invented, unconfirmed). The object: +0x10 the grid's width, +0x1c its cells (0x60 bytes each; null returns 0), +0x20 to +0x2c the rectangle's x0, y0, x1, y1,
// +0x30 to +0x3c its corners on screen likewise, +0x40 colour (an alpha of 0xff: batch flags 0x1256, else 0x556), +0x44 z (with 0.03 added in render mode 4), +0x48 the first cell's index, +0x5c 0: batch flag 0x20000.
// Each cell of the rectangle (in 64s, the outer ones cut to it) is a Batch_DrawQuad with mask -3, by the cell's +8 flags (0x1000: its +0x1c the batch's extra; 0x100000: polygon flag 4, which stays for the later
// cells, sic), its +0x20 / +0x24 UV size and +0x28 / +0x2c UV start, trimmed by half a texel's 0.3 or, with render flag 2, 1.0 in v, when the +0x18 holder holds a texture. An answer 0 returns 0, else 1.
// Sic: the spec and its points stay from one cell to the next, so what the call changes in them carries over. The polygon flags start as stack garbage (0 here). The original's Texture_FormatBits of each cell is
// left out: its answer is overwritten. Its null test of the cell is left out: a null cell faults either way. Over 32 columns or 64 rows the tables run over the original's return address: not kept.
u32 __fastcall TileGrid_Draw(u8 *object, void *);
}
