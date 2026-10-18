// The engine's matrix object methods: reimplementation of 0x578800 and 0x578bc0.
//
// Unlike the matrix library (matrix.h, cdecl on float arrays) these are thiscall on a 4x4 at the
// object's start: the engine's function-local static matrices (0xfb3400, built at 0x578473) and
// the view at [0x87afe8], which the camera copies its matrix (0x790990) into (0x42aa2a, 0x42cc93).
#pragma once

#include "../../../re/types.h"

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
