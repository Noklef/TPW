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

// 0x65d69f (thiscall (a, b), pops 8; no direct callers; name unconfirmed): Window_ImplSlot7 for the object at +0x98.
u32 __fastcall Window_Object98Slot7(u8 *window, void *, u32 a, u32 b);

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

// 0x65da8d (thiscall (enabled: a word), pops 4; slot 5 of the vtable 0x705980; name unconfirmed): flag 2 of +0x48 (disabled) cleared when
// enabled is non-zero, set when it is 0; when that changes it, the handler +0x114 (when set) gets (window, 0x17, enabled sign-extended, 0)
// and the hover is updated (Window_UpdateHover), 1 in AX; unchanged, 0 in AX. Only AX is set.
u32 __fastcall Window_SetEnabled(u8 *window, void *, u32 enabled);

// 0x65bc90 (thiscall; name unconfirmed): the rectangle at +8 copied to the one at +0x2c (GuiRect_Assign); returns +0x2c.
u8 *__fastcall Window_SaveRect(u8 *window);

// 0x65d42e (thiscall (rect, inner: a word), pops 8; name unconfirmed): 0 for no rect; otherwise the rectangle +8 is assigned to it
// (GuiRect_Assign) and, for inner not 0, its slot 3 (GuiRect_Adjust for a GUI rectangle) shrinks it by the insets, the signed words
// +0xf8, +0xfa, +0xfc and +0xfe negated; 1 (AX).
u16 __fastcall Window_GetRect(u8 *window, void *, u8 *rect, u32 inner);

// 0x65cfd4 (thiscall (rect), pops 4; name unconfirmed): 0 for no rect; otherwise it is assigned to the saved rectangle +0x2c
// (GuiRect_Assign); 1 (AX).
u16 __fastcall Window_SetSavedRect(u8 *window, void *, const u8 *rect);

// The rectangles +0x14 and +0x20 (+0x20 is where Window_SetRect keeps the old +8; +0x14's use unknown). 1 in AX for each below (the
// rest of EAX what the call left), or 0 for no pointer where one is checked.
// 0x65cef7 (thiscall (rect), pops 4; name unconfirmed): +0x20 assigned from rect (GuiRect_Assign; no check for none).
u16 __fastcall Window_SetRect20(u8 *window, void *, const u8 *rect);
// 0x65cf17 (thiscall (rect), pops 4; name unconfirmed): +0x14 assigned from rect when given; 1 either way.
u16 __fastcall Window_SetRect14(u8 *window, void *, const u8 *rect);
// 0x65cf3d (thiscall; name unconfirmed): +0x20 assigned from the rectangle +8.
u16 __fastcall Window_KeepRect(u8 *window);
// 0x65d4a0 / 0x65d4cb / 0x65d4f6 (thiscall (rect), pops 4; names unconfirmed): rect (0 for none) assigned from +0x20, +0x14 or the saved
// rectangle +0x2c.
u16 __fastcall Window_GetRect20(u8 *window, void *, u8 *rect);
u16 __fastcall Window_GetRect14(u8 *window, void *, u8 *rect);
u16 __fastcall Window_GetSavedRect(u8 *window, void *, u8 *rect);
// 0x65d8e7 (thiscall (width, height: word pointers, each may be 0), pops 8; name unconfirmed): the size of +0x14 (GuiRect_GetSize).
u16 __fastcall Window_GetSize14(u8 *window, void *, u16 *width, u16 *height);

// 0x65cfff (thiscall (area), pops 4; name unconfirmed): the hit area +0x44 replaced: the old one, unless none or the window's own rectangle +8,
// deleted (slot 4 (1)); then +0x44 = area, or +8 for none. EAX is left the window.
u8 *__fastcall Window_SetHitArea(u8 *window, void *, u8 *area);

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

// 0x65c388 (thiscall (object), pops 4; name unconfirmed): nothing when the object is +0x4c already; else +0x4c = object and the objects
// +0x98 and +0x9c (when set) told by slot 3 (0x10000, an event on the stack whose +0x10 points to a second one on the stack with its byte
// +8 the object's byte +0x10, or 0xff for none; the rest of both unset). EAX not kept.
void __fastcall Window_Set4c(u8 *window, void *, u8 *object);

// 0x65c428 (thiscall (a, b: bytes), pops 8; name unconfirmed): with an object +0x98 or +0x9c set, each set one told by slot 3 (0xc00,
// an event on the stack whose +0x10 points to a second one with the bytes +4 = a and +5 = b; the rest unset). EAX not kept.
void __fastcall Window_NotifyC00(u8 *window, void *, u32 a, u32 b);

// 0x65c4b9 (thiscall (a, b: bytes), pops 8; name unconfirmed): as Window_NotifyC00 with 0x3000 and the bytes at +6 / +7.
void __fastcall Window_Notify3000(u8 *window, void *, u32 a, u32 b);

// 0x65c672 (thiscall (a, b: words), pops 8; name unconfirmed): as Window_NotifyC00 with 0xc000 and the words at +0xa / +0xc.
void __fastcall Window_NotifyC000(u8 *window, void *, u32 a, u32 b);

// 0x65c54a (thiscall (a: a byte), pops 4; name unconfirmed): as Window_NotifyC00 with 0x20000 and the one byte at +9.
void __fastcall Window_Notify20000(u8 *window, void *, u32 a);

// 0x65c5d5 (thiscall (a, b, c, d: bytes), pops 16; name unconfirmed): as Window_NotifyC00 with 0x40000 and the four bytes at +0x10-+0x13.
void __fastcall Window_Notify40000(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);

// 0x65f5b7 (thiscall (a), pops 4; name unconfirmed): the object +0x9c only (when set) gets slot 3 (0x100, event), the event's +0x10
// pointing to a second one whose dword +0 is a (the rest of both unset). EAX not kept.
void __fastcall Window_NotifyImpl100(u8 *window, void *, u32 a);

// 0x65f5fe (thiscall (a), pops 4; name unconfirmed): Window_NotifyImpl100 for the object +0x98.
void __fastcall Window_NotifyObject98_100(u8 *window, void *, u32 a);

// 0x65f645 (thiscall (a), pops 4; name unconfirmed): Window_NotifyImpl100 (a), then Window_NotifyObject98_100 (a); 1 in AX.
u16 __fastcall Window_NotifyBoth100(u8 *window, void *, u32 a);

// 0x65d070 (thiscall (object), pops 4; name unconfirmed): as Window_SetImpl for the object +0x98, except that the old one's own slot 0
// (1) is called when its release (WindowImpl_Release) answers -1, and the event's word +6 is +0xe0 itself and its +0x14 the window + 8.
void __fastcall Window_SetObject98(u8 *window, void *, u8 *object);

// 0x65d1c8 (thiscall (impl), pops 4; name unconfirmed): nothing when impl is +0x9c already. The old one (when set) let go
// (WindowImpl_Release), and when that answers -1 (as a word) +0x98's slot 0 (1) called when +0x98 is set (sic: not the old one's).
// +0x9c = impl; when set: Fn_6617f0 on it (+0x12 = 1), then its slot 3 (flags, an event on the stack: the words +4 = 0, +6 = the
// word +0xe0 + 1, +8 = +0xe2, +0xc / +0xe = +0xe4 / +0xe6, +0x14 = the window + 0x2c and +0x18 = the flags 0x57; with +0x4c set, flags
// | 0x10000 and +0x10 points to a second event on the stack whose byte +8 is +0x4c's byte +0x10) and, when the window is shown
// (Window_IsShown), its slot 5 (1). EAX not kept.
void __fastcall Window_SetImpl(u8 *window, void *, u8 *impl);

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

// 0x65e640 (thiscall (pairs), pops 4; called by 0x665e55; name unconfirmed): Window_DropModal's opposite: 0 in AX when the window has the
// focus [0xfaa5ec]; else it becomes the modal one [0xfaa57c] with the pairs [0xfaa578], 1 in AX. Only AX is set (sic: the rest of EAX
// is the focus's).
u32 __fastcall Window_SetModal(u8 *window, void *, u32 pairs);

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

// 0x658df4 (cdecl (window, message, a, b, delay); name unconfirmed): with the timed table [0xfaa5f8], a window and a message other
// than 1, 2 and 3, the four and the delay added to it (GuiTimers_Add; its result not used) and 1; else 0.
u32 __cdecl Window_PostTimed(u8 *window, u32 message, u32 a, u32 b, u32 delay);

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

// 0x65f538 (thiscall; name unconfirmed): the window taken out of its parent's child list as the destructor does: with a parent (+0x60)
// and no previous sibling (+0x58) the parent's first child +0x5c becomes the next sibling (+0x54); the previous one's next and the next
// one's previous are joined; +0x54, +0x58 and +0x60 cleared; then the hover updated (Window_UpdateHover, its AX returned).
u16 __fastcall Window_Unlink(u8 *window);

// 0x65e526 (thiscall (handler), pops 4; name unconfirmed): a handler not 0 becomes the window's (+0x114); 1 in AX either way.
u16 __fastcall Window_SetHandler(u8 *window, void *, u32 handler);

// 0x65e6d5 (thiscall (index: a word, value), pops 8; name unconfirmed): for an index below 16 (signed: a negative one writes before the
// array, sic) the dword +0xa0 + 4 * index = value and 1 in AX (the rest of EAX value's upper half); else 0.
u32 __fastcall Window_SetDwordA0(u8 *window, void *, u32 index, u32 value);

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

// 0x65e549 / 0x65e56e (thiscall; names unconfirmed; called by the GUI panels' handlers 0x488xxx, 0x4a2xxx, 0x4a7xxx, 0x4bd5xx and the
// windows 0x66bd1e / 0x66bd70, 0x66cf5c / 0x66cf72): the mouse capture [0xfaa5e8]: Capture takes it for the window when nobody has it,
// Release gives it up when the window has it and updates the hover (Window_UpdateHover); 1 in AX when done, else 0. Only AX is set.
u32 __fastcall Window_CaptureMouse(u8 *window);
u32 __fastcall Window_ReleaseMouse(u8 *window);

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

// 0x658bcb (cdecl (delta, x, y: words); name unconfirmed): the mouse wheel turned. 0 in AX while [0x77c488] is 0 (the rest of EAX not
// set). Else the capture [0xfaa5e8], or without one the hovered window [0xfaa5e0], is posted 0x1000d (delta, x << 16 | y; signed, sic as
// Gui_MouseDown's) (Window_PostMessage); 1 in AX.
u32 __cdecl Gui_MouseWheel(u32 delta, u32 x, u32 y);

// 0x658ab4 (cdecl (button, x, y: words); no callers; name unconfirmed, a double click going by the message): 0 in AX while [0x77c488]
// is 0; else the hovered window [0xfaa5e0] is posted 0x10007 (button, x << 16 | y; all three signed, sic as Gui_MouseDown's)
// (Window_PostMessage); 1 in AX. Only AX is set.
u32 __cdecl Gui_MouseDouble(u32 button, u32 x, u32 y);

// 0x658c38 / 0x658c67 / 0x658c96 (cdecl (key: a word, b); called from the window procedure 0x46b752; names unconfirmed, key down, key
// up and a character going by it): 0 in AX while [0x77c488] is 0; else 0x1000a / 0x1000b / 0x1000c (key signed, b) posted with the
// window 1 (Window_PostMessage, which sends those to the first window that takes them); 1 in AX. Only AX is set.
u32 __cdecl Gui_KeyDown(u32 key, u32 b);
u32 __cdecl Gui_KeyUp(u32 key, u32 b);
u32 __cdecl Gui_KeyChar(u32 key, u32 b);

// 0x6588ef (cdecl (x, y: words); name unconfirmed): the mouse moved. 0 in AX while [0x77c488] is 0. Else the words 0xfaa540 / 0xfaa544
// = x / y; without the root window [0xfaa5d8], 0 in AX. With a capture window [0xfaa5e8], it is posted 0x10003 (x << 16 | y, both signed:
// sic, a negative y fills the top half; 0); else the window under the point (Window_HitTest on the root), when it is not the hovered one
// [0xfaa5e0], takes its place as UpdateHover's does (the old one posted 0x10002 (0, 0) when set, the new one 0x10001 (x << 16 | y, 0)),
// and when it is, the hovered one is posted 0x10003 (x << 16 | y, 0) (Window_PostMessage). 1 in AX. Only AX is set.
u32 __cdecl Gui_MouseMove(u32 x, u32 y);

// 0x658840 (cdecl; name unconfirmed; no direct callers): when the mouse words 0xfaa540 / 0xfaa544 differ from the last ones seen
// (0xfaa5cc / 0xfaa5d0, signed compares), those become them and Gui_MouseMove is called with them (their upper halves the original's
// register leftovers: the sign of the old x, and of the new x or, with x unchanged, the old y).
void __cdecl Gui_PollMouseMove();

// 0x65f16b (thiscall (depth: a word), pops 4; name unconfirmed): the depth word +0xe0 = depth, at most 0x100 (signed); the objects +0x98
// and +0x9c (when set) told through their slot 3 (2, an event on the stack whose only word set, +6, is +0xe0 for +0x98 and +0xe0 + 1 for
// +0x9c); then each child (+0x5c, then +0x54) given depth + 2 (from the word passed, not the clamped one). EAX not kept.
void __fastcall Window_SetDepth(u8 *window, void *, u32 depth);

// 0x65f286 (thiscall (value: a word), pops 4; name unconfirmed): the word +0xe2 = value; the objects +0x98 and +0x9c (when set) told through
// their slot 3 (4, an event on the stack whose only word set, +8, is +0xe2); then each child (+0x5c, then +0x54) given the same. EAX not
// kept.
void __fastcall Window_SetWordE2(u8 *window, void *, u32 value);

// 0x65d3a3 (thiscall (value: a word), pops 4; name unconfirmed): the objects +0x98 and +0x9c (when set) told through their slot 3 (1, one
// event on the stack whose only word set, +4, is the value). EAX not kept.
void __fastcall Window_Notify1(u8 *window, void *, u32 value);

// 0x6615a3 (thiscall (value: a word), pops 4; name unconfirmed): a second copy of Window_Notify1: the objects +0x98 and +0x9c (when set)
// told through their slot 3 (1, an event on the stack whose only word set, +4, is the value). EAX not kept.
void __fastcall Window_Notify1b(u8 *window, void *, u32 value);

// 0x661525 (thiscall (value), pops 4; name unconfirmed): the objects +0x98 and +0x9c (when set) told through their slot 3 (0x80, an event
// on the stack whose only dword set, +0, is the value). 1 in AX (the rest of EAX left).
u32 __fastcall Window_Notify80(u8 *window, void *, u32 value);

// 0x66160c (thiscall (value: a word), pops 4; name unconfirmed): the objects +0x98 and +0x9c (when set) told through their slot 3
// (0x80000, an event on the stack whose only word set, +0xa, is the value). EAX not kept.
void __fastcall Window_Notify80000(u8 *window, void *, u32 value);

// 0x65d323 / 0x65d363 (thiscall (value: a word), pops 4; no direct callers; names unconfirmed): Window_Notify1 for only the object +0x98
// / only +0x9c (when set).
void __fastcall Window_NotifyObject98_1(u8 *window, void *, u32 value);
void __fastcall Window_NotifyImpl1(u8 *window, void *, u32 value);

// 0x65f246 (thiscall (value: a word), pops 4; no direct callers; name unconfirmed): the object +0x9c (when set) told through its slot 3
// (2, an event on the stack whose only word set, +6, is the value). EAX not kept.
void __fastcall Window_NotifyImpl2(u8 *window, void *, u32 value);

// 0x65d865 (thiscall (width, height: word pointers, each may be 0; inner: a word), pops 12; name unconfirmed): the size of the rectangle
// +8 (GuiRect_GetSize); with inner not 0 each one set less the margins: width less +0xfc - +0xf8, height less +0xfe - +0xfa. 1 in AX (the
// rest of EAX left).
u32 __fastcall Window_GetSize(u8 *window, void *, s16 *width, s16 *height, u32 inner);

// 0x65dcaf (thiscall (message, button, point), pops 12; name unconfirmed): when the point (x the upper half, signed, y the lower) is in
// the rectangle +0x2c (its slot 0), the window is posted message | 0x1000 (button, the button's press point: the words +2 << 16 | +4,
// each signed, of the entry button of [0xfaa598] (0xc bytes each); sic: a negative +4 fills the upper half) (Window_PostMessage). EAX not
// kept.
void __fastcall Window_PostIfHit(u8 *window, void *, u32 message, u32 button, u32 point);

// 0x65dd18 (thiscall; name unconfirmed): nothing when the window is the active one [0xfaa5dc] already; else that one is posted 0x1b, the
// window becomes the active one and is posted 0x1a (Window_PostMessage). EAX not kept.
void __fastcall Window_Activate(u8 *window);

// 0x65dbf0 (thiscall; name unconfirmed): 0 when the window has no next sibling (+0x54); else it moves to the end of its siblings: after
// the last one, its old previous (+0x58) and next linked up (the next one the parent's (+0x60) first child +0x5c when there was no
// previous), the window posted 0x18 and the old last one 0x19 (Window_PostMessage). Returns the old last one.
u8 *__fastcall Window_RaiseToTop(u8 *window);

// 0x65c707 (thiscall; recursive; name unconfirmed): for a window with bit 0 of +0x48 set: unless one of its top four bits is set, its slot 0
// and then the message 3 sent (Window_SendMessage); then each child (+0x5c, then +0x54) the same. 1 in AX (the rest of EAX left).
u32 __fastcall Window_OpenTree(u8 *window);

// 0x65c778 (thiscall; recursive; name unconfirmed): Window_OpenTree in reverse: for a window with bit 0 of +0x48 set, its children from
// the last one back (by +0x58) first, then (unless one of the top four bits of +0x48 is set) the message 3 sent and its slot 0. 1 in AX.
u32 __fastcall Window_CloseTree(u8 *window);

// 0x65cc04 (thiscall (rect), pops 4; name unconfirmed): nothing for no rectangle or one equal to +8 (GuiRect_Equals). One of the same size
// (GuiRect_GetSize) moves the window (Window_Offset by the left / top differences). Otherwise +8 is kept at +0x20, set from the rectangle
// (GuiRect_Assign), the children laid out again (Window_ResizeChildren with the four edges' moves), the objects +0x98 / +0x9c told
// through slot 3 (0x10, an event on the stack whose only dword set, +0x14, points to +8 for the first and +0x2c for the second) and the
// window sent 0x12 (the new width and height, sign-extended). Both ways end with Window_UpdateHover. 1 in AX.
u32 __fastcall Window_SetRect(u8 *window, void *, u8 *rect);

// 0x65c9bd (thiscall (dl, dt, dr, db: words), pops 16; name unconfirmed): the rectangle +0x2c grown by the four (its slot 3), then each
// child (+0x5c, then +0x54) by its anchors (+0x48, read before its rectangle with the margins, Window_GetRect (1)): with all of
// 0xf0000 each edge moved (when any is not 0) and the rectangle set (Window_SetRect), and nothing more; else with both of 0x30000 the
// left and right moved (when either is not 0) and set, or with both of 0xc0000 the top and bottom, those bits then dropped; then each
// of 0x10000 / 0x20000 / 0x40000 / 0x80000 left set moves it (Window_Offset) by dl / dr across or dt / db down, when that is not 0.
void __fastcall Window_ResizeChildren(u8 *window, void *, u32 dl, u32 dt, u32 dr, u32 db);

// 0x65cf5e (thiscall (left, top, right, bottom: words), pops 16; name unconfirmed): Window_SetRect with a rectangle of those on the stack
// (GuiRect_CtorWith, GuiRect_Dtor). 1 in AX.
u32 __fastcall Window_SetRectTo(u8 *window, void *, u32 left, u32 top, u32 right, u32 bottom);

// 0x65ce10 (thiscall (x, y: word pointers, each may be 0), pops 8; name unconfirmed): the window moved (Window_SetRect) to the left / top
// words given (+0xc / +0xe for none), its width and height (the words of +8, as words) kept. 1 in AX (the rest of EAX left).
u32 __fastcall Window_MoveTo(u8 *window, void *, const s16 *x, const s16 *y);

// 0x65f345 (thiscall (a, b: words), pops 8; recursive; name unconfirmed): the words +0xe4 = a and +0xe6 = b; the objects +0x98 and +0x9c
// (when set) told through their slot 3 (0x40, an event on the stack whose only words set, +0xc / +0xe, are those two); then each child
// (+0x5c, then +0x54) given the same. EAX not kept.
void __fastcall Window_SetWordsE4(u8 *window, void *, u32 a, u32 b);

// 0x65f410 (thiscall (da, db: words), pops 8; no callers; name unconfirmed): Window_SetWordsE4 with +0xe4 + da and +0xe6 + db (signed
// words added as dwords). EAX not kept.
void __fastcall Window_AddWordsE4(u8 *window, void *, u32 da, u32 db);

// 0x65f447 (thiscall (child, depth, words: bytes), pops 12; name unconfirmed): 0 in AX for no child or one with a parent (+0x60), a next
// (+0x54) or a previous (+0x58) already. Else it becomes the window's last child (its parent the window, its previous the old last one
// or 0; the window's first +0x5c when it had none); with depth (signed byte) not 0 it is given the window's depth +0xe0 + 2
// (Window_SetDepth), with words not 0 the window's +0xe4 / +0xe6 (Window_SetWordsE4); the hover updated (Window_UpdateHover) and the
// window's focus let go (Window_KillFocus (0)); 1 in AX (the rest of EAX the focus call's).
u32 __fastcall Window_AddChild(u8 *window, void *, u8 *child, u32 depth, u32 words);

// 0x65e9a4 (thiscall; recursive; name unconfirmed): one step of the window's script (+0xe8, steps of four words; the step +0xec). Only with
// the byte +0x135 1 (signed), a script and the GUI clock (GuiClock_Now) past the last time +0xf4 by more than the interval +0xf0 (signed);
// else 0xffff in AX. The time is set to now and the step's first word says: -9999 the end (+0x135 and +0xec 0, the rectangle +8 less the
// margins +0xf8..+0xfe set (Window_SetRect) and the margins 0); -9998 the word +0xe2 the second word (Window_SetWordE2); -9997 a jump (+0xec
// the second word and the step run again: its answer dropped); -9996 a notification of the second word (Window_Notify1, signed); else the
// four words added to the rectangle +8 (Window_SetRect) and the margins. The step +0xec moves on after all but the end and the jump; it is
// returned in AX (the rest of EAX the rectangle's destructor's).
u32 __fastcall Window_ScriptStep(u8 *window);

// 0x65ee7a (thiscall; name unconfirmed): with the byte +0x136 set, either (the count +0x10c 0) the timer +0x100 stopped (Fn_662097), +0x136
// 0, the rectangle set to the target +0x38 (Window_SetRect) and the window posted 0x1d (0, 0) (Window_PostMessage); or the rectangle +8
// moved by the words +0x104..+0x10a (Window_SetRect) and the count less 1. 0xffff in AX (the rest of EAX the rectangle's destructor's).
u32 __fastcall Window_SlideStep(u8 *window);

// 0x65ecb0 (thiscall (target: a GUI rectangle, time, interval), pops 12; name unconfirmed): a slide to the target: the count +0x10c =
// time / interval (signed IDIV, a word; 0 or an overflow faults), the target kept at +0x38 (GuiRect_Assign), the moves +0x104..+0x10a =
// the target's edges less the window's (+0xc..+0x12, words) / the count (signed IDIV: a count of 0 faults); the timer +0x100 given the
// interval (Fn_6620cc) or, when there is none, made (WindowTimers_Add (window, interval, 0x80085)); +0x136 = 1 and the timer started
// (WindowTimer_Start). 1 in AX (the rest of EAX the start's).
u32 __fastcall Window_SlideTo(u8 *window, void *, const u8 *target, s32 time, s32 interval);

// 0x65c357 (thiscall; name unconfirmed): the manager [0x77c48c]'s slot 12 (no arguments) when its +0x14 is the window; then the script's
// step (Window_ScriptStep), whose answer it returns.
u32 __fastcall Window_ScriptTick(u8 *window);

// 0x65e72d (thiscall (script, interval), pops 8; called by the panels 0x48a85b, 0x48a935, 0x48ad7a and others; name unconfirmed): a
// running script (+0x135 set, signed) first undone (the rectangle +8 less the margins +0xf8..+0xfe set by Window_SetRect, the margins 0);
// then with a script: +0x135 = 1, the step +0xec 0, the script +0xe8, the interval +0xf0 and the last time +0xf4 now (GuiClock_Now); 1 in
// AX. Without one 0 in AX (the rest of EAX left in both, by no caller read).
u16 __fastcall Window_RunScript(u8 *window, void *, u8 *script, s32 interval);

// 0x65e855 (thiscall (paused: a word), pops 4; no direct callers; name unconfirmed): with a script running (+0x135 set) +0x135 = 2 when the
// word is not 0, else 1; 1 in AX. Else 0.
u16 __fastcall Window_PauseScript(u8 *window, void *, u32 paused);

// 0x65e897 (thiscall; name unconfirmed): with a script running (+0x135 set) +0x135, the step +0xec and the script +0xe8 0, then the
// rectangle +8 less the margins set (Window_SetRect) and the margins 0; 1 in AX. Else 0.
u16 __fastcall Window_StopScript(u8 *window);

// 0x65ee1b (thiscall (move: a word), pops 4; name unconfirmed): with a slide running (+0x136 set, signed) its timer +0x100 stopped
// (Fn_662097), +0x136 and the count +0x10c 0 and, when the word is not 0, the rectangle set to the target +0x38 (Window_SetRect); 1 in AX.
// Else 0.
u16 __fastcall Window_StopSlide(u8 *window, void *, u32 move);

// 0x65edd3 (thiscall (paused: a word), pops 4; no direct callers; name unconfirmed): with a slide timer +0x100 it is stopped (Fn_662097)
// when the word is not 0, else started (WindowTimer_Start); 1 in AX. Without one 0.
u16 __fastcall Window_PauseSlide(u8 *window, void *, u32 paused);

// 0x65cd95 (thiscall (dl, dt, dr, db: words), pops 16; called by 0x48dc92, 0x4a12e3; name unconfirmed): the rectangle +8 moved edge by
// edge by the words (GuiRect_CopyCtor, GuiRect_Adjust) and set (Window_SetRect). EAX the rectangle's address (its destructor's), by neither caller read.
void __fastcall Window_AdjustRect(u8 *window, void *, u32 dl, u32 dt, u32 dr, u32 db);

// 0x65f6d1 (cdecl; name unconfirmed): a window's default handler (+0x114 / +0x118 of a new window). Messages with the top bit set: nothing.
// 1 / 2 the tree opened / closed (Window_OpenTree / Window_CloseTree); 3 the window's slot 4; 4 its slot 2 (1: its deleting destructor);
// 5 the window posted 4; 6 shown by a's word (Window_SetShown); 7 moved by a's and b's words (Window_Offset); 8 its rectangle set to the
// edges a >> 16 (signed), a's word, b >> 16 (signed), b's word (Window_SetRectTo); 9 raised (Window_RaiseToTop) and, when that answers a
// window, a is not 0 and [0xfaa5d4] is set, posted 1; 12 its slot 11's answer kept at 0xfaa618, whose address it returns. 0x10 a timer: a
// 0x80084 the script's step, 0x80085 the slide's (Window_ScriptStep / Window_SlideStep); 0x1e Window_ScriptTick. 0x10001 the window's
// cursor (Window_ApplyCursor) and the manager [0x77c48c]'s slot 9 (window); 0x10002 the manager's slot 10; 0x10003 the mouse moved to a
// (x a >> 16 signed, y its word: the manager's slot 8) and, with buttons held (+0x11c), each of the press table [0xfaa598]'s first three
// pressed entries (word +0 1) more than 6 away from its point (+2 / +4) in either axis made 2 (a drag). 0x10004 / 0x10005 a button a
// released / pressed and 0x10006 / 0x10007 a click / double click: first Window_PostIfHit (the message, a, b); a press (after
// Window_Activate) of a free entry (+0 0) sets the button's bit of +0x11c, the entry's point to b and its state 1, and, within the
// double-click time [0x77c480] (signed) of the entry's last time +8 and the window's flags +0x48 without bit 3, posts 0x10007 (a, b)
// and the state 2; then the time now. A release of a pressed entry with the bit set posts, within that time of its last one, 0x10006
// (a, the point: sic, the y signed, so a negative one fills the upper half) and keeps now as the time; else the time 0; the bit and the
// state cleared. Returns 0 but for 12.
u32 __cdecl Window_DefaultProc(u8 *window, u32 message, u32 a, u32 b);
}
