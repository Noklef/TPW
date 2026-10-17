// The renderer settings' texture format helpers: reimplementation of 0x581790, 0x583ba0, 0x5839a0, 0x5839e0 and 0x5839f0.
//
// The renderer settings are the object at 0x8bd550 (its +0 the renderer flags; +4 the current
// texture format, read all over as 0x8bd554). 0x5839f0 fills its tables, 0x1000 bytes per format
// from +0x88. With the software driver (0x8bd524 == 4) the pixel format at +0x68, laid out as a
// DDPIXELFORMAT, is made 32-bit ARGB (0x5727c3).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5839f0 (thiscall on the settings): for each of the 4 formats k (0x20 bytes apart from
// +0x8; masks a, b, c, d at +0x24, +0x18, +0x1c, +0x20 + 0x20k): +0x4088 + 0x10k = the four
// masks scaled, (mask << 16) * 0x80808081 >> 39; and when the format's +8 + 0x20k is 0x20 the
// 256-entry table at +0x88 + 0x1000k (16 bytes an entry, from the entry at +4 behind): entry i =
// (i >> (8 - width)) << shift for each mask's width and shift (Bits_Count, Bits_Shift), the
// shifts counted mod 32 as on the x86.
void __fastcall TexFormat_FillTables(u8 *settings);

// 0x583ba0 (stdcall; pixels): classifies 60 test pixels (dwords) by the colour masks of the
// current format (the software driver, 0x8bd524 == 4, uses 0x808080 and the values 0x800000,
// 0x8000, 0x80; the others take them from 0x8bd4e0, 0x8bd4c8, 0x8bd4d0, 0x8bd4d8): class 0, 1
// or 2 by which value the masked pixel equals, else 3. 0x8c1644 = the number of pixels that
// fit the pattern 1, then (3, 2) in pairs across the middle, then 1: the first pixel's
// class 1, the odd ones 3, the even ones 2, the last 1. Returns 0 when all 60 do, else -1 if
// the first pixel isn't class 0 and the last isn't class 2, else 1.
s32 __stdcall TexFormat_Classify(const u32 *pixels);

// 0x581790 (thiscall on an object whose +4 points at a flags dword and +8 is a flags dword;
// request, texture): the word for the driver to load a texture with. From the scene flags
// (0x8bcbc8: 0x2000 -> 0x100000, 0x400 -> 0x80000), the object's flags (+4's: 2 -> 0x800,
// 0x2000000 -> 0x5156, 0x8000000 -> 0x20000; +8: 0x10000000 -> 0x10000000, 0x4000000 ->
// 0x30000000, 0x8000000 -> 0x50000000), the request's byte +0xc & 2 -> 0x800000 and, when there
// is a request (its first dword) and a texture, the texture: Texture_FormatBits, or 0x1256 when
// the request's byte +1 has 8; flag 0x100000 -> 0x40000, 0x30000 -> 0x400000, and 0x400 with
// no dword at +0x3c -> 0x80. Without either, 0x1256.
u32 __fastcall TexLoad_DriverFlags(const u8 *object, void *, const u8 *request, const u8 *texture);

// 0x5839a0 (thiscall): the DDPIXELFORMAT at +0x68 = size 0x20, flags 0x41 (RGB with alpha), no
// FourCC, 32 bits, masks 0xff0000, 0xff00, 0xff and 0xff000000.
void __fastcall TexFormat_SetArgb32(u8 *settings);

// 0x5839e0 (thiscall): the current format's table, +0x88 + 0x1000 * (+4).
u8 *__fastcall TexFormat_Table(u8 *settings);

// 0x562f70 (cdecl; to, from, side; name unconfirmed): side * side 32-bit ARGB pixels put into the current texture format (0x8bd554) at to: 0 RGB565,
// 1 ARGB1555, 2 ARGB4444 (each field the channel's top bits, the fields added together), 3 a copy, of only `side` dwords (sic). Any other format pushes
// the error (2, 0xb0754003). The 565's green is the top six bits at bit 6 (sic, its top bit added onto the red's lowest) and the 4444's fields are
// green at bit 8, red at bit 4 and alpha at 0 (as the original lays them out).
void __cdecl TexFormat_ConvertSquare(u16 *to, const u32 *from, u32 side);
}
