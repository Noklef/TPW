// The graphics settings (gBalanceGraphics' values, the dwords from 0x785878) applied to the renderer's flags: reimplementation of 0x44d8f0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x44d8f0 (cdecl; name unconfirmed): each setting, by value (any other value leaves its target as it was):
//   0x785878: 0 or 1 clear bits 3-4 of 0x8bd550, 2 or 3 set them;   0x78587c: 0 clears bits 13-14 of 0x7a1a8c, 1 / 2 / 3 set them and bit 2 / 1 / 0
//   of the mode word 0x79d86c;   0x785880: 0 / 1 / 2: 0x8c162c = 1500 / 2000 / 2500;   0x785884: bit 6 of 0x8bd550 (0 clears, 1 sets);
//   0x785888: 0, 1, 2 or 4 is the landscape's (0x8c1828) detail level (Landscape_SetDetail);   0x78588c: bit 22 of 0x8bcbc8;   0x785890: its bit 16;
//   0x785894: bit 9 of the mode word;   0x785898: bits 17, 18 and 20 of 0x8bcbc8: 0 sets 17 and clears 18 and 20, 1 sets 18 and clears 17 and 20,
//   2 or 3 clear 17, 18 and 20;   0x78589c: bit 0 of 0x8bcbc8;   0x7858a0: 0 or 1 clear bits 13-14 of 0x8bcbc8;
//   0x7858a4: bit 1 of 0x8bd550;   0x7858a8: bit 21 of 0x7a1a8c;   0x7858ac: its bit 22;   0x7858b0: bit 5 of 0x8bd550;
//   0x7858b4: 0 / 1 / 2 / 3: the view 0x8bcbc8's depth range (View_SetDepthRange) 50-60 / 100-110 / 135-150 / 230-250 and the float 0x8bd4c0
//   = 200 / 325 / 450 / 700;   0x7858b8: 0 sets bits 11-12 of 0x8bcbc8, 1 clears them.
void __cdecl GraphicsSettings_Apply();
}
