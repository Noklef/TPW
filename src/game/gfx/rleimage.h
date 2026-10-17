// A run-length image's scaled draw (0x564790, reached through the virtual base thunk 0x564b00): reimplementation. The image's fields lie before the
// pointer the thunk hands over: -0x14 its rows of codes, -0x10 its palette, -0xc its width, -8 its height.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x564790 (thiscall on the adjusted image; dest {pixels, width, height, pitch}, pops 4; name unconfirmed): the pixel size is the dword of 0x701158
// the format 0x8bd554 indexes (2: u16 palette entries, else u32). Steps: x (width << 16) / dest width and y (height << 16) / dest height (signed).
// Each of height rounds takes a state (1 at first): 1 decodes a source row (its length byte skipped; codes: a signed count n > 0 then n palette
// indices, else one index repeated -n times, until the width is reached) straight to dest, or, when the widths differ, into a 0x200-byte local
// row that is then sampled at x steps into dest width pixels; then dest moves by the pitch's excess over the row (0 for no pitch) and y adds the
// y step. 2 copies the dest row above (pixel size x dest width bytes, a forward dword copy) and moves by the pitch, adding the y step. 0 skips a
// source row by its signed length byte. Then y of 0x20000 or more gives 0 and loses 0x10000; else 1 when bit 16 is set, else 2, keeping the low
// 16 bits. (A row wider than the local overruns it, sic.) Returns 1.
u32 __fastcall RleImage_DrawScaled(const u8 *image, void *, const u32 *dest);
}
