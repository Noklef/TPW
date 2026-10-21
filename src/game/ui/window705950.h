// The GUI library's window subclass with the vtable 0x705950 (kind 4 of the factory 0x65dd7b: a GUI rectangle +0x138 inset from the
// saved one, which leaves 0x30 on the right for a scroll bar (the factory's kind 3) made with flag 0x1000; the handler 0x65bb6b; what it
// is is unconfirmed, a list going by the scroll bar). Reimplementation of 0x65aac0 and 0x65ab64. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65aac0 (thiscall; name unconfirmed): the saved rectangle +0x2c set from +8 (GuiRect_Assign) less 0x30 on the right (the word +0x34),
// and the rectangle +0x138 that one inset by 4 on each side (words).
void __fastcall Window705950_Layout(u8 *window);

// 0x65ab64 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, the rectangle +0x138
// (GuiRect_Ctor), the vtable 0x705950, +0x148 -1, +0x144, +0x14c and +0x150 0, the words +0x154 / +0x156 0; Window705950_Layout; with
// flags & 0x1000 a child of the factory's kind 3 (flags 0x90, and 0x40 with flags & 0x40; id 1) from +0x34 + 4 / +0xe to +0x10 / +0x12
// that, when made, gets slot 5 (0), Window_SetShown(1) and Window705b20_SetRange(0, 0); then the handler 0x65bb6b (Window_SetHandler).
// Returns the window.
u8 *__fastcall Window705950_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x65bb10 (thiscall (left, top, right, bottom: words), pops 16; name unconfirmed): the rectangle +0x138's four words set (Fn_659dac), whose
// EAX is returned.
u32 __fastcall Window705950_SetInner(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom);

// 0x65bb3f (thiscall (dx, dy: words), pops 8; name unconfirmed): the rectangle +0x138 moved through its slot 2 (GuiRect_Offset; the words pushed
// as dwords whose upper halves are what the registers held).
void __fastcall Window705950_OffsetInner(u8 *window, void *, u32 dx, u32 dy);

// The list's rows: the selection +0x148 (-1 for none), the row count +0x14c, the first row shown +0x150, the row height +0x154 and the
// rows shown +0x156 (words), the rows from the inner rectangle's top (+0x13e).
// 0x65b93f (thiscall (x, y: words), pops 8; name unconfirmed): nothing with a row height of 0; else the row at y ((y - top) / height,
// signed, plus the first row shown when below both the rows shown and the count; else -1) becomes the selection when it is not already:
// the parent (+0x60) posted 0x201 (id +0x8c, row) when it is a row, and with [0xfaa5d4] set the window posted 3 (Window_PostMessage).
void __fastcall Window705950_SelectAt(u8 *window, void *, u32 x, u32 y);

// 0x65bac2 (thiscall (x, y: words), pops 8; name unconfirmed): with flag 0x80 of +0x48 and the point in the rectangle +0x2c (its slot 0),
// Window705950_SelectAt (x, y).
void __fastcall Window705950_TrackAt(u8 *window, void *, u32 x, u32 y);

// 0x65ba83 (thiscall; name unconfirmed): with a selection (not negative), the parent (+0x60) posted 0x200 (id +0x8c, the selection).
void __fastcall Window705950_PostChosen(u8 *window);

// 0x65ba1e (thiscall (x, y: words), pops 8; name unconfirmed): with flag 0x80 of +0x48, Window705950_PostChosen; without it,
// Window705950_SelectAt (x, y).
void __fastcall Window705950_ClickAt(u8 *window, void *, u32 x, u32 y);

// 0x65b8dc (thiscall; name unconfirmed): the child with id 1 (Window_FindChild; the scroll bar) asked its slot 8; when that differs from
// the first row shown +0x150 it becomes it, and with [0xfaa5d4] set the window is posted 3 (Window_PostMessage).
void __fastcall Window705950_Scrolled(u8 *window);

// 0x65bb6b (cdecl (window, message, a, b); name unconfirmed): the list's handler. 0x10003 the mouse at a (x a >> 16 signed, y its word:
// Window705950_TrackAt), 0x13 the inner rectangle moved by a's and b's words (Window705950_OffsetInner), 0x11006 a click at b
// (Window705950_ClickAt), 0x11007 the chosen row posted (Window705950_PostChosen), 0x800 the scroll bar followed (Window705950_Scrolled);
// then every message (those too) goes on to the default handler (Window_DefaultProc), whose answer it returns.
u32 __cdecl Window705950_Proc(u8 *window, u32 message, u32 a, u32 b);

// 0x65ada4 (thiscall (n), pops 4; name unconfirmed): 0 unless n (signed) is below the count +0x14c and there is a first node +0x144;
// else the n-th node from that one in a depth-first walk (a node's first child +0x48, else the next +0x44 of it or of its nearest
// parent (+0x4c) with one); negative n the first.
u8 *__fastcall Window705950_NthNode(u8 *window, void *, s32 n);
}
