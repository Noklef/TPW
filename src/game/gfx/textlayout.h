// The text layout engine (0x6ad000-0x6b1000) the GUI's text drawing (0x492750, 0x4929f0) builds on the stack: three nested objects.
// - The text style (vtable 0x70a0d8, 0x50 bytes; the global one at 0x78ba2c): +4 the font (a reference-counted object: slot 0 releases,
//   slot 1 adds a reference, slot 9 answers its flags), +8 the font's flags, +0xc a state (3 after construction or a font change, 1 after a
//   colour), +0x10 1, +0x14 a SurfaceInfo (0x28 bytes), +0x3c, +0x40, +0x44 the colour (5 bytes), +0x4c.
// - The style holder (vtable 0x70a13c, 0x58 bytes): +4 the style, +0x54.
// - The layout (vtable 0x70a10c, 0xbc bytes): +4 the holder; +0x5c, +0x70 and +0x84 one byte each before three vectors (begin, end, capacity
//   at +0x60, +0x74, +0x88), +0x94 a flag, +0x98, +0x9c, +0xa0 / +0xa4 two kinds, +0xac-+0xb8.
// Reimplementation of 0x6ad5d0, 0x6ad620, 0x6ad640, 0x6ad6e0, 0x6ad7c0, 0x6ade00, 0x6ade90, 0x6adeb0, 0x6b0230, 0x6b0250 and 0x6b0270.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ad5d0 (thiscall): the font (+4) cleared, the state (+0xc) 3, +0x10 1, the SurfaceInfo at +0x14 constructed (SurfaceInfo_Ctor),
// +0x3c, +0x40, the colour bytes +0x44-+0x48 and +0x4c cleared, the vtable 0x70a0d8. Returns this.
u8 *__fastcall TextStyle_Ctor(u8 *style);

// 0x6ad640 (thiscall): the vtable 0x70a0d8; a font (+4) is released (its slot 0) and cleared.
void __fastcall TextStyle_Dtor(u8 *style);

// 0x6ad620 (thiscall (flags), pops 4): TextStyle_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextStyle_DeletingDtor(u8 *style, void *, u32 flags);

// 0x6ad7c0 (thiscall (status, font), pops 8): the state (+0xc) 3; the old font released (slot 0) and cleared; the new one stored, and when
// it is not 0 given a reference (slot 1) and its flags (slot 9) stored at +8. *status = 0. Returns status.
s32 *__fastcall TextStyle_SetFont(u8 *style, void *, s32 *status, u8 *font);

// 0x6ad6e0 (thiscall (status, colour), pops 8): with a font (+4) whose flags (+8) have bit 1, the colour's 5 bytes go to +0x44, the state
// (+0xc) becomes 1 and *status = 0; otherwise *status = -1. Returns status.
s32 *__fastcall TextStyle_SetColour(u8 *style, void *, s32 *status, const u8 *colour);

// 0x6b0230 (thiscall): the style at +4 constructed, the vtable 0x70a13c, +0x54 cleared. Returns this.
u8 *__fastcall TextStyleHolder_Ctor(u8 *holder);

// 0x6b0270 (thiscall): the vtable 0x70a13c, then the style at +4 destroyed (a jump to TextStyle_Dtor).
void __fastcall TextStyleHolder_Dtor(u8 *holder);

// 0x6b0250 (thiscall (flags), pops 4): TextStyleHolder_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextStyleHolder_DeletingDtor(u8 *holder, void *, u32 flags);

// 0x6ade00 (thiscall): the holder at +4 constructed; +0x5c, +0x70 and +0x84 get the top byte of `this` (the byte the original reads from its
// pushed ECX, sic: an allocator temporary that was never set); the vectors at +0x60, +0x74, +0x88, the word +0x6c, the byte +0x94 and the
// dwords +0x98, +0x9c, +0xac-+0xb8 cleared (+0xa0-+0xa8 are left); the vtable 0x70a10c. Returns this.
u8 *__fastcall TextLayout_Ctor(u8 *layout);

// 0x6adeb0 (thiscall): the vtable 0x70a10c; the vectors at +0x88, +0x74 and +0x60 (in that order) freed (Mem_Free, null too) and their
// three dwords cleared; then the holder at +4 destroyed.
void __fastcall TextLayout_Dtor(u8 *layout);

// 0x6ade90 (thiscall (flags), pops 4): TextLayout_Dtor, then the object freed (Mem_Free) for flags bit 0. Returns this.
u8 *__fastcall TextLayout_DeletingDtor(u8 *layout, void *, u32 flags);

}
