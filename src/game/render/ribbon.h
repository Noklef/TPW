// A ribbon of quads, one point at a time, in the render object's settings block (meshqueue.h). Reimplementation of 0x57dd10 (name invented, unconfirmed).
//
// The object has 16 ribbons, [+0xa188] the current one: +0x9da4 its point count, +0x9de4 its last point (0x10 bytes each), +0x9ee4 / +0x9ef4 its left and right
// edges at the last point (0x20 bytes each), +0xa0e4 its sideways x and y (8 bytes each); the width +0xa16c (over the ribbon's number + 1), the corners' u and v
// +0xa170, +0xa174, +0xa178, +0xa17c, the colour +0xa19c.
#pragma once

#include "../../../re/types.h"

// What it calls (the port by default; its bench runs it as a recorder).
struct RibbonEnv {
    u32(__fastcall *addPolygon)(u8 *object, void *, u8 *vertices, u32 count, u32 stride, const u8 *codes, u32 flags);   // DrawBatch_AddPolygon (0x565bb0)
};
// The calls go to env (nullptr: the port).
void Ribbon_SetEnv(const RibbonEnv *env);

extern "C" {
// 0x57dd10 (thiscall (from, to); name invented, unconfirmed; 0x57eaef, 0x57eba4, 0x57ec0e and 0x57ed1a call it): 1, or 0 when a polygon was refused. Nothing
// when from and to are the same place (their x, y, z each equal or unordered). Counts the point. The ribbon's first point (none without from): both through the
// projection ([0x87b1d8]'s slot 1, cdecl (it, [0x87b088], out, in, 0x10, 0xc, 2, 0)): the last point is to's, the sideways from - to scaled to the width / (ribbon + 1)
// and turned a quarter for the edges. Later: to through the matrix [0x87b088] (Mat4_Transform, mode 0). With from the new sideways is the step to the new point
// scaled to the width / (ribbon + 1), the edges the last point's with the sum of the old and new sideways scaled likewise, and the point, sideways and edges are
// kept; without from the edges are the new point's, the step scaled and turned, and nothing is kept. A step of no length (or unordered), or such a sum: only the
// count. Then the quad old left, old right, new right, new left through [0x879ab0] (DrawBatch_AddPolygon, 0x20 bytes each, no codes).
u32 __fastcall Ribbon_AddPoint(u8 *object, void *, const float *from, const float *to);
}
