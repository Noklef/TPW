// The GUI library's dialog base (vtable 0x7058f8): +4 a dword given to the constructor, words +8 (Fn_659994), +0xa and +0xc (each set only
// below the limit +0xe, Fn_659a40), +0x10, +0x12, two rectangles (ui/guirect) at +0x14 and +0x20, and words +0x2c-+0x36. The game's
// dialogs derive from it. Reimplementation of 0x6597b0-0x659cb0 and 0x6599f2 / 0x659a19.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6599f2 (thiscall (value: a word), pops 4; name unconfirmed): the word +0xa = value when value < the word +0xe (signed).
void __fastcall GuiDialog_SetA(u8 *dialog, void *, u32 value);

// 0x659a19 (thiscall (value: a word), pops 4; name unconfirmed): the word +0xc = value when value < the word +0xe (signed).
void __fastcall GuiDialog_SetC(u8 *dialog, void *, u32 value);

// 0x6597b0 (thiscall (value), pops 4; name unconfirmed): the rectangles +0x14 and +0x20 constructed (GuiRect_Ctor), the vtable 0x7058f8,
// +4 = value, the words +0x10, +0x12, +0xa, +0xc, +8 and +0x2c-+0x36 0 (+0xe left), then Fn_659994(0); returns the dialog.
u8 *__fastcall GuiDialog_Ctor(u8 *dialog, void *, u32 value);

// 0x659886 (thiscall; name unconfirmed): the vtable 0x7058f8, then the rectangles +0x20 and +0x14 destroyed (GuiRect_Dtor).
void __fastcall GuiDialog_Dtor(u8 *dialog);

// 0x659cb0 (thiscall (flags), pops 4; slot 0; name unconfirmed): GuiDialog_Dtor, the dialog freed (Mem_Free) for flags bit 0; returns it.
u8 *__fastcall GuiDialog_ScalarDtor(u8 *dialog, void *, u32 flags);

// 0x6598f7 (thiscall (flags, source), pops 8; slot 3; name unconfirmed): nothing for flags 0 or no source; else from the source's words:
// bit 0: GuiDialog_SetC(+4); bit 2: the word +0x30 = +8; bit 19: GuiDialog_SetA(+0xa); bit 6: the words +0x34 / +0x36 = +0xc / +0xe.
void __fastcall GuiDialog_Apply(u8 *dialog, void *, u32 flags, const u8 *source);
}
