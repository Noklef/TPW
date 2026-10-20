// A texture's image file read into pixels: reimplementation of 0x574bd0.
//
// The file is a WCT image (gfx/wctimage, its name's extension turned into "WCT") kept as such, or
// a TGA (with its 0x12-byte header: +0xc width, +0xe height (s16), +0x10 bits per pixel, +0x11
// the descriptor, bit 5 the rows from the top) whose pixels are converted (gfx/color's
// Format_ConvertPixels) to the device's format. 0x87a24c gets what the load found out (0x24 /
// 0x25 for a WCT image without / with its second block, 2 for its alpha, its +7 << 8; 8 for an
// alpha mask made, 0x300 and the conversion's flags for a TGA); 0x87a254 the WCT image's virtual
// base; 0x87a264 the mip levels after the first (each a level of the file and of the pixels).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x574bd0 (cdecl; name, &width, &height, flags, format table or 0 (the current one,
// TexFormat_Table on 0x8bd550), pixel size or 0 (the format's, 0x7016f8[[0x8bd554]]); name
// unconfirmed): 0x87a24c cleared. Flag 1: the raw file (VFS_Open; none: 0): its header read
// (0x12 bytes), a seek of width * height * bits / 8 (signed) from there (sic), width * height
// bytes (TrackedGlobal_Alloc; not got: 0) read and the file closed; with descriptor bit 5 the rows
// of width * bits / 32 bytes (sic: the count of dwords swapped) swapped top for bottom for all
// height rows (so each pair twice, sic; a count of 0 swaps 2^32 dwords); 0x87a25c = -1, 0x87a24c =
// 0 and the bytes returned. Otherwise the name is copied (0x104 bytes, unchecked), its extension
// (after the last '.' that is not one of the leading dots; none: a null read, sic) overwritten
// with "WCT" as far as both go, and a WCT image (GameHeap_Alloc(0x30), Obj_CtorVirtual564ba0 most
// derived; WctImage_Load on it even when not got) loaded: the width and height its u16 +8 / +0xa,
// 0x87a24c and 0x87a254 as above, the image returned. Not loaded: the image (when got) deleted
// through its virtual base's slot 0 (1), then the named file opened (none: 0), its size asked
// (slot 0x14), its header read and (mip levels + 1) * width * height * bits / 8 bytes allocated
// (not got: closed, 0) and one level read; closed. The width and height (s16) stored; flag 2
// makes a mask of each pixel's fourth byte (width * height pixels) after the level's bytes
// (width * bits / 8 * height): device kind ([0x879ab0]+0x24) 3 the byte in all four, 1 the byte
// / 2 there and (not byte) / 2 after that; 0x87a24c |= 8. Then 0x87a24c |= 0x300, the pixels
// ((mip levels + 1) * pixel size * width * height + 4, TrackedGlobal_Alloc; not got: the file's
// bytes freed, 0) converted level by level (descriptor bit 5 the flip), the flags ORed into
// 0x87a24c; the file's bytes freed and the pixels returned.
u8 *__cdecl TexFile_Load(const char *name, u32 *width, u32 *height, u32 flags, const u32 *table, s32 pixelSize);
}
