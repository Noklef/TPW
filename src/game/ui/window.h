// The GUI library's windows (0x65xxxx): a window's children are a list from +0x5c linked by +0x54, each with its id at +0x8c
// (Fn_65d7bc). Reimplementation of 0x65d7d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x65d7d0 (thiscall (id), pops 4; name unconfirmed): the first child of the window whose id is `id`, 0 when none is.
u8 *__fastcall Window_FindChild(u8 *window, void *, u32 id);

// 0x65f697 (thiscall (message, a, b), pops 12; name unconfirmed): the window's handler (the cdecl function at +0x114) called with
// (window, message, a, b) and its result returned; -1 when there is none.
s32 __fastcall Window_CallHandler(u8 *window, void *, u32 message, u32 a, u32 b);

// 0x65f66e (thiscall (message, a, b), pops 12; name unconfirmed): the window's default handler (the cdecl function at +0x118, 0x65f6d1
// unless replaced; not checked for 0) called with (window, message, a, b) and its result returned.
s32 __fastcall Window_CallDefault(u8 *window, void *, u32 message, u32 a, u32 b);

// 0x658d9f (cdecl (window, message, a, b); name unconfirmed): -1 for no window or one whose byte +4 (Fn_659710) is set; otherwise the
// global message hook [0xfaa5fc] (when set) is called with the same four, then Window_CallHandler's result is returned.
s32 __cdecl Window_SendMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x65d6db (thiscall (a, b), pops 8; name unconfirmed): slot 7 (thiscall (a, b)) of the window's object at +0x9c, its result; 0 when there is
// none.
u32 __fastcall Window_ImplSlot7(u8 *window, void *, u32 a, u32 b);

// 0x65d745 (thiscall; name unconfirmed): slot 6 (thiscall, no arguments) of the window's object at +0x9c, its result; the window itself when
// there is none (sic: EAX still holds it).
u32 __fastcall Window_ImplSlot6(u8 *window);

// 0x6616b0 (thiscall; name unconfirmed): a window's object (+0x98) let go: its word count +0x12 decremented, its slot 5 called with
// (0), then the count less (1 - the word +0x10) (both signed) returned; -1 tells the caller to delete it.
s32 __fastcall WindowImpl_Release(u8 *impl);

// 0x65d773 (thiscall; name unconfirmed): 1 in AX when the window and every parent (+0x60) up the chain has flag 0 of +0x48, else 0
// (1 for no window). Only AX is set.
u16 __fastcall Window_IsShown(u8 *window);

// 0x65d90b (thiscall (shown: a word), pops 4; recursive; name unconfirmed): a window told it is shown or hidden: unless it is hidden (flag 0
// of +0x48 clear) and shown is not 0, its object +0x98's slot 5 and then +0x9c's (when set) get shown, the handler +0x114 (when set) gets
// (window, 0x11, shown, 0), then each child (+0x5c, next +0x54 read after its call) in turn. The objects' arguments are pushed as dwords
// whose upper halves are left over (sic): +0x98's from EDX (the caller's, here `edx`, when the window is hidden; else shown's sign), +0x9c's
// from ECX (the window, or what +0x98's slot left in it). Each child is entered with EDX the window (the first) or the previous child.
void __fastcall Window_NotifyShown(u8 *window, void *edx, u32 shown);

// 0x65d9dd (thiscall (shown: a word, any non-zero 1), pops 4; name unconfirmed): nothing when flag 0 of +0x48 already says so; else the flag
// set or cleared, the window told (Window_NotifyShown, EDX the window's upper half and shown) when hiding or when it and its parents are now
// shown (Window_IsShown), the hover updated (Window_UpdateHover), and on hiding the manager [0x77c48c]'s slot 13 (window). Returns 1 in AX,
// the upper half of EAX the manager slot's result when hiding, else 0.
u32 __fastcall Window_SetShown(u8 *window, void *, u32 shown);

// 0x65bc90 (thiscall; name unconfirmed): the rectangle at +8 copied to the one at +0x2c (GuiRect_Assign); returns +0x2c.
u8 *__fastcall Window_SaveRect(u8 *window);

// 0x65d42e (thiscall (rect, inner: a word), pops 8; name unconfirmed): 0 for no rect; otherwise the rectangle +8 is assigned to it
// (GuiRect_Assign) and, for inner not 0, its slot 3 (GuiRect_Adjust for a GUI rectangle) shrinks it by the insets, the signed words
// +0xf8, +0xfa, +0xfc and +0xfe negated; 1 (AX).
u16 __fastcall Window_GetRect(u8 *window, void *, u8 *rect, u32 inner);

// 0x65cfd4 (thiscall (rect), pops 4; name unconfirmed): 0 for no rect; otherwise it is assigned to the saved rectangle +0x2c
// (GuiRect_Assign); 1 (AX).
u16 __fastcall Window_SetSavedRect(u8 *window, void *, const u8 *rect);

// 0x485e30 (cdecl (window, dx, dy); game code, called from 0x493b79, 0x493ba3 and 0x494793; name unconfirmed): the saved rectangle +0x2c
// set (Window_SetSavedRect) to the inner one (Window_GetRect) shrunk by dx on the left and right and dy on the top and bottom
// (GuiRect_Adjust: dx, dy, -dx, -dy as words), through a GUI rectangle on the stack (EAX: its destructor's, the stack address; left out).
void __cdecl Window_SaveInsetRect(u8 *window, s32 dx, s32 dy);

// 0x65c979 (thiscall (rect), pops 4; name unconfirmed): for each parent up the chain (+0x60), the rectangle's slot 2 (GuiRect_Offset)
// called with the parent's words +0xc and +0xe (sic: pushed as dwords whose upper halves are the parent's address for the first and the
// previous one's, the window's at first, for the second). EAX is left as the last parent, or the window when there is none.
u8 *__fastcall Window_OffsetByParents(u8 *window, void *, u8 *rect);

// 0x65c7fe (thiscall (dx, dy: words), pops 8; name unconfirmed): moves the window and its children: 0 when both are 0; otherwise slot 2
// (GuiRect_Offset) of the rectangles +8, +0x44 (unless it is +8) and +0x2c (the original pushes the words as dwords whose upper halves are
// what the registers held; the GUI rectangles read only the words, and these pass 0), the words +0xe4 / +0xe6 moved too, then an event
// on the stack (its words +0xc / +0xe those two, +0x14 the rectangle +8, then +0x2c; the rest unset) to the objects +0x98 and +0x9c when
// set (slot 3 with 0x50), message 0x13 (dx, dy sign-extended; Window_SendMessage), and each child (+0x5c, then +0x54) moved the same way.
// 1 in AX; the upper half of EAX is the last child's result's, or the window's for none.
u32 __fastcall Window_Offset(u8 *window, void *, u32 dx, u32 dy);

// 0x65bcad (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; name unconfirmed): the constructor: five rectangles
// (+8, +0x14, +0x20, +0x2c, +0x38; GuiRect_Ctor), the vtable 0x705980, the flags at +0x48, the parent at +0x60, the id at +0x8c, the
// handlers +0x114 / +0x118 0x65f6d1 and most fields cleared (+0x90 / +0x94 -1, the word +0x50 0xffff). The rectangle +8 is set from the
// four words (offset by the parents when flag 0x400 is set: Window_OffsetByParents), saved to +0x2c (Window_SaveRect) and +0x14, and +0x44
// points to it; [0xfaa5e4] becomes the window. The depth word +0xe0 is the parent's + 2 (1 with none, at most 0x100); +0xe4 / +0xe6 the
// rectangle's centre from the words (sic: the parent's, or left and bottom, are stored first and overwritten). +0xa0 cleared for 0x40 bytes;
// with flag 1 the hover is updated (Window_UpdateHover); then the window is posted 0x15 (0, 0). Returns the window.
u8 *__fastcall Window_Ctor(u8 *window, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x6592b6 (cdecl (window); name unconfirmed): each of the globals [0xfaa5e0] (the hovered window), [0xfaa5dc], [0xfaa5e8] and the
// three from 0xfaa588 that is the window cleared. EAX is left 3 (the loop's count).
u32 __cdecl Window_ForgetGlobals(u8 *window);

// 0x65e672 (thiscall; name unconfirmed): when the window is the modal one [0xfaa57c] (msgqueue's first taker of 0x1000a-0x1000c), it and
// its pairs [0xfaa578] are cleared and 1 is in AX; else 0. Only AX is set (sic: the rest of EAX is [0xfaa57c]'s).
u32 __fastcall Window_DropModal(u8 *window);

// 0x65c2d8 (thiscall; name unconfirmed): the window and its children (from +0x5c by +0x54, each in turn) told they are going: sent 0x14
// (0, 0) (Window_SendMessage), then the byte +4 set; after the children, Window_DropModal, and [0xfaa5ec] and [0xfaa5dc] cleared when
// they are the window. EAX is left as [0xfaa5ec] as it was read.
u32 __fastcall Window_Destroying(u8 *window);

// 0x658fe9 / 0x659012 (cdecl (window); names unconfirmed): the window's messages in the queue [0xfaa5f0] / [0xfaa5f4] cancelled
// (MsgQueue_Cancel), its result; 0xffff without the queue or a window. Only AX is set.
u32 __cdecl Window_CancelPosted(u8 *window);
u32 __cdecl Window_CancelPostedFirst(u8 *window);

// 0x65903b (cdecl (window); name unconfirmed): likewise the window's timed messages in [0xfaa5f8] (GuiTimers_Cancel).
u32 __cdecl Window_CancelTimed(u8 *window);

// 0x659064 (cdecl; name unconfirmed): the timed table [0xfaa5f8] reset (GuiTimers_Reset), then the queues [0xfaa5f4] and [0xfaa5f0]
// cleared (MsgQueue_Clear), each when set.
void __cdecl Gui_ResetQueues();

// 0x65c025 (thiscall; name unconfirmed): the destructor. The vtable 0x705980; Window_Destroying unless the byte +4 is set; the manager
// [0x77c48c] told (slot 13 (window)); each object +0x98 / +0x9c let go (WindowImpl_Release) and, when that answers -1 (as a word),
// deleted (slot 0 (1)) as read again; the timer of each set dword +0x120-+0x12c killed (WindowTimers_Kill, ids 0x80080-0x80083); the
// rectangle +0x44 deleted (slot 4 (1)) when it is not its own +8; Window_ForgetGlobals and the three cancels; unlinked from its parent
// (+0x60, when first: the parent's +0x5c) and siblings (+0x58 / +0x54); each child deleted (slot 2 (1)) until +0x5c is 0 (each child
// unlinks itself); the hover updated; the five rectangles destroyed (GuiRect_Dtor, the last's EAX left).
u32 __fastcall Window_Dtor(u8 *window);

// 0x661680 (thiscall (flags), pops 4; slot 2; name unconfirmed): the scalar deleting destructor: Window_Dtor, then with flag 1 freed
// (Mem_Free). Returns the window.
u8 *__fastcall Window_DeletingDtor(u8 *window, void *, u32 flags);

// 0x65e705 (thiscall (index: a word), pops 4; name unconfirmed): the dword +0xa0 + 4 * index (signed) for an index below 16, else 0.
u32 __fastcall Window_DwordA0(u8 *window, void *, u32 index);

// 0x658ccc (cdecl (window, message, a, b); name unconfirmed): queues the message (MsgQueue_Push, ui/msgqueue). Messages 1-3 go to the
// queue [0xfaa5f4] and 0x1000a-0x1000c to [0xfaa5f0] without any check (sic: no queue faults) and 0 returned; the others to [0xfaa5f0].
// Messages 1-3 and the others need the queue, a window and its byte +4 (Fn_659710) clear: then 1, else 0 (the push's own result is
// dropped).
u32 __cdecl Window_PostMessage(u8 *window, u32 message, u32 a, u32 b);

// 0x65db25 (thiscall (x, y: words), pops 8; name unconfirmed): the deepest window under the point. 0 for a window with flag 2 of +0x48 or
// its byte +4 set, or with neither flag 0 nor any of the top four. Otherwise each child (from +0x5c by +0x54) is searched in turn, the
// last one found kept; then slot 0 (x, y) of the rectangle at +0x44 (GuiRect_Contains) is always asked, and the window itself is the
// answer when that holds and no child was found.
u8 *__fastcall Window_HitTest(u8 *window, void *, u32 x, u32 y);

// 0x6589f9 (cdecl; name unconfirmed): the hover update: 0 in AX without the root window [0xfaa5d8] or with [0xfaa5e8] set. Else the
// window under the mouse (the words 0xfaa540, 0xfaa544; Window_HitTest on the root): when it is not the hovered one [0xfaa5e0], that
// one (when set) is posted 0x10002 (0, 0), the new one becomes it and is posted 0x10001 (x << 16 | y, both signed: sic, a negative y
// fills the top half, 0) (Window_PostMessage). 1 in AX. Only AX is set.
u32 __cdecl Window_UpdateHover();

// 0x65e59b (thiscall (a), pops 4; name unconfirmed): the window takes the focus [0xfaa5ec]: nothing and 0 in AX when it has it already
// (the rest of EAX the focus's); else the old focus (when set) is posted 0x20 (a, 0), the window becomes the focus and is posted 0x1f
// (a, 0) (Window_PostMessage), and 1 in AX (the rest of EAX the post's).
u32 __fastcall Window_SetFocus(u8 *window, void *, u32 a);

// 0x65e5fa (thiscall (a), pops 4; name unconfirmed): when the window has the focus [0xfaa5ec] it is posted 0x20 (a, 0), the focus
// cleared and the hover updated (Window_UpdateHover), 1 in AX (the rest of EAX the update's); otherwise 0 in AX (the rest the focus's).
u32 __fastcall Window_KillFocus(u8 *window, void *, u32 a);

// 0x658af1 (cdecl (button, x, y: words); name unconfirmed): a mouse button pressed. 0 in AX while [0x77c488] is 0 (the rest of EAX not
// set: 0 here). Else the hovered window [0xfaa5e0] is posted 0x10005 (button, x << 16 | y; all three signed: sic, a negative y fills the
// top half) (Window_PostMessage), becomes the button's window (0xfaa588 + 4 * button, signed) and the button's bit (1 << button, the
// shift taken mod 32) is set in the word [0xfaa5bc]; 1 in AX (the rest of EAX the bit's).
u32 __cdecl Gui_MouseDown(u32 button, u32 x, u32 y);

// 0x658b5b (cdecl (button, x, y: words); name unconfirmed): a mouse button released: the same, the button's window posted 0x10004 and
// cleared, its bit cleared in [0xfaa5bc]; 1 in AX (the rest of EAX the inverted bit's).
u32 __cdecl Gui_MouseUp(u32 button, u32 x, u32 y);
}
