// The ground the world view [0x879d54] sees, row by row: the view's four corner rays and the eye as a convex polygon on the ground, clipped to the view's ranges and scaled to cells, then turned into each row's first and last cell for the renderer (Renderer_DrawCellLists).
// Reimplementation of 0x56bb00 and 0x56d600 (x87 at the game's 24-bit precision); the names are invented (unconfirmed).
//   A scan table: +0 the first row, +4 the last, +8 each of 0x80 rows' first cell (s8), +0x88 each row's last cell (s8).
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Test seam: Mat4_SetFar 0x578ca0 and EngineDebug_Log 0x564650 replaced by address (tests/fncmp/viewfootprint.py).
void ViewFootprint_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x56d600 (thiscall (scan, out, unused, limit), pops 0x10): the polygon's (+0xd0 its count, 12-byte vertices x, y, z) edges walked into the scan table (first cells from the edges rising in z, last cells from those falling; a NaN z as falling), each row's last raised to its first. With bit 24 of the scene's flags 0x8bd508, each vertex's x and z logged ("%f --- %f"). Then out: each row widened by the next row's cells, its first row the scan's, its last the scan's unless out's old +4 is above limit, then limit (sic).
void __fastcall Polygon_ScanRows(const u8 *polygon, void *, u8 *scan, u8 *out, u32, s32 limit);

// 0x56bb00 (thiscall; called from 0x576add; returns 1): the scan +0xc4 emptied; the corners +0 .. +0x2c from the field of view 0x87af58 (as ViewQuad_SetFov); the eye and each corner's ray point (WorldView_RayPoint) added to a polygon, the far distance the longest of 10 and the rays; the polygon clipped to (+0x3c, +0x40 - +0x4c, +0x44, +0x48 - +0x50) and scaled by 1 / +0x4c and 1 / +0x50; Polygon_ScanRows into +0xc4 and +0x1cc with limit +0x58; Mat4_SetFar on 0x87b110.
u32 __fastcall WorldView_UpdateFootprint(u8 *view);
}
