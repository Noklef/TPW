// The render mode word (0x79d86c, set at 0x449e7d) and two scale globals set next to it:
// reimplementation of 0x44a590-0x44a632.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x44a590: the first of 0x2000, 0x1000, 0x800 set in the mode word, else 0x4000 when that is
// set, else 0x1000.
u32 __cdecl RenderMode_Size();

// 0x44a5d0: the first of 8, 0x10 set in the mode word, else 0x20 when that is set, else 8.
u32 __cdecl RenderMode_Depth();

// 0x44a600: 0x79d9e4 = 1.0f. 0x44a620: 0x79d9e8 = 1.0f / 8388608.0f (0x6fe2a0 / 0x6fe270).
void __cdecl RenderMode_ResetScale();
void __cdecl RenderMode_InitEpsilon();

// 0x44a3d0 / 0x44a3f0: the same pair for 0x79d9c4 (1.0f) and 0x79d9cc (1.0f / 8388608.0f,
// 0x6fe268 / 0x6fe238), static initialisers too; nothing reads either.
void __cdecl RenderMode_InitUnusedOne();
void __cdecl RenderMode_InitUnusedEpsilon();
}
