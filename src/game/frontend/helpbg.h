// The help background (f_helpbg; vtable 0x6ff598, base 0x705a80): a frame-library window behind the help text. Reimplementation of 0x492320, 0x4922d0
// 0x491fe0, 0x492180, 0x492160, 0x492130, 0x4922e0, 0x492300 and 0x4923d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492320 (thiscall; slot 7 of the help background's vtable 0x6ff598; name unconfirmed): when the word at +4 is set, the object's virtual method 0 (a tail call).
void __fastcall HelpBg_Refresh(u8 *self);

// 0x4922d0 (thiscall; slot 15 of the help background's vtable 0x6ff598): +0x18 = 0 and the word +0xc = 0; returns 0.
u32 __fastcall Obj_Clear18_0c(u8 *object);

// 0x491fe0 (cdecl (window, message, a, b); the handler of the help background's window, installed at 0x4922b0; name unconfirmed): on message 0x14
// the window's object (Window_DwordA0 0, the help background) gets its slot 15 (Obj_Clear18_0c); then Window_CallDefault with the message,
// its result returned.
s32 __cdecl HelpBg_WindowHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x492160 (thiscall (value), pops 4; slot 9 of 0x6ff598; name unconfirmed): only while the word +0xc is set: the base's
// slot 9 (0x66a084, ui/frame), then the object's slot 0.
void __fastcall HelpBg_SetInterval(u8 *self, void *, u32 value);

// 0x492130 (thiscall; slot 1 of 0x6ff598, which the frame's stop calls; name unconfirmed): only while the word +0xc is set: the window +0x18
// (when set) hidden (Window_SetShown(0)), then the word +6 = 0 (Fn_669f94).
void __fastcall HelpBg_Close(u8 *self);

// 0x4922e0 / 0x492300 (thiscall; slots 3 and 4 of 0x6ff598; names unconfirmed): only while the word +0xc is set: the frame started (the word
// +4 = 1, Fn_66a002) or stopped (Frame_Stop, ui/frame), then the window +0x18 shown or hidden (Window_SetShown(1 / 0); not checked for 0).
void __fastcall HelpBg_Show(u8 *self);
void __fastcall HelpBg_Hide(u8 *self);

// 0x492330 (thiscall (x, y), pops 8; called by 0x492530 for the object at 0x7ca0c0; name unconfirmed): with the window +0x18 set, moved
// (Window_Offset) by x and y less its inner rectangle's left and top (Window_GetRect through a GUI rectangle on the stack); a word 0xffff
// for x or y keeps that one. EAX: the rectangle's destructor's (a stack address), or 0 without the window.
u32 __fastcall HelpBg_MoveTo(u8 *self, void *, u32 x, u32 y);

// 0x4923d0 (thiscall; not called, name unconfirmed): with the window +0x18, its rectangle +8 set (Window_SetRect) from its rectangle +0x14
// (Window_GetRect14, through a GuiRect on the stack); EAX the rectangle's destructor's (0 without the window).
u32 __fastcall HelpBg_RestoreRect(u8 *self);

// 0x492530 (cdecl (x, y); called by 0x4bd54f, 0x4bf716, 0x5f1019; name invented): for the object at 0x7ca0c0, both low words 0xffff:
// HelpBg_RestoreRect, else HelpBg_MoveTo(x, y); its EAX returned.
u32 __cdecl HelpBg_Place(u32 x, u32 y);

// 0x4924b0 (cdecl (text); name unconfirmed): the help background at 0x7ca0c0, while its words +0xc and +4 are set, shows a wide text: its
// window +0x18 told Window_NotifyC00(1, 1 when the text has no '\n' (wcschr), else 0); an empty text (wcslen 0) then only calls the
// background's slot 1 (its close); any other shows the window (Window_SetShown(1)) and hands it the text through the window's slot 9.
void __cdecl HelpBg_SetText(const u16 *text);

// 0x492180 (thiscall; slot 14 of 0x6ff598): the help background's window +0x18 made: a child of the GUI root [0xfaa5d8]
// (Window_CreateChild kind 1, flags 1, id 0xfff355e4) from (0x23e, 0x5f0 - h) to (0x5c2, 0x5f0), h = font 7's height (its slot 1, a
// signed word) * 0xc00 / the signed word [0xfaa5c0] + 10; then (the window +0x18 read again for each, not checked) a TextPart_Ctor part
// (none when the allocation fails), the bytes 1, 1 as 0xc00 and as 0x3000, font 7, the bytes 0xff as 0x40000, the text 0
// (Window_NotifyImpl100), the GUI window registered as "f_helpbg" (or 0) as its object +0x98, the dword A0[0] the background, the depth
// 0x26, the window's slot 5 with 0 and the handler 0x491fe0 (HelpBg_WindowHandler); the word +0xc = 1.
void __fastcall HelpBg_Build(u8 *self);
}  // extern "C"
