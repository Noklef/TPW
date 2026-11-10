// The engine's matrix object methods: reimplementation of 0x578800 and 0x578bc0.
//
// Unlike the matrix library (matrix.h, cdecl on float arrays) these are thiscall on a 4x4 at the
// object's start: the engine's function-local static matrices (0xfb3400, built at 0x578473) and
// the view at [0x87afe8], which the camera copies its matrix (0x790990) into (0x42aa2a, 0x42cc93).
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x5780a0 (static initialiser): the eight identity matrices at 0x87af68, 0x87afa8, 0x87aff0,
// 0x87b030, 0x87b090, 0x87b110, 0x87b158 and 0x87b198.
void __cdecl Mat4_InitGlobals();

// 0x578800 (thiscall): the identity (zeroed, then the diagonal 1.0f).
void __fastcall Mat4_Identity(u32 *m);

// 0x578bc0 (thiscall; from): the 16 dwords copied in.
void __fastcall Mat4_Assign(u32 *m, void *, const u32 *from);

// 0x577460 (thiscall; distance; pops 4; x87): the translation row moved along the first row, m[12 + i] += distance * m[i]
// (the name is unconfirmed).
void __fastcall Mat4_MoveAlongX(float *m, void *, float distance);

// 0x578820 (cdecl; lu, index, b; x87): Numerical Recipes' lubksb for n = 4: b solved in place against the LU decomposition lu
// (row-major) with its row permutation index; the forward pass starts at the first nonzero b (a NaN counts as zero), and the back
// pass divides by the diagonal unchecked. The index is not range-checked (the name is unconfirmed).
void __cdecl Mat4_LuBackSubstitute(const float *lu, const s32 *index, float *b);

// 0x578900 (cdecl; a, index, sign; x87; the name unconfirmed): Numerical Recipes' ludcmp (Crout, implicit partial pivoting) for n = 4 on a
// row-major a in place: *sign = 1, each row's scale 1 / its largest magnitude (NaNs passed over; a row of zeros gives infinity), then per
// column j the upper sums, then the lower ones weighted by |sum| * scale, the last weight >= the best so far (stored as a float) taking the
// pivot; a pivot other than j swaps the rows (the one moved down through the x87, a signalling NaN quieted), negates *sign and takes j's
// scale; index[j] = the pivot; a zero or NaN diagonal becomes 1e-20; the column below it times (float)(1 / the diagonal). The pivot is
// never reset: a column whose weights are all NaN keeps the column before's, and at the first column the stack's leftover (no row then
// has a non-NaN a[i][0] in a row with a nonzero non-NaN entry; the port has 0 there).
void __cdecl Mat4_LuDecompose(float *a, s32 *index, float *sign);

// 0x578b20 (thiscall; x87; the name unconfirmed): the matrix inverted in place by Numerical Recipes' method: a copy decomposed
// (Mat4_LuDecompose), then each column of the identity solved against it (Mat4_LuBackSubstitute) as the inverse's column.
void __fastcall Mat4_Invert(u32 *m);

// 0x5783b0 (x87; the name is unconfirmed): the matrices made from the two inputs, 1. 0x87aff0 copied to 0x87af68 and inverted; through the
// transformer ([0x87b1d8] slot 6, cdecl (self, in, by, out)) with 0x87b110 into the static 0xfb3400, which goes to 0x87b158, and that with
// 0x87b0d0 into the static 0xfb3460, which goes to 0x87b158 too. (Each static made identity and given to atexit (0x5d1750) on first use, guards
// 0xfb33e0 and 0xfb3440, bit 0.) 0x87b078 = the unit vector of ([0x87af70], [0x87af90]). With the flag 0x20000000 of 0x8bd508 the same from
// 0x87b030 by 0x87afa8 and 0x87b198, and 0x87b080 = the unit vector (Vec2_Normalize) of ([0x87afb0], [0x87afd0]). 0x87b090 made identity. From the
// matrix at [0x87afe8] its (x, y) = (+4, +0x14) as a unit vector (zero length or a NaN: c = y = 1): [0x87af60] = acos(c * y) / (2 pi), negative
// when asin(-c * x) as a float is not above 0.
u32 __cdecl Matrices_Update();

// 0x578be0 (thiscall; near, far, field of view in degrees; pops 0xc; x87): the perspective matrix scaled by the sine of half the
// angle: zeroed, then m[0] = m[5] = cos, m[10] = q = sin / (1 - near / far), m[14] = -q * near and m[11] = sin, or m[15] = 1 with the
// render flag 0x200 (0x8bcbc8) set. The three arguments are kept at 0x87b1dc, 0x768acc and 0x87af58 (the name is unconfirmed).
void __fastcall Mat4_Perspective(float *m, void *, float near_, float far_, float fov);

// 0x578ca0 (thiscall; far: pops 4; x87): Mat4_Perspective with the kept near (0x87b1dc) and field of view (0x87af58) and this far;
// then the depth d = ((far - m[14]) / m[10]) * the render view's +0x44, raised to the near when below it (or a NaN), handed to
// View_SetProjection (0x583220) as (far's bits, d * m[10] + m[14], m[11] * d + m[15]) (the name is unconfirmed).
void __fastcall Mat4_SetFar(float *m, void *, u32 far_);

// 0x578d20 (thiscall; out, in, mode; pops 0xc; x87): the vector in (x, y, z, w; row vector times the matrix) into out by mode: 0 the
// point (four components with the last row added), 1 the 4D vector (w times the last row), 2 the point's first three, 3 the
// direction (three, no last row), 4 the direction through the transposed 3x3 (the inverse rotation); other modes (unsigned) do
// nothing (the name is unconfirmed).
void __fastcall Mat4_Transform(const float *m, void *, float *out, const float *in, u32 mode);
}

// Test seam: fncmp's driver sends Matrices_Update's atexit to a recorder.
void Mat4_SetCalls(const AddrCall *table, u32 count);

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const float, gMatrices_Update_701888, 0x00701888, 0x4);
GAME_DATA(const float, gMatrices_Update_70189c, 0x0070189c, 0x4);
GAME_DATA(const double, gMatrices_Update_7018a0, 0x007018a0, 0x8);
GAME_DATA(u32, gMat4_Perspective_768acc, 0x00768acc, 0x4);
