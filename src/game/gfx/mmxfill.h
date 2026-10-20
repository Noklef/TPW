// The screen buffer's fill with a pattern: reimplementation of 0x678d30 (hand-written MMX in the original).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x678d30 (cdecl; width, rows, buffer, pitch; name unconfirmed; the loading screen's first step clears the renderer's buffer with it,
// 0x587cc2): for each of the rows (a count of 0 is 2^32, sic) the 8-byte pattern at 0x77e580 stored twice (16 bytes) at a time from the
// row's start while the width (signed, 8 a store) stays above 0 after the step, so at least once; the next row pitch bytes on.
void __cdecl Mmx_FillRows(s32 width, u32 rows, u8 *buffer, s32 pitch);
}
