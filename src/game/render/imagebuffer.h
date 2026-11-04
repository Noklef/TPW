// An image file kept in the screen's pixel format: reimplementation of 0x586e40 and 0x586f40. Names invented, unconfirmed.
//
// The object: +0 the pixels (tracked block), +4 the width, +8 the height, +0xc the bytes a pixel.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void ImageBuffer_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x586e40 (thiscall (name), pops 4): the screen's pixel format asked of the DirectDraw surface (a failure logged; any answer but 0: the error (2, answer) pushed, 0). +0xc = its bits over 8 for driver kind 1, else 4. The file loaded (TexFile_Load) into the
// object, with a channel table made from the format for kind 1, else the table 0x8c05d8. 1 when there are pixels.
u32 __fastcall ImageBuffer_Load(u8 *self, void *, const char *name);

// 0x586f40 (thiscall (name, unused, y), pops 0xc): with pixels, the same asking of the format; a second file loaded and copied in over the rows from y up from the bottom: rows above the top skipped (its first ones), rows counted from y, none copied from the
// height on. The file's block freed. 1, and 1 without pixels or without the file.
u32 __fastcall ImageBuffer_Stamp(u8 *self, void *, const char *name, u32 unused, s32 y);
}
