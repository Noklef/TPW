// The width x height x bytes-per-pixel buffer (Array3D_Read, 0x5e78b0, reads one): reimplementation
// of 0x5e7740, 0x5e77c0, 0x5e77f0 and 0x5e7ef0, used by the sign renderer (0x5ecc8f-0x5ecf5e).
//
// +0 width, +4 height, +8 bytes per pixel, +0xc the pixel bytes. The return values of these
// thiscalls are whatever the code left in EAX; they are kept.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5e7740 (thiscall; a, b, c, d): without pixels, nothing. At 4 bytes a pixel every pixel
// (width * height of them, the count re-read after each) becomes the bytes d, a, b, c; at 1 byte
// the byte d; other depths nothing. Returns the pixel pointer (after the last pixel written).
u8 *__fastcall Array3D_Fill(u8 *buffer, void *, u32 a, u32 b, u32 c, u32 d);

// 0x5e77c0 (thiscall; dest): the width * height * bytes bytes copied into the dest's pixels,
// dwords first then the rest, forwards. Returns the dest buffer, or this buffer without pixels.
u8 *__fastcall Array3D_CopyTo(u8 *buffer, void *, u8 *dest);

// 0x5e77f0 (thiscall): each of width * height 4-byte pixels has its bytes reversed (ARGB <-> BGRA).
// Returns the pixel pointer after the last (the pixels' own start when none are done).
u8 *__fastcall Array3D_ReverseBytes(u8 *buffer);

// 0x5e7ef0 (thiscall; other): over this buffer's height, the first width bytes of each row of the
// other's pixels become the larger (unsigned) of the two; the other's rows are its own width *
// bytes-per-pixel apart. Returns the other's pixel pointer after the last byte (its start without
// pixels or rows).
u8 *__fastcall Array3D_MaxInto(u8 *buffer, void *, u8 *other);

// 0x5e7840 (thiscall; a, b, c, pops 12): the block at +0xc is freed (when there is one), a new one of a * b * c bytes cleared; a, b, c at
// +0, +4, +8 (not when the block could not be made). Returns 1, or 0 when there is no block.
u8 __fastcall Array3D_Realloc(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x5d3830 (thiscall; a, b, c, pops 12): a at +0, b at +4, c at +8, a new block of a * b * c bytes at +0xc. Returns the object.
u8 *__fastcall Array3D_Init(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x5d3860 (thiscall): the dwords at +0, +4 and +8 = 0; the block at +0xc, when there is one, is freed and the pointer cleared.
void __fastcall Array3D_Free(u8 *object);

// 0x5eb090 (thiscall destructor-like): Array3D_Free on the arrays at +0x2c, +0x3c, +0x4c and +0x5c, then for +0x5c, +0x4c, +0x3c and +0x2c in that order the block at +0xc of
// each freed when still set and its four dwords cleared.
void __fastcall ArrayHolder_Clear(u8 *object);

// 0x5eb170 (thiscall): Array3D_Free of the members at +0x2c, +0x3c, +0x4c and +0x5c.
void __fastcall Array3Ds_Free4(u8 *object);
}
