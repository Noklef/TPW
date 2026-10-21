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

// 0x659f3d (thiscall (shape), pops 4; name unconfirmed): how the shape (its words +4 / +6 / +8 / +0xa read as a rectangle's, whatever it
// is) lies against the rectangle, in AX: 0x200 when they are the same (the shape's bottom compared with the top, sic); 0x100 when the
// shape contains (its slot 0) the points (left + 1, top + 1) and (right - 1, bottom - 1); else the rectangle's own slot 0 tells which of
// the shape's corners it contains (top left 0x51, top right 0x92, bottom right 0xa8, bottom left 0x64, or-ed), returned when any of the
// bits 0xf is set; else the shape's edges crossing it: with the shape spanning it across (left <= left, right >= right), its top (0x10)
// and bottom (0x20) strictly between the top and bottom; spanning it down, its left (0x40) and right (0x80) strictly between the sides.
u32 __fastcall GuiRect_Overlap(u8 *rect, void *, u8 *shape);

// 0x65a1c3 (thiscall (shape, first, second: rectangles, each may be 0), pops 12; name unconfirmed): the rectangle split by the shape's
// edge that crosses it (GuiRect_Overlap): its top (0x10) or bottom (0x20) into the parts above and below it, its left (0x40) or right
// (0x80) into the parts left and right of it, the first / second given set (Fn_659dac) to (0x10) above / below, (0x20) below / above,
// (0x40) left / right, (0x80) right / left; then 1 in AX. 0 in AX with no shape, when the shape contains it (0x100), or otherwise (a
// corner inside, or apart): then the first given is set to the rectangle itself. Only AX is set.
u32 __fastcall GuiRect_Split(u8 *rect, void *, u8 *shape, u8 *first, u8 *second);

// 0x659e3e (thiscall (width, height: word pointers, each may be 0), pops 8; name unconfirmed): right - left and bottom - top stored through
// each pointer given.
void __fastcall GuiRect_GetSize(u8 *rect, void *, u16 *width, u16 *height);

// 0x661700 (thiscall (other), pops 4; name unconfirmed): 1 when the four words equal the other's, else 0.
u32 __fastcall GuiRect_Equals(const u8 *rect, void *, const u8 *other);
}
