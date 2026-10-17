// A holder of four arrays and a reader of eleven dwords: reimplementation of 0x5eb090 and 0x5eb1a0. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5eb090 (thiscall destructor-like): Array3D_Free on the arrays at +0x2c, +0x3c, +0x4c and +0x5c, then for +0x5c, +0x4c, +0x3c and +0x2c in that order the block at +0xc of
// each freed when still set and its four dwords cleared.
void __fastcall ArrayHolder_Clear(u8 *object);

// 0x5eb1a0 (thiscall; stream, pops 4): the eleven dwords at +0 .. +0x28 read from the stream (Object_CallSlot8Product with 4, 1). Returns true.
u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream);

// 0x55f210 (thiscall destructor-like): the byte at +4 and the dword at +0 cleared; Array3D_Free on the thirteen arrays at +8, +0x18, ..., +0xc8; the blocks of those from +0xc8 down
// to +0x28 freed and their four dwords cleared; then BufferC_FreeAndClear on +0x18 and +8.
void __fastcall TileSet_Clear(u8 *object);

// 0x5ec220 (thiscall destructor-like): slot 6 (offset 0x18) of the object at +0x14 is called; then for the groups at +0x3c4 and +0x358 the four arrays at +0x2c, +0x3c, +0x4c and
// +0x5c are Array3D_Free'd, the last one's block is freed and cleared and BufferC_FreeAndClear runs on +0x4c, +0x3c and +0x2c; then the blocks at +0x28 and +0x18 are freed
// when set and their four dwords cleared.
void __fastcall Renderer_Teardown(u8 *object);

// 0x5ea3f0 (cdecl; destination image, source image, x, y; images {width, height, bytes per pixel, data}): the destination is made over (Array3D_Realloc: the source's width
// and height, one byte per pixel, cleared) and the part of the source that is left once it is moved by (x, y) (its rows and columns clamped to the source) is copied into it, the
// first byte of each pixel, at (max(x, 0), max(y, 0)). Returns true.
u8 __cdecl Image_CropInto(u8 *destination, u8 *source, s32 x, s32 y);

}  // extern "C"
