// A quad's four vertices turned about its centre: reimplementation of 0x591430 and 0x591510 (cdecl, x87 at the game's 24-bit
// precision; used by the drawing at 0x57c96f, 0x58625a and 0x58938d). The names are unconfirmed.
//
// The quad: four vertices of x, y, z, w floats, 0x10 bytes apart. Each turn takes the vertices last to first; a vertex's two
// coordinates (a, b) become ((sin * db + da * cos) + ca, (cos * db - da * sin) + cb) with (da, db) its offsets from the centre
// (ca, cb), da (and db, in the second product) as floats and db otherwise unrounded.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x591430 (quad, angle, centre): x and y turned by angle (in turns), with the 256-entry sine table at 0x8c63d8: sin = the entry
// __ftol(angle * 256) & 0xff, cos = the entry (0x40 - __ftol(angle * -256)) & 0xff. The centre is the x, y floats at centre, or
// when that is 0, ((x0 + x1) * 0.5, (y0 + y2) * 0.5) (vertex 0's x with vertex 1's, its y with vertex 2's).
void __cdecl Quad_TurnXY(float *quad, float angle, const float *centre);

// 0x591510 (quad, angle in radians): x and z turned by fsin / fcos of angle about ((x0 + x1) * 0.5, (z0 + z2) * 0.5), z's
// centre rounded to a float.
void __cdecl Quad_TurnXZ(float *quad, float angle);
}
