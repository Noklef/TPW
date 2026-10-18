// The view's corner quad (the object at 0x879d54, freed by 0x56b760): four corners of three floats on the plane z = 1, set from a field
// of view: reimplementation of 0x56b790.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x56b790 (thiscall; fov in degrees; name unconfirmed): h = tan(fov pi / 360) / 2 and w = h / the aspect float at 0x8bcbcc (h kept on
// the x87 for that), x87 at the game's precision: the corners (-w, h, 1), (w, h, 1), (w, -h, 1), (-w, -h, 1). The game passes 90. Sic:
// from an angle of 2^63 on fptan pushes nothing, the fxch after it meets an empty register, and h is the indefinite.
void __fastcall ViewQuad_SetFov(float *quad, void *, float fov);
}
