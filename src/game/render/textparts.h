// The renderer's text parts: a RendererPart (render/renderer, 0x1a0 bytes in all) with a text layout (gfx/textlayout) at +0xe4, the
// base class's vtable 0x6ff320 and eleven classes derived from it (vtables 0x6ff350-0x6ff530, every 0x30). Their constructors, destructors
// and deleting destructors: reimplementation of 0x48f6a0-0x491a00 (three functions a class). The names are unconfirmed.
//
// A constructor (thiscall (value), pops 4) runs RendererPart_Ctor(value) and TextLayout_Ctor(+0xe4), sets +0x48-+0x4a to 0xff, +0xd8 0,
// +0xd9 1, +0xda, +0xdb 0, the words +0xde, +0xe0 0, +0xdc 0xff, +0xdd 0 and its class's vtable; 0x6ff380 and 0x6ff4d0 also set the
// layout's flag (+0x178, the layout's +0x94) to 1. Returns this.
// A destructor (thiscall) sets the base vtable 0x6ff320, destroys the layout (TextLayout_Dtor) and then the RendererPart (the original
// inlines RendererPart_Dtor); all twelve are the same code. A deleting destructor (thiscall (flags), pops 4; slot 0 of the class's
// vtable) runs its class's destructor and frees the part (Mem_Free) for flags bit 0; returns this.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x48f6a0 / 0x48f760 / 0x48f740: vtable 0x6ff320.
u8 *__fastcall TextPart_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart_Dtor(u8 *part);
u8 *__fastcall TextPart_DeletingDtor(u8 *part, void *, u32 flags);

// 0x48fde0 / 0x48fea0 / 0x48fe80: vtable 0x6ff350.
u8 *__fastcall TextPart6ff350_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff350_Dtor(u8 *part);
u8 *__fastcall TextPart6ff350_DeletingDtor(u8 *part, void *, u32 flags);

// 0x490040 / 0x490100 / 0x4900e0: vtable 0x6ff380, the layout flag set.
u8 *__fastcall TextPart6ff380_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff380_Dtor(u8 *part);
u8 *__fastcall TextPart6ff380_DeletingDtor(u8 *part, void *, u32 flags);

// 0x490250 / 0x490310 / 0x4902f0: vtable 0x6ff3b0.
u8 *__fastcall TextPart6ff3b0_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff3b0_Dtor(u8 *part);
u8 *__fastcall TextPart6ff3b0_DeletingDtor(u8 *part, void *, u32 flags);

// 0x4904c0 / 0x490580 / 0x490560: vtable 0x6ff3e0.
u8 *__fastcall TextPart6ff3e0_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff3e0_Dtor(u8 *part);
u8 *__fastcall TextPart6ff3e0_DeletingDtor(u8 *part, void *, u32 flags);

// 0x4907b0 / 0x490870 / 0x490850: vtable 0x6ff410.
u8 *__fastcall TextPart6ff410_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff410_Dtor(u8 *part);
u8 *__fastcall TextPart6ff410_DeletingDtor(u8 *part, void *, u32 flags);

// 0x4909f0 / 0x490ab0 / 0x490a90: vtable 0x6ff440.
u8 *__fastcall TextPart6ff440_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff440_Dtor(u8 *part);
u8 *__fastcall TextPart6ff440_DeletingDtor(u8 *part, void *, u32 flags);

// 0x490c30 / 0x490cf0 / 0x490cd0: vtable 0x6ff470.
u8 *__fastcall TextPart6ff470_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff470_Dtor(u8 *part);
u8 *__fastcall TextPart6ff470_DeletingDtor(u8 *part, void *, u32 flags);

// 0x490e60 / 0x490f20 / 0x490f00: vtable 0x6ff4a0.
u8 *__fastcall TextPart6ff4a0_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff4a0_Dtor(u8 *part);
u8 *__fastcall TextPart6ff4a0_DeletingDtor(u8 *part, void *, u32 flags);

// 0x491170 / 0x491230 / 0x491210: vtable 0x6ff4d0, the layout flag set.
u8 *__fastcall TextPart6ff4d0_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff4d0_Dtor(u8 *part);
u8 *__fastcall TextPart6ff4d0_DeletingDtor(u8 *part, void *, u32 flags);

// 0x491710 / 0x4917d0 / 0x4917b0: vtable 0x6ff500.
u8 *__fastcall TextPart6ff500_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff500_Dtor(u8 *part);
u8 *__fastcall TextPart6ff500_DeletingDtor(u8 *part, void *, u32 flags);

// 0x491920 / 0x4919e0 / 0x4919c0: vtable 0x6ff530.
u8 *__fastcall TextPart6ff530_Ctor(u8 *part, void *, u32 value);
void __fastcall TextPart6ff530_Dtor(u8 *part);
u8 *__fastcall TextPart6ff530_DeletingDtor(u8 *part, void *, u32 flags);

// 0x48fc40 (thiscall (flags, info), pops 8; slot 3 of vtable 0x6ff320; name unconfirmed): nothing for no info or no flags; else
// RendererPart_SetLayout, then with any of the flags 0x7fd00 and the info's +0x10 set, from that record: flag 0x400 its byte +4 to
// +0xd8, 0x800 +5 to +0xd9, 0x1000 +6 to +0xda, 0x2000 +7 to +0xdb, 0x4000 its word +0xa to +0xde, 0x8000 +0xc to +0xe0, 0x10000 the
// byte +8 to +0xdc, 0x20000 +9 to +0xdd; 0x40000 its dword +0x10 to the colour bytes +0x48..+0x4b and +0x78 their colour (+0x4b, +0x48,
// +0x49, +0x4a from the top byte down, each times the floats 1 / 255 and 255 at the x87's precision and truncated); 0x100 with its
// dword +0 set: slot 9 called with it.
void __fastcall TextPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info);

// Slot 3 of the number classes (thiscall (flags, info), pops 8; names after the vtables): without the flag 0x80 TextPart_SetLayout
// (flags, info); with it the info's dword +0 formatted (0x6acd60 with the table [0x78ba94], a number argument 'MRAV' (n, value, minus)
// into 0x3f characters on the stack) and that text sent by TextPart_SetLayout's flag 0x100 alone (to slot 9).
// 0x4903e0 (vtable 0x6ff3b0): a negative value's magnitude by the text 0x1c2 (n 0x13), else the value by 0x1c1 (n 0xd).
void __fastcall TextPart6ff3b0_SetLayout(u8 *part, void *, u32 flags, const u8 *info);
// 0x490650 (vtable 0x6ff3e0): a magnitude over 0xeeeeeee (signed) less 0xeeeeeee as 0x4903e0's; else a negative value's magnitude by 0x1c7
// (n 0x17), a positive one by 0x1c1 (n 0xd).
void __fastcall TextPart6ff3e0_SetLayout(u8 *part, void *, u32 flags, const u8 *info);
// 0x490940 (vtable 0x6ff410): 0 an empty text (not formatted), else by 0x1c1 (n 0xd).
void __fastcall TextPart6ff410_SetLayout(u8 *part, void *, u32 flags, const u8 *info);
// 0x490b80 (vtable 0x6ff440): by 0x1c3 (n 0x14).
void __fastcall TextPart6ff440_SetLayout(u8 *part, void *, u32 flags, const u8 *info);
// 0x490ff0 (vtable 0x6ff4a0): over 0x30 a 24th by 0x1bb, over 0x18 by 0x1ba (n 8; the original's minus branches there dead), else its
// magnitude and minus flag by 0x1b9 (n 0xb).
void __fastcall TextPart6ff4a0_SetLayout(u8 *part, void *, u32 flags, const u8 *info);

}
