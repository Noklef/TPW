// A ride base's cells and its frames placed on its grid. Name invented (unconfirmed).
#pragma once

#include "../../../re/types.h"
#include "gridfit.h"

// What RideBase_Prepare calls (the ports by default; the bench runs them as recorders).
struct RideBaseEnv {
    void(__cdecl *fit)(const u8 *node, u8 *frame, u16 **cursor, GridFitMatrix matrix, const u8 *grid, u32 place, s32 *hits, float lift,
                       u8 **bestFrame, s32 *bestCount, u32 flags, const u8 *only, u32 fine);  // Frame_FitToGrid (0x469a80)
    void(__cdecl *fillCells)(const u8 *object, const u8 *grid, u32 width, u32 height, u32 all);  // RideBase_FillCells (0x46aa10)
    void *(__cdecl *alloc)(u32 size, u32 zero);                                                  // GlobalMem_Alloc (0x45b9d0)
};
// The calls go to env (nullptr: the ports).
void RideBase_SetEnv(const RideBaseEnv *env);

// 0x46a5a0 (cdecl; object, owner, cells, width, height). holder = object +8, model = holder +4. The edge table [0x7afe20] (20 by 20 dwords)
// is cleared; with cells (rows of 0xa0, cells of 8 bytes) and without model +0x30's bit 0x80 [0x7afe18] = width and [0x7b0460] = height,
// and each cell gets 4, 2, 1, 8 where a cell's low four bits differ (as set or not) from those of the one past it and of three
// more (next row, this cell, last row, the one after: not there counts as clear); else both 0.
// With an owner and its +0x6c, the fit from the identity matrix (node model +0x78 twice, grid owner +0x6c, flags model +0x30's bit 0x100)
// fills a stack buffer of u16s: its best frame gets bit 0x4000 and goes to holder +0x2c. holder +0xc = the hits, and above 0
// holder +0x14 = a copy of them (alloc, zeroed; none if null). Then each of model +0x44's frames at model +0x70 (0xa0 each) with bit 0x40,
// other than the best: the fit with only that frame, flags 0, fine 1 (owner +0x6c read, with or without an owner), and over 0 hits its +0x98 = a block
// of (count, pointer to what follows, the hits). With cells: unless model +0x30 lacks 8 (Debug_Printf in the original: left out), or there
// is no best frame without 0x80, RideBase_FillCells. The best count starts as stack garbage when the first fit is skipped (0 here).
void __cdecl RideBase_Prepare(const u8 *object, const u8 *owner, const u8 *cells, s32 width, s32 height);
