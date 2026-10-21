// The GUI library's window subclass with the vtable 0x705a50 (built by 0x6688b3 over Window_Ctor: the handlers 0x668f9c, a text row
// (ui/textpool) at +0x148, the dwords +0x13c 500 and +0x140 125, colour-like bytes +0x14c-+0x14f 0xff; what it is is unconfirmed).
// Reimplementation of 0x6689bc, 0x6693a0, 0x66852b, 0x668820, 0x668ad3, 0x668bec, 0x668f35, 0x668f5a, 0x668f7b, 0x668dd9 and 0x668c99. 0x66852b reads a wide text at +0x138 (it may be a
// sibling class's); the constructor and 0x668bec keep a state byte there, +0x139 a byte, +0x13c a limit.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6688b3 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; kind 2 of the factory 0x65dd7b; name unconfirmed):
// Window_Ctor, then the vtable 0x705a50, the bytes +0x138 / +0x139 0, +0x13c 500, +0x140 125, both handlers 0x668f9c, the byte +0x15a 0,
// no text row (+0x148), the bytes +0x14c-+0x14f 0xff, +0x154-+0x156 0, +0x157 0xff, +0x158 / +0x159 1. Returns the window.
u8 *__fastcall Window705a50_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x6689bc (thiscall; name unconfirmed): the destructor: the vtable 0x705a50, the text row +0x148 (when set) released
// (TextPool_Release), then Window_Dtor, whose EAX is left.
u32 __fastcall Window705a50_Dtor(u8 *window);

// 0x66852b (thiscall; name unconfirmed): the length of the wide text +0x138 (the C library's wcslen; its low word in AX), 0 for none.
u16 __fastcall Window705a50_TextLength(const u8 *window);

// 0x6693a0 (thiscall (flags), pops 4; slot 2; name unconfirmed): the scalar deleting destructor: Window705a50_Dtor, then with flag 1
// freed (Mem_Free). Returns the window.
u8 *__fastcall Window705a50_DeletingDtor(u8 *window, void *, u32 flags);

// 0x668820 (thiscall; name unconfirmed): the window's look (AX): 1 with flag 2 of +0x48; else with the byte +0x139 set 3 or 2 by bit 0
// of the state +0x138; else 4 for its bit 1, 5 for its bit 0, 0 for neither.
u16 __fastcall Window705a50_Look(const u8 *window);

// 0x668ad3 (thiscall; name unconfirmed): with an object at +0x9c, its slot 3 called with (code, event): the event (on the stack, only
// partly set) points (+0x10) to a second one whose words +0xa / +0xc and dword +0x10 are, by the look (Window705a50_Look): 4 the byte
// +0x158 + 1, the byte +0x159 (both signed) and +0x14c; 3 or 5 the bytes +0x158 / +0x159 and +0x154; 0 or 2 zero, zero and +0x14c;
// with these the code is 0x4c000, for look 1 it is 0 and nothing is set. The event's dword +0x18 is the code too.
void __fastcall Window705a50_NotifyLook(u8 *window);

// 0x668bec (thiscall (state, notify: bytes), pops 8; name unconfirmed): 0 (AX) when the state +0x138 is `state` already. Otherwise the
// state is set; with `notify` and the dword [0xfaa5d4] set the window is posted 1 (0, 0) (Window_PostMessage); the object +0x98 (when
// set) gets slot 3 (1, event), the event's word +4 the look (Window705a50_Look) and its dword +0x18 1 (the rest unset); then
// Window705a50_NotifyLook, and 1 (AX).
u16 __fastcall Window705a50_SetState(u8 *window, void *, u8 state, u8 notify);

// 0x668a23 (thiscall (enabled: a word), pops 4; slot 5 of the vtable 0x705a50; name unconfirmed): Window_SetEnabled; when that changed
// nothing, 0 (AX). Otherwise the objects +0x98 and +0x9c (each when set) get slot 3 (1, event), the event's word +4 the look
// (Window705a50_Look; the rest unset), and 1 (AX).
u16 __fastcall Window705a50_SetEnabled(u8 *window, void *, u32 enabled);

// 0x668f35 (thiscall (a, b), pops 8; name unconfirmed): +0x13c = a (Fn_6693f0), then +0x140 = b (Fn_6693d0, whose EAX, the window, is
// returned): the two dwords the constructor sets to 500 and 125.
u32 __fastcall Window705a50_SetPair(u8 *window, void *, u32 a, u32 b);

// 0x668f5a / 0x668f7b (thiscall (value), pops 4; names unconfirmed): +0x14c / +0x154 = value (the values Window705a50_NotifyLook hands
// on: +0x14c for the looks 4, 0 and 2, +0x154 for 3 and 5), then Window705a50_NotifyLook.
void __fastcall Window705a50_Set14c(u8 *window, void *, u32 value);
void __fastcall Window705a50_Set154(u8 *window, void *, u32 value);

// 0x668dd9 (thiscall; name unconfirmed): a held press's repeat, with flag 0x20 of +0x48 only: in phase 1 (the byte +0x15a, signed) once
// the GUI clock (GuiClock_Now) is at least the delay +0x13c past the time +0x144 (signed), phase 2 and the time now (asked again), then
// with bit 1 of the state +0x138 the window +0x60 posted 0x10009 (+0x8c, 0) (Window_PostMessage); in phase 2 once the clock is more
// than the interval +0x140 past it, with bit 1 0x10008 (+0x8c, 0) posted, then the time now (asked again).
void __fastcall Window705a50_Repeat(u8 *window);

// 0x668c99 (thiscall (flag, notify: bytes), pops 8; name unconfirmed): 0 (AX) when the byte +0x139 is `flag` already. Otherwise it is set;
// with `notify` and the dword [0xfaa5d4] set the window is posted 1 (0, 0) (Window_PostMessage); the objects +0x98 and +0x9c (each when
// set) get slot 3 (1, event), the event's word +4 the look (Window705a50_Look) and its dword +0x18 1 (the rest unset); then 1 (AX).
// As Window705a50_SetState, without the NotifyLook.
u16 __fastcall Window705a50_SetFlag139(u8 *window, void *, u8 flag, u8 notify);
}
