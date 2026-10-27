// The GUI library's window subclass with the vtable 0x705a20 (kind 5 of the factory 0x65dd7b: with flag 0x40 a 0x200-byte buffer +0x138,
// a window timer +0x164 (0x2ee, id 0x75e88), the handler 0x66857d; what it is is unconfirmed, a text field going by the buffer and the
// timer). Reimplementation of 0x665fb0, 0x665fcd, 0x66656c and others. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x665fb0 (thiscall; name unconfirmed): the saved rectangle +0x2c set from +8 (GuiRect_Assign).
void __fastcall Window705a20_SaveRect(u8 *window);

// 0x665fcd (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): Window_Ctor, then the vtable
// 0x705a20 and +0x13c 0; with flags & 0x40 +0x138 a new 0x200-byte block (Mem_Alloc) whose first word is 0 and +0x13c 0xff (none got:
// +0x138 0), without it +0x138 and +0x13c 0; Window705a20_SaveRect; +0x164 a window timer (WindowTimers_Add(window, 0x2ee, 0x75e88));
// the words +0x14a and +0x14c 0xffff, +0x148 and +0x14e-+0x158 0, the dwords +0x140, +0x144 and +0x15c 0, the byte +0x161 0; the
// handler 0x66857d (Window_SetHandler) and +0x118 too, Window_SetFocus(0); the byte +0x160 1 and +0x162 whether the byte 0xfaa56c is set.
// Returns the window.
u8 *__fastcall Window705a20_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x6661bc (thiscall; name unconfirmed): the destructor: the vtable 0x705a20; the buffer +0x138 freed (Mem_Free) when set and the flags
// +0x48 have 0x40; with the timer +0x164 set, the window's timer 0x75e88 killed (WindowTimers_Kill); the caret +0x15c let go
// (WindowImpl_Release), and when that answers -1 (as a word) its slot 0 (1) called; then Window_Dtor, whose EAX is left.
u32 __fastcall Window705a20_Dtor(u8 *window);

// 0x66656c (thiscall (caret: an object), pops 4; name unconfirmed): nothing when it is +0x15c already. The old one (when set) let go
// (WindowImpl_Release), and when that answers -1 (as a word) its slot 0 (1) called. +0x15c = caret; when set: Fn_6617f0 on it, then its
// slot 3 (0x17, an event on the stack: the words +4 = 0, +6 = the depth word +0xe0 + 3, +8 = +0xe2, +0x14 a GUI rectangle (GuiRect_CtorWith)
// from +0x30 + +0x150 / +0x32 + +0x154, 8 wide and +0x156 high, and +0x24 0x17) and, when the window is shown (Window_IsShown), its
// slot 5 (1). EAX not kept.
void __fastcall Window705a20_SetCaret(u8 *window, void *, u8 *caret);

// 0x66836a (thiscall (two words, unread), pops 8; name unconfirmed): message 0x13's (moved) handler: with a caret +0x15c, its slot 3
// (0x10, an event on the stack of which only +0x14 is set: a GUI rectangle (GuiRect_CtorWith) from +0x30 + +0x150 / +0x32 + +0x154,
// 8 wide and +0x156 high). EAX not kept.
void __fastcall Window705a20_MoveCaret(u8 *window, void *, u32, u32);

// 0x667fee (thiscall (key: a word, value), pops 8; name unconfirmed): the parent +0x60 sent (Window_SendMessage) for key 9 0x803, 0xd
// 0x802 (none when flag 0x80 of +0x48 is set), 0x1b 0x804, each with the id +0x8c and the value; any other key 0x806 with the id and
// the key sign-extended. EAX: the send's, or (sic) the window for a key 0xd not sent.
u32 __fastcall Window705a20_KeyToParent(u8 *window, void *, u32 key, u32 value);

// 0x6667eb (thiscall; name unconfirmed): with no selection (the word +0x14c -1) and the word +0x158 not -1, that word's bit 0 flipped
// (insert or overwrite, unconfirmed), the caret +0x15c (when set) told it through its slot 5 (the word, the upper half the window's
// address's, sic), and with [0xfaa5d4] set the window posted 3 (0, 0). EAX not kept.
void __fastcall Window705a20_ToggleInsert(u8 *window);

// 0x66671d (thiscall (post), pops 4; name unconfirmed): the object +0x9c (when set) told 0x4100 through its slot 3, the event's +0x10
// pointing to the text (+0 the buffer +0x138, the word +0xa the length +0x148); with post set the parent +0x60 posted 0x801 (the id
// +0x8c, 0). EAX not kept.
void __fastcall Window705a20_NotifyText(u8 *window, void *, u32 post);

// 0x66679e (thiscall; called by 0x667272, 0x6672e4 after they move the scroll word +0x14e; name unconfirmed): the object +0x9c (when
// set) told 0x8000 through its slot 3, the event's +0x10 pointing to a second one on the stack whose only word set, +0xc, is +0x14e.
// EAX not kept.
void __fastcall Window705a20_NotifyScroll(u8 *window);

// 0x666875 (thiscall (mode: a word), pops 4; name unconfirmed): the word +0x158 the mode, -1 for 0; the caret +0x15c (when set) told the
// mode through its slot 5 (the pushed dword's upper half EDX's: the caller's for mode 0, else the window's address's; sic), and with
// [0xfaa5d4] set the window posted 3 (0, 0). EAX not kept.
void __fastcall Window705a20_SetInsertMode(u8 *window, void *edx, u32 mode);

// 0x6662b7 (thiscall (buffer, size), pops 8; name unconfirmed): a buffer of the caller's: unless the window has its own (flag 0x40 of
// +0x48) or either is 0, +0x138 = buffer, +0x13c = size and the byte +0x160 1; AX 1 then, else 0 (EAX's upper half the window's).
u32 __fastcall Window705a20_SetBuffer(u8 *window, void *, u32 buffer, u32 size);

// 0x668447 (thiscall; name unconfirmed): the selection's start: the anchor +0x14c when set (not -1) and below the cursor +0x14a, else
// the cursor (the words compared signed). AX it (EAX's upper half: the window's for the anchor, else the anchor's sign, sic).
u32 __fastcall Window705a20_SelectionStart(u8 *window);

// 0x66848f (thiscall (from, to: word pointers, each optional), pops 8; name unconfirmed): from = the selection's start as
// Window705a20_SelectionStart gives it, to = the anchor +0x14c when above the cursor +0x14a, else the cursor (signed). EAX not kept.
void __fastcall Window705a20_Selection(u8 *window, void *, u16 *from, u16 *to);

// 0x6668e9 (thiscall (cursor, mark, anchor: words), pops 12; name unconfirmed): moves the caret and the selection. Nothing without a text
// +0x138 or a font +0x4c; the line height +0x156 worked out when 0 (the font's slot 1 * 0x600 / the word [0xfaa5c0]). The cursor: -1 kept,
// -2 / -3 one back (not below 0) / on, -4 the start, -5 the end (the text's length), -6 / -7 (only with flag 0x80 of +0x48, several lines)
// the character on the line above / below nearest the caret's x +0x150 (the object +0x9c's slot 4 (1, index) gives each one's position:
// x +4, y +6; nothing when none is found; the cursor kept without the object), any other the word itself. The anchor likewise (-1 none,
// -2 / -3 from the anchor or else the cursor). Both at most the length, the anchor none when it is the cursor; nothing when the byte +0x162
// is set and the cursor is not at the end, when neither changes, or the cursor is past the text. Then: one line: the scroll +0x148 brought
// to the anchor's (or the cursor's) x (Window705a20_NotifyText 0); several: the scroll +0x14e to the caret's line (slot 4 (2, text): the
// layout's height +0xa set; Window705a20_NotifyScroll, NotifyText 0). The caret's left / right / top +0x150 / +0x152 / +0x154 (each in
// the window) when changed: +0x158 1 for a mark, [0xfaa5d4]'s post 3 (Window_PostMessage), and the caret +0x15c (when set) told: slot 5
// (1) unless +0x158 is -1 (then 1), and 0x10 with the rectangle (Fn_659dac) through slot 3. Sic: the x or the bottom left unset when the
// object answers none (one line: the cursor's entry; several: the layout).
void __fastcall Window705a20_SetSelection(u8 *window, void *, u32 cursor, u32 mark, u32 anchor);
}
