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
}
