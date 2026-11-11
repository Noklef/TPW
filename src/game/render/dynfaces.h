// The dynamic face queue of the scene: reimplementation of its two writers 0x53c8d0 and 0x53c9c0. The count of faces is the dword at 0x871ee0 (at most 0x806, past which a
// message is printed (left out) and nothing is added); each face is 0xa0 bytes from 0x820b00, with its tag word (n << 5 | tag & 0x1f) in the table at 0x872168.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x53c8d0 (cdecl; a, b, c, d byte, tag, e, f, g, h byte, i, j): a square face. Nothing is added for the tag 1 while the flag byte at 0x763c98 is 0. The face has a, b, c at
// +0, +4, +8, d at +0xc, e, f, g at +0x10, +0x14, +0x18, h at +0x1c, j at +0x90, the dword 1 at +0x94, i (1 while the dword at 0x874fc4 is not 0) at +0x98 and the tag byte at +0x9c.
void __cdecl DynFaces_AddSquare(u32 a, u32 b, u32 c, u8 d, u32 tag, u32 e, u32 f, u32 g, u8 h, u32 i, u32 j);

// 0x53c9c0 (cdecl; corners (4 x 3 dwords), tag, uvs (4 x 2 dwords, stride 12), colours (4 x 3 dwords), user): a polygon face of four corners: the corners at +0..+0x2b, the
// uvs at +0x30 (stride 12), the colours at +0x60 (stride 12), user at +0x90, 0 at +0x94 and +0x98 and the tag byte at +0x9c.
void __cdecl DynFaces_AddPoly(const u32 *corners, u32 tag, const u32 *uvs, const u32 *colours, u32 user);

// 0x53cab0 (cdecl; dx, dy, dz, rows, cols, row, col; its callers push two more, 0x10 and 0x13, unread; name unconfirmed): the quads of a
// grid 19 vertices wide (positions at 0x874180, colours at 0x870ed8, 12 bytes each, both as 0x53d570 fills them) queued as polygon faces
// (DynFaces_AddPoly, user 1): for each column i from col (cols of them) and each row from row (rows of them), the corners are the vertex
// and those below, right and below-right of it, offset by (dx, dy, dz) on the x87, with their colours (through the x87: a signalling NaN
// comes out quiet), the uvs (0, 0) (1, 0) (0, 1) (1, 1) in that order, and the tag the next of the dwords at 0x871d18.
void __cdecl DynFaces_AddGrid(float dx, float dy, float dz, s32 rows, s32 cols, s32 row, s32 col);

// 0x53cda0 (cdecl; dx, dy, dz, xoff, rows, cols, tag, angle; name unconfirmed; 0x53c6e8 calls it with xoff 0.5, 6 rows, 4 columns, tag
// 0x12): that grid from its first vertex, turned about y by angle, as two polygon faces per quad (DynFaces_AddPoly, user 3, the tag given),
// all on the x87 at the game's precision. With s and c the sine table's (sintable.h) entries for angle and angle + pi / 2, a vertex
// (x + xoff, y, z) turns to x' = c x - z s, z' = z c + s x and then moves by (dx, dy, dz); its colour (the normal at 0x870ed8) turns
// likewise. For each column (v from 1, or 0.55 for tag 0x13, down by 0.55 / cols) and row (u from 0 up by 1 / rows), the first face has
// the vertices below, here, below-right and right with the colours negated and the uvs (u + du, v) (u, v) (u + du, v - dv) (u, v - dv);
// the second here, below, right and below-right with the colours as they are and the uvs (u, v) (u + du, v) (u, v - dv) (u + du, v - dv).
// The original's four turning loops write the products in different orders, kept (they decide between two NaNs).
void __cdecl DynFaces_AddTurnedGrid(float dx, float dy, float dz, float xoff, s32 rows, s32 cols, u32 tag, float angle);

// 0x53d380 (cdecl; rows, cols, step, phase; name unconfirmed): that grid filled as a wave, x87 at the game's precision with the sine table
// (sintable.h): with k = 2 pi / 3 and A = 10 degrees, for each column (z = 0, step, ...) and row (x = 0, step, ...) the vertex is
// (x, 1.7 cos h, z), h = x * k cos A - z * k sin A - phase / 2, and its normal (sin h, 1, sin h) normalised.
void __cdecl DynFaces_WaveGrid(s32 rows, s32 cols, float step, float phase);

// 0x53d570 (cdecl; rows, cols, step, phase; name unconfirmed): that grid filled as a ripple, likewise: for each column (z = 0, -step, ...) and
// row (x = 0, step, ...), with d = sqrt(x^2 + (z - cols step)^2) - 0.6 phase and the fall x / ((rows - 4) step) up to 1 (a NaN kept), the
// vertex is (x, 0.6 fall cos(d pi / 2) + z - 0.02 x^2, 0.3 fall cos(2 pi d / 3)) and its normal (s, s, 1) normalised, s = fall sin(2 pi d / 3).
void __cdecl DynFaces_RippleGrid(s32 rows, s32 cols, float step, float phase);

// 0x53c1f0 (cdecl; tags, count; name unconfirmed): the count tag words at tags[1..count] (the caller hands 0x872168 - 4) sorted by their tag (the low five
// bits): a quicksort with a stack of 50 dwords and insertion sorts of runs under 7, unsigned throughout (a count of 0 goes to the partition). Sic: the
// insertion sort steps down to tags[1], not to the run's start.
void __cdecl DynFaces_SortTags(u32 *tags, u32 count);

// 0x53c810 (cdecl; name unconfirmed): with the flag at 0x874fcc set, each of the three objects the set-up at 0x53bfe0 makes (the pointers at 0x871f64,
// 0x871ee4, 0x871ed8, in that order; null ones skipped, none cleared, sic) has its animator at +0x54 put to sleep (Animator_Sleep) and disposed of
// (Animator_Dispose) and is freed (Mem_Free); then bit 0 of the dword at 0x763c78 is cleared (Dword_ClearBit0). The flag is cleared either way.
void __cdecl DynFaces_ReleaseModels();

// 0x53c3f0 (cdecl, no arguments; called at 0x54e2fc; name unconfirmed): the scene's generated faces for a frame, x87 at the game's
// precision. The ground offsets' moves (the ints at 0x8186b8 / 0x8186b4 * [0x700de0] less the floats at 0x820afc / 0x820af8) taken
// first; the four corners (x, y at 0x871f24 + 12 i: (-0.4, -0.4), (0.4, -0.4), (-0.4, 0.4), (0.4, 0.4)) and the other constants from
// 0x871ee8 set, the corners turned by the sine table's entries (rounded indices, fistp) for t ([0x790a38]) and t - [0x700df0] (scaled)
// and moved by -[0x700df4]; 0x820afc / 0x820af8 raised by the moves * [0x700de4]. Then the wave (DynFaces_WaveGrid 16 x 19, step 20, phase
// [0x871f60]), two grids (DynFaces_AddGrid offset by 0x870ec8.., 10 x 11 from the start and 4 x 6 from row 10), the ripple
// (DynFaces_RippleGrid 7 x 5, step 1.8, phase [0x871ed0]), eight turned grids (DynFaces_AddTurnedGrid at the offsets 0x763cf4 + 16 k,
// xoff 0.5, 6 x 4, tags 0x12 and 0x13 by turns, the angle the float +0x5c of the world's record (GameTable7cfb90_Get1da724_519590 of
// [0x7cf83c]) * [0x700dd0]); the tags sorted (DynFaces_SortTags) and the three blocks [0x871f64], [0x871ee4], [0x871ed8] given their
// faces (VertexBlock_TakeFaces), the count 0. Unless the clock 0x785970 is paused (+0x30): [0x874fc0] less [0x700df8], the phases
// [0x871f60] and [0x871ed0] less the clock's frame time (its double +0x20) * [0x700de8], and the counter 0x874fc8 up by the frame time
// (__ftol); past 6 with the flag byte 0x763c98 set, or past 1 with it clear, the flag flips and the counter is 0.
void __cdecl DynFaces_Frame();

// 0x53bfe0 (name invented, unconfirmed): 0x874fc4 cleared; with the holder 0x763c78 awake (0x424ec0) the queue's counts cleared, the flag byte 0x763c98 set, the table 0x871d18 dealt 15-17 at random,
// and the three models made (0x871f64, 0x871ee4, 0x871ed8; 0xa4 bytes each, VertexBlock_Construct with 0x600, 0x86, 0x180 cubes), a result of 0 putting the holder to sleep once (0x424ed0); 0x874fcc set.
void __cdecl DynFaces_InitModels();

}  // extern "C"

void DynFaces_SetCalls(const AddrCall *table, u32 count);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gDynFaces_Frame_700dd0, 0x00700dd0, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700de0, 0x00700de0, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700de4, 0x00700de4, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700de8, 0x00700de8, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700df0, 0x00700df0, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700df4, 0x00700df4, 0x4);
GAME_DATA(const float, gDynFaces_Frame_700df8, 0x00700df8, 0x4);
GAME_DATA(u32[8], gDynFaces_763c78, 0x00763c78, 0x20);   // the textures' record: flags, count 20, the folder 0x74cb98, 0, the entries 0x763b38
GAME_DATA(u8, gDynFaces_763c98, 0x00763c98, 0x1);
GAME_DATA(float[8][4], gDynFaces_Frame_763cf0, 0x00763cf0, 0x80);   // the turned grids' rows (-1, x, y, z)
GAME_DATA(u8[0x806][0xa0], gDynFaces_Faces_820b00, 0x00820b00, 0x503c0);   // the face queue (at most 0x806 faces)
GAME_DATA(float[3], gDynFaces_GridAt_870ec8, 0x00870ec8, 0xc);   // the grid's place (x, y, z)
GAME_DATA(float, gDynFaces_871ed0, 0x00871ed0, 0x4);   // the ripple's phase
GAME_DATA(u8 *, gDynFaces_871ed8, 0x00871ed8, 0x4);
GAME_DATA(u32, gDynfaces_871ee0, 0x00871ee0, 0x4);
GAME_DATA(u8 *, gDynFaces_871ee4, 0x00871ee4, 0x4);
GAME_DATA(float[10][3], gDynFaces_Uv_871ee8, 0x00871ee8, 0x78);   // the corners' uv rows (0xc each; rows 5-8 turned each frame)
GAME_DATA(float, gDynFaces_WavePhase_871f60, 0x00871f60, 0x4);
GAME_DATA(u8 *, gDynFaces_871f64, 0x00871f64, 0x4);   // the third model
GAME_DATA(u32[0x806], gDynFaces_Tags_872168, 0x00872168, 0x2018);   // the faces' sort tags (index << 5 | tag)
GAME_DATA(float, gDynfaces_874fc0, 0x00874fc0, 0x4);
GAME_DATA(s32, gDynFaces_874fc8, 0x00874fc8, 0x4);
GAME_DATA(u32, gDynFaces_874fcc, 0x00874fcc, 0x4);

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0xc], gText_DynFaces_InitModels_763e3c, 0x00763e3c, 0xc);
GAME_DATA(char[0xc], gText_DynFaces_InitModels_763e48, 0x00763e48, 0xc);
GAME_DATA(char[0x10], gText_Dynfaces_763e54, 0x00763e54, 0x10);
GAME_DATA(u8[0xe40], gDynFaces_870ed8, 0x00870ed8, 0xe40);
GAME_DATA(u8[0x1b8], gDynFaces_871d18, 0x00871d18, 0x1b8);
GAME_DATA(u8[0xe40], gDynFaces_874180, 0x00874180, 0xe40);
