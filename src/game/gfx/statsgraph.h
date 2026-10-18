// The park statistics graph (a Gauge at 0x7ca250 whose marks are set from these; the lines shown by the flags byte 0x751fa0, the window by the kind at
// 0x7ca2ec): reimplementation of 0x49a7b0 and 0x49a8f0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x49a7b0 / 0x49a8f0 (cdecl; names unconfirmed): from 0, the greatest / least sample (CircBufU32_144_MaxInWindow / MinInWindow with the kind [0x7ca2ec]) of
// each shown line: bits 0-7 of 0x751fa0 (re-read for each) select the CircBufU32s of 144 at +0x1f100, +0x1f350, +0x1fc90, +0x1fee0, +0x20130, +0x20380,
// +0x1f5a0 and +0x1f7f0 of the game's record (GameTable7cfb90_Get1da720_519510 of the game object [0x7cf83c]).
s32 __cdecl StatsGraph_ShownMax();
s32 __cdecl StatsGraph_ShownMin();

// 0x4a51d0 (cdecl; name unconfirmed): the same for the graph whose Gauge is at 0x7cb318: from 0, the greatest sample (window kind [0x7cb39c]) of
// each line shown by bits 0-4 of 0x753c48: the CircBufU32s of 144 at +0x20cc0, +0x21160, +0x205d0 and +0x213b0 (CircBufU32_144_MaxInWindow) and,
// for bit 1, the CircBufF32 at +0x20f10 (CircBufF32_144_MaxInWindow), of the game's record.
s32 __cdecl StatsGraph7cb318_ShownMax();
}
