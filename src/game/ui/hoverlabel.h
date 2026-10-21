// The game's hover-lit labels: a child window that shows the shared text part at 0x7c2118 (TextPart6ff530, built at start-up) as its
// object +0x98, drawn yellow and lit white while the mouse is over it, and the owner that binds one. Reimplementation of 0x492d20, 0x485d20,
// 0x48d2e0 and 0x48d290. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492d20 (cdecl (window); 0x485d20, 0x48d2e0 and the dialogs' builders): the bytes 0xea, 0xef, 0x66, 0xff sent as 0x40000
// (Window_Notify40000): the labels' yellow.
void __cdecl HoverLabel_SetYellow(u8 *window);

// 0x485d20 (cdecl (parent, id, text)): the child `id` of the parent (Window_FindChild; nothing without it). Unless it already has an
// object +0x98: its inner rectangle (a GUI rectangle on the stack, GuiRect_Ctor; Window_GetRect (1)) widened by half its width (the
// width's signed half, rounded to 0) on the left and on the right (Fn_659dac; the words only) and set (Window_SetRect), the shared text
// part 0x7c2118's +0x4c = 0 (Set4c_48f690), the part as the child's object +0x98 (Window_SetObject98), font 5 (Window_Set4c; the
// original indexes the font table inline, as Language_Font(5)), the bytes 1, 1 sent as 0xc00 (Window_NotifyC00), the yellow
// (HoverLabel_SetYellow) and the rectangle destroyed (GuiRect_Dtor). Then the text sent as 0x100 to the object +0x98
// (Window_NotifyObject98_100).
void __cdecl HoverLabel_Attach(u8 *parent, u32 id, u32 text);

// 0x48d2e0 (cdecl (window, message, a, b): the handler HoverLabel_Bind sets): 0x10001 (the mouse over it) the bytes 0xff, 0xff, 0xff, 0xff
// sent as 0x40000 (Window_Notify40000); 0x10002 (the mouse gone) the yellow (HoverLabel_SetYellow); 0x10006 with a = 0 (a click) the
// owner (the window's dword +0xa0 0, Window_DwordA0) called through its slot 14. Then the default handler (Window_CallDefault), whose
// result it returns.
s32 __cdecl HoverLabel_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x48d290 (thiscall (id), pops 4; the owner's window at +0x10): the child `id` of the owner's window (Window_FindChild) kept at +0x20,
// its handler HoverLabel_Handler (0x48d2e0; Window_SetHandler), its dword +0xa0 0 the owner (Window_SetDwordA0) and the shared part
// attached with the text 0x78a374 (HoverLabel_Attach).
void __fastcall HoverLabel_Bind(u8 *owner, void *, u32 id);

}
