// A ride base's cell table: for each cell of its grid, the triangle of the model that lies best along the axes there, and how its texture runs.
#pragma once

#include "../../../re/types.h"

// Its calls (a bench's recorders in their place).
struct BaseCellsEnv {
    u8 *(__cdecl *markEdges)(const u8 *grid);                                                                // MapGrid_MarkEdges (0x52c420)
    void *(__cdecl *alloc)(u32 size, u32 zero);                                                              // GlobalMem_Alloc (0x45b9d0)
    void(__cdecl *transform)(const u8 *frame);                                                               // WorldVerts_TransformFrame (0x4628b0)
    void(__cdecl *tidy)(const u8 *holder, u8 *part, float tolerance, const u8 *map, const float *positions); // RideBase_TidyAllUVs (0x4679d0)
};
void BaseCells_SetEnv(const BaseCellsEnv *env);

extern "C" {
// 0x46aa10 (cdecl (object, grid, width, height, all); name invented, unconfirmed; 0x46a9db calls it): the object's +8 part; its model +0x2c, else its +4 holder's +0x78. The part's +0x18 gets a zeroed block of a dword for each
// cell (MapGrid_MarkEdges(grid) gives a byte for each cell, x * 16 + y; the model's world vertices made, 0x7aecd4). Each triangle (the model's groups +0x6c, 0x10 each, count the word +0x5a; a group's +4 first and +6 count of
// the triangles +0x70, 8 each, their words +2, +4, +6 through the word table +0x94 into the vertices) goes to the cell of its centre (over [[0x7a0854] + 0x6c]'s +0x10 and +0x14); its group's material (+0) has bit 11 set, or
// cleared when not all and the cell's byte is 0. Its edges' largest |x| and largest |z| of the unit directions, both over 0.9 and either over the cell's so far, make it the cell's. RideBase_TidyAllUVs(object, model, 0.02, marked, vertices).
// Then each cell (not all: needs its byte unless no marks, and the grid's dword at +8 + 8 * (y * 20 + x)) with a triangle whose two are still over 0.9: the block's words are the flags and the material's index (from the holder's +0x50,
// 8 each); the two edges have a common corner and the fourth corner of the texture's rectangle is made from the three (the UVs +0x68, 0x20 for each four corners, u at +0, v at +0x10); the corner nearest the least u and least v gives
// 0x80, 0x88, 0x90 or 0xa0, and 0x40 when that corner's next edge is longer in u than v. A cell without keeps its word 1 and 0x2710. The part's +0 gets bit 6.
// Left out: the two Debug_Printf calls, and their counts. Sic: 20 by 20 cells, unchecked, as the original's frame, and those outside width by height are never initialised (a triangle there reads and writes them).
void __cdecl RideBase_FillCells(const u8 *object, const u8 *grid, u32 width, u32 height, u32 all);
}
