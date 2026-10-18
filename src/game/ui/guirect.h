// The GUI library's rectangle (vtable 0x705920, over the base 0x705938): four words, left +4, top +6, right +8 and bottom +0xa (right and
// bottom exclusive). Dialogs (0x7058f8) hold two at +0x14 and +0x20. Reimplementation of 0x659d00-0x65aa20.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x659d00 (thiscall; name unconfirmed): the base constructor (Fn_65a9d0), the vtable 0x705920 and the four words 0; returns the rectangle.
u8 *__fastcall GuiRect_Ctor(u8 *rect);

// 0x659d43 (thiscall (left, top, right, bottom: words), pops 16; name unconfirmed): as GuiRect_Ctor with the four words given.
u8 *__fastcall GuiRect_CtorWith(u8 *rect, void *, u32 left, u32 top, u32 right, u32 bottom);

// 0x659d90 (thiscall; name unconfirmed): the vtable 0x705920, then the base destructor (Fn_66bff7), whose EAX is left.
u32 __fastcall GuiRect_Dtor(u8 *rect);

// 0x659e83 (thiscall (x, y: words), pops 8; slot 0; name unconfirmed): 1 in AX when left <= x < right and top <= y < bottom (signed), else
// 0. Only AX is set.
u32 __fastcall GuiRect_Contains(u8 *rect, void *, u32 x, u32 y);

// 0x659ee2 (thiscall (x, y: word pointers, each may be 0), pops 8; slot 1; name unconfirmed): the centre, left + (right - left) / 2
// (an arithmetic shift) and likewise top / bottom, stored through each pointer given.
void __fastcall GuiRect_Centre(u8 *rect, void *, u16 *x, u16 *y);

// 0x65a3e9 (thiscall (dx, dy: words), pops 8; slot 2; name unconfirmed): moves the rectangle: dx added to left and right, dy to top and
// bottom.
void __fastcall GuiRect_Offset(u8 *rect, void *, u32 dx, u32 dy);

// 0x65a43e (thiscall (dl, dt, dr, db: words), pops 16; slot 3; name unconfirmed): adds each to left, top, right and bottom.
void __fastcall GuiRect_Adjust(u8 *rect, void *, u32 dl, u32 dt, u32 dr, u32 db);

// 0x65aa20 (thiscall (source), pops 4; name unconfirmed): the assignment: the base's (Fn_65aa80, which does nothing), then the four words
// copied from the source; returns the rectangle.
u8 *__fastcall GuiRect_Assign(u8 *rect, void *, const u8 *source);
}
