// The game's dialog windows (each a global window pointer) sent their messages: Window_SendMessage wrappers. Reimplementation of 0x4b87f0,
// 0x4b8810 and 0x4b8830 and others (0x4bcec0 chooses the window 0x7cc698; 0x4bd400, 0x4bd450 and 0x49ab80 set their buttons).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4b87f0 / 0x4b8810 / 0x4b8830 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc47c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0); its result returned.
s32 __cdecl Dialog7cc47c_Message4();
s32 __cdecl Dialog7cc47c_Message6On();
s32 __cdecl Dialog7cc47c_Message6Off();

// 0x4b8e80 / 0x4b8ec0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4b4] with message 4 (0, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cc4b4_Message4();
s32 __cdecl Dialog7cc4b4_Message6Off();

// 0x4b8ee0 (cdecl (window, on: a byte); the 0x7cc4b4 dialog's, whose handler calls it with (0, 0) as it goes; name unconfirmed): on: the
// window's inner rectangle (Window_GetRect, through a GUI rectangle on the stack) gives its vertical centre, top + (bottom - top) / 2
// (an arithmetic shift), and unless the particle system [0x7cc4bc] is already set it becomes Particle_Spawn(0x61, 50000, 0, centre *
// 75000 / 1536 (32-bit, wrapping; the quotient toward 0)). Off: the system ended (ParticleSystem_End) and [0x7cc4bc] cleared when set.
void __cdecl Dialog7cc4b4_SetEffect(u8 *window, u32 on);

// 0x4b9c00 / 0x4b9c20 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cc4c4] with message 4 (0, 0) and message 6 (1, 0).
s32 __cdecl Dialog7cc4c4_Message4();
s32 __cdecl Dialog7cc4c4_Message6On();

// 0x4c5780 / 0x4c57a0 / 0x4c57c0 (cdecl; names unconfirmed): Window_SendMessage of the window [0x7cdb0c] with message 4 (0, 0), message 6
// (1, 0) and message 6 (0, 0).
s32 __cdecl Dialog7cdb0c_Message4();
s32 __cdecl Dialog7cdb0c_Message6On();
s32 __cdecl Dialog7cdb0c_Message6Off();

// 0x4c57e0 / 0x4c5800 (cdecl; names unconfirmed): the same with message 6 (1, 0) / (0, 0) to its child 0x16007 (Window_FindChild; none:
// the send's -1).
s32 __cdecl Dialog7cdb0c_Child16007Message6On();
s32 __cdecl Dialog7cdb0c_Child16007Message6Off();

// 0x4c5820 (cdecl (on); called by 0x5dcf90, 0x5dcfaf, 0x5dd5d0; name invented): with the dialog [0x7cdb0c], its child 0x16006
// (Window_FindChild, not checked) given the state on == 1 with notice (Window705a50_SetState(.., 1)). EAX not kept.
void __cdecl Dialog7cdb0c_SetChild16006State(u32 on);

// 0x48d990 (cdecl; name unconfirmed): when the window [0x7ca098] is not 0, Window_SendMessage of it with message 4 (0, 0); then the dword
// 0x7ca090 = 0. Returns the send's result (0 without a window).
s32 __cdecl Dialog7ca098_Message4IfUp();

// 0x4c25e0 (cdecl; name unconfirmed): when the window [0x7cca94] is not 0, Window_SendMessage of it with message 4 (0, 0) (its result; 0
// without a window).
s32 __cdecl Dialog7cca94_Message4IfUp();

// 0x48cf70 / 0x492e60 (thiscall; names unconfirmed): Window_SendMessage with message 4 (0, 0) of the window at +0x10 of the object (0x492e60
// only when it is not 0; 0 then).
s32 __fastcall WindowAt10_Message4(u8 *object);
s32 __fastcall WindowAt10_Message4IfAny(u8 *object);

// 0x492e00 (cdecl (window, message, a, b); the handler of such an object's window, also called from 0x48bb5f and 0x48c0be; name unconfirmed): on
// message 0x14 the object (Window_DwordA0 0) forgets its window (+0x10 = 0); then Window_CallDefault with the message, its result returned.
s32 __cdecl WindowAt10_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x496570 (cdecl (which); called from 0x4962a4, 0x4962bd and 0x4962d6; name unconfirmed): the child 0x32a of the child 0x329 of the
// window [0x7ca168] (Window_FindChild) gets the language text (Language_WideText) 0x6b, 0x6c, 0x6e, 0x6d or 0x6f for which 0-4 through its
// slot 9 (thiscall, one argument), whose result is returned; for any other which (unsigned) nothing is set and which is returned.
u32 __cdecl Dialog7ca168_ShowText(u32 which);

// 0x4964d0 (cdecl (tab); called at 0x4962e0 with 2, 3 or 4; name invented): the child 0x32d of the child 0x329 of the window [0x7ca168]
// (Window_FindChild, unchecked) is given through its slot 7 the average +0x1f8 of the staff of the tab's kind (tabs 0-4: kinds 5, 4, 6, 7,
// 8; any other tab is taken as the kind; Things_AverageStat1f8OfKind), times 1024 / 100 (signed, truncated). GameTable7cfb90_Get1da720_519510
// of [0x7cf83c] is called first, its answer only the ECX of that stdcall. The child's vtable is read before.
void __cdecl Dialog7ca168_ShowStaffAverage(u32 tab);

// 0x4a0a20 (cdecl (window, message, a, b); the handler of the dialog [0x7cb2b8]; name unconfirmed): on message 0x14 [0x7cb2b8] = 0; then
// Window_CallDefault, its result returned (a GUI rectangle built and destroyed on the stack around it, unused, is left out).
s32 __cdecl Dialog7cb2b8_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a0a90 (cdecl (window, message, a, b); a handler installed at 0x4a245b and 0x4a24f4; name unconfirmed): Window_CallDefault.
s32 __cdecl Window_DefaultHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4bcb70 (cdecl (window, message, a, b); the handler a window made at 0x4bc821 gets; name unconfirmed): on message 0x800 the window
// told 0 and -b (Window_NotifyC000) and its slot 9 (thiscall) given 0x7cb4d0; then Window_CallDefault, its result returned.
s32 __cdecl WindowText7cb4d0_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x48c8b0 (thiscall (flags), pops 4; name unconfirmed): the scalar deleting destructor: WindowAt10_Message4IfAny, then the object freed
// (GameHeap_Free) for flags bit 0; returns the object.
u8 *__fastcall WindowAt10_ScalarDtor(u8 *object, void *, u32 flags);

// 0x48cea0 (thiscall (script, handler, first, second, inner), pops 0x14; called by the panels' builders 0x4979b4 to 0x4b79cf; name
// unconfirmed): unless the object has its window +0x10: +0x18, +0x14, +0x1c and +8 = 0, Window7c24c8_Close, the window built on
// [0x7cb2ac] from the script with the handler (WindowScript_Build; nothing more without it) kept in +0x10 and made the [0x7c24c8] one
// (Window7c24c8_Set); the object's slot 0; +0x18 = its child first (Window_FindChild): when there, with inner not 0 +0x1c = that child's
// child inner (not checked) and its slot 5 (0), then +0x18 given the handler 0x48d1a0 (Window_SetHandler) and the object as its dword 6
// +0xa0 (Window_SetDwordA0) and the object's slot 4; +0x14 = the window's child second, and when there the object's slot 3; then the
// object's slot 1 and the object as the window's dword 6.
void __fastcall WindowAt10_Create(u8 *object, void *, u8 *script, u32 handler, u32 first, u32 second, u32 inner);

// 0x493130 (thiscall (flag), pops 4; called at 0x48c855 / 0x48c8a0; name unconfirmed): with the window +0x10 and +0xc not yet set: +0xc
// = 1, the window shown (Window_SetShown(1)), the depth Gui_TopDepth(0, 0) + 1 taken, Window7c24c8_Close for flag 0, then the window
// (+0x10 read again) added to the GUI root [0xfaa5d8] (Window_AddChild(1, 1)), given that depth (Window_SetDepth) and the focus
// (Window_SetFocus(0)). EAX not kept.
void __fastcall WindowAt10_Show(u8 *object, void *, u32 flag);

// 0x4931b0 (thiscall; called directly, no table; name invented): WindowAt10_Show undone: with +0xc set, +0xc = 0, the window +0x10
// hidden (Window_SetShown(0)), unlinked from its parent when there is one (Window_Unlink), its focus let go (Window_KillFocus(0)) and the
// focus restored (Focus_Restore). EAX not kept.
void __fastcall WindowAt10_Hide(u8 *object);

// 0x48d1a0 (cdecl (window, message, a, b); the handler WindowAt10_Create gives its window's child +0x18; name unconfirmed): on message
// 0x10006 the camera centred on the thing [0x7c2658] (Thing_GetHandle, Camera_CentreOnThing; the handle's high word the thing pointer's)
// and the window's dword 6 (Window_DwordA0, the object) told its slot 11; 0. Any other message: Window_CallDefault's result.
s32 __cdecl WindowAt10_CentreHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x48ee20 (cdecl; name unconfirmed): message 5 (0, 0) sent (Window_SendMessage) to the window (+0) of each of the ten entries of 0x208 bytes
// from 0x7c26b0 whose word +4 is not 0.
void __cdecl Windows7c26b0_Message5();

// 0x4bcef0 (cdecl; name unconfirmed): with `same` = (the dword 0x7cc698 == the window [0x7cc8bc]), message 6 (same, 0) to [0x7cc8bc] and
// message 6 (!same, 0) to [0x7cc8c0] (Window_SendMessage; the second's result returned); then 0x7cc698 = 0.
s32 __cdecl Windows7cc8bc_Message6Pair();

// 0x4bce80 (cdecl; called from 0x4e6ad0; name unconfirmed): message 6 (0, 0) to the window [0x7cc8bc] and message 6 (1, 0) to [0x7cc8c0]
// (Window_SendMessage), the byte 0x7cc6a0 = 1, then [0x7cc8c0] (read again) takes the focus (Window_SetFocus(0)), whose result is returned.
u32 __cdecl Windows7cc8bc_SelectSecond();

// 0x4bcf50 (cdecl; name invented): with the byte 0x7cc8b4 set and 0x7cc6a0 not, message 6 (0, 0) to the window [0x7cc8bc] and message 6
// (that byte, 0) to [0x7cc8c0] (Window_SendMessage); then, the byte read again, when set the chain 0xc3123 / 0xc312b / 0xc312c under
// [0x7cc8bc] (Window_FindChild) takes the focus (Window_SetFocus(0)), the mouse modes' holder is made (MouseModes_EnsureHolder) and,
// when the mode [0x7b05d8] answers 9 from its slot 9 (none: 0), [0x7cc8bc]'s cursor is 0x13 (Window_SetCursor); else [0x7cc8c0] takes
// the focus. EAX not kept.
void __cdecl Windows7cc8bc_SelectFirst();

// 0x4bcec0 (cdecl): the window 0x7cc698 (that Windows7cc8bc_Message6Pair compares) = [0x7cc8b8] if set, else [0x7cc8bc] if the byte
// 0x7cc8b4 is set, else [0x7cc8c0]. Returns the value stored in the first two cases, else 0.
u32 __cdecl Global7cc698_Choose();

// 0x4a2590 (cdecl (value); name unconfirmed): with the dialog [0x7cb2b8] and a value not 0, its child 0x21 (Window_FindChild): 0 when that
// child's dword +0xa0 slot 2 (Window_DwordA0) is set; else message 0xb (0, 0) sent to it (Window_SendMessage) and message 0xa (value, 0)
// posted (Window_PostMessage). Returns the value otherwise (no dialog, value 0 or no such child included).
u32 __cdecl Dialog7cb2b8_Child21Post(u32 value);

// 0x4a25f0 (cdecl (flag: a word); name unconfirmed): with the window [0x7cb2ac] and the dialog [0x7cb2b8] set, the dialog's child 0x21
// (Window_FindChild), when there is one: for its byte +0x134 3 or 4 its dword +0xa0 slot 2 cleared when set (Window_DwordA0 /
// Window_SetDwordA0), else message 0xb (0, 0) sent (Window_SendMessage). With the flag, the dialog's child 0x25's child 0x27 (neither
// checked, sic) gets state 0, notified (Window705a50_SetState).
void __cdecl Dialog7cb2b8_ResetChild21(u32 clear27);

// 0x4b4470 (cdecl; name unconfirmed): Dialog7cb2b8_ResetChild21(0).
void __cdecl Dialog7cb2b8_ResetChild21Only();

// 0x4816b0 (thiscall on the object at 0x7b51f0, this not used; name unconfirmed): 0 when the dword [0x7cc2f0] (Global7cc2f0_Get_4b4480) is
// 0; else Dialog7cb2b8_ResetChild21Only and 1.
u32 __fastcall Obj7b51f0_ResetDialogChild21(void *self);

// 0x4bd400 / 0x4bd450 (cdecl (value: a byte); names unconfirmed): with the window [0x7cc8bc] set, its child 0xc3123's child 0xc3124's child
// 0xc3128 / 0xc3129 (Window_FindChild three times, none checked): 0x4bd400 sets that 0x705a50 window's state to value == 1, notifying
// (Window705a50_SetState(.., 1)); 0x4bd450 calls its slot 5 (thiscall) with value == 0.
void __cdecl Window7cc8bc_SetC3128(u8 value);
void __cdecl Window7cc8bc_SetC3129(u8 value);

// 0x49ab80 (cdecl (mode); name unconfirmed): the window [0x7ca2e8]'s child 0x12272 (not checked), then the states of its 0x705a50 children
// set to 0, notifying (Window705a50_SetState(0, 1)): with mode 3 the children 0x12277 and 0x1227b, else 0x12279, 0x12278, 0x1227c, 0x1227d,
// 0x1227a and 0x1227e, in that order.
void __cdecl Window7ca2e8_ClearButtons(u32 mode);

// 0x4b9a70 (cdecl; called by 0x5dd650 / 0x5dd910 after 0x4c4fb0; name unconfirmed): unless the dialog [0x7cc4c4] is made, it is built
// from the window script 0x7581a0 (WindowScript_Build: the last child it makes) on the window +4 of the frame [0xf82884] (not checked)
// with the handler 0x4b9950; its children 0x16c11 and 0x16c12 (Window_FindChild, not checked) each get a TextPart_Ctor part (0x1a4 bytes,
// GameHeap_Alloc; none when that fails; Window_SetImpl), the bytes 1, 1 as 0xc00 and 0x3000, the font 5 / 8 (Language_Font), the bytes
// 0xff and the text 0; then [0x7cc4e4] = 0.
void __cdecl Dialog7cc4c4_Create();

// 0x4b9950 (cdecl (window, message, a, b); name invented): Dialog7cc4c4_Create's handler: the message 0x14 forgets the dialog ([0x7cc4c4]
// = 0); Window_CallDefault's result.
s32 __cdecl Dialog7cc4c4_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4c4fb0 (cdecl (flag: a byte); called by 0x5dd650 / 0x5dd910 before Dialog7cc4c4_Create; name unconfirmed): unless the dialog [0x7cdb0c]
// is made or the frame [0xf82884] has no window +4, it is built from the window script 0x7596b0 on that window (WindowScript_Build: the
// last child it makes) with the handler 0x4c53f0 and its dword +0xa0 0 set to the flag (Window_SetDwordA0). With a flag its child 0x16007
// is hidden (Window_SetShown 0) and its children 0x16003 and 0x16000 told slot 5 (0). Without one the child 0x16007 is told slot 5 (1) and
// its own children 0x16008-0x1600c each get a 0x6ff350 text part (0x1a4 bytes, GameHeap_Alloc; TextPart6ff350_Ctor(0), none when that
// fails; Window_SetImpl), font 6 (Language_Font; Window_Set4c), slot 7 (0), the bytes 1, 1 as 0xc00 (Window_NotifyC00), slot 7 (0) again,
// the handler 0x4c56e0 (Window_SetHandler), the dword +0xa0 0 set to 1-5 in turn (Window_SetDwordA0) and slot 5 (1); then [0x7cdb10] = 1.
// Either way the children 0x16002, 0x16004, 0x16005 and 0x16001 get the pair 0, 10 (Window705a50_SetPair). No child is checked.
void __cdecl Dialog7cdb0c_Create(u32 flag);
}
