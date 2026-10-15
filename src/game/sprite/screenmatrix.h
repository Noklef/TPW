// The matrix that puts a sprite on the screen: reimplementation of 0x476c50 (called by the
// sprite drawing at 0x4770e2, 0x477f05 and 0x478555).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x476c50: out = shift * turn * scale (Matrix_Mul, each from the identity), for a w x h
// sprite at (x, y) showing width x height units from (left, top), on a 2048 x 1536 frame
// centred on (1024, 768) (sx = width / w, sy = height / h):
//   shift: row 3 = (x sx - left - 1024 sx, (1536 - y) sy - top - 768 sy);
//   scale: 2 / sx * [0x74f888], 2 / sy * [0x74f88c], 0 down the diagonal;
//   turn (identity for angle 0): about (cx, cy), (cx - 1024) sx and ((1536 - cy) - 768) sy,
//   moved there (its negation), rotated (cos, sin * -4/3; sin * 3/4, cos; gSinTable at angle
//   pi / 2048 a unit) and moved back.
// The 16-bit arguments are read sign-extended from their low words.
void __cdecl Sprite_ScreenMatrix(float *out, s16 x, s16 y, s16 w, s16 h, float left, float top, float width,
                                 float height, s16 cx, s16 cy, s16 angle);
}
