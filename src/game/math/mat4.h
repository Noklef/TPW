// The engine's matrix object methods: reimplementation of 0x578800 and 0x578bc0.
//
// Unlike the matrix library (matrix.h, cdecl on float arrays) these are thiscall on a 4x4 at the
// object's start: the engine's function-local static matrices (0xfb3400, built at 0x578473) and
// the view at [0x87afe8], which the camera copies its matrix (0x790990) into (0x42aa2a, 0x42cc93).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x578800 (thiscall): the identity (zeroed, then the diagonal 1.0f).
void __fastcall Mat4_Identity(u32 *m);

// 0x578bc0 (thiscall; from): the 16 dwords copied in.
void __fastcall Mat4_Assign(u32 *m, void *, const u32 *from);
}
