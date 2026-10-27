// The ten message tags' windows (the 0x208-byte records from 0x7c26b0: +0 the window, +4 a word (in use), +6 their text) and the tags
// put up from messages, the one shown in the message box [0x7ca098], and their load: reimplementation of 0x48ea00, 0x48eaa0, 0x48d9c0,
// 0x48eb20, 0x48ede0, 0x48f090, 0x48ef70 and 0x481680. The names are unconfirmed.
//
// A tag's message (on the stack or a save's record): +0 an id, +4 its kind (the same as +0: an update of the tag of that kind), +8 its
// text (wide; 0 none), +0xc its life in ms (0 or less: none), +0x10 a word (signed: the tag's notification), +0x14 / +0x18 dwords, +0x1c
// a word, +0x20 a dword, +0x24 a byte (1: shown at once). A tag window keeps them in its dwords +0xa0 (Window_SetDwordA0): 1 the id, 2
// the text buffer (the record's +6), 3 the word +0x1c, 4 the word +0x10, 5 / 6 the dwords +0x14 / +0x18 (6 0x1b: a tag that stays), 7
// the life, 8 the dword +0x20, 9 a particle system (-1 none), 10 0.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x48ea00 (cdecl (id)): a window of 0x138 bytes (GameHeap_Alloc; 0 when that fails) built (Window_Ctor: no parent, flags 0, the id, the
// rectangle (0, 0, 70, 65)), given the object of the GUI window registered as "msgtag" (GuiWindows_FindByName; Window_SetObject98), the
// handler 0x48dae0 (Window_SetHandler) and +0x90 = 0x1db. Returns the window or 0.
u8 *__cdecl MsgTag_NewWindow(u32 id);

// 0x48eaa0 (cdecl): for each of the ten records (i 0-9) the word +4 = 0, the window +0 = MsgTag_NewWindow(900 + i) and, when there is
// one, its dword 2 (Window_SetDwordA0) the address of the record's +6.
void __cdecl MsgTags_Create();

// 0x48d9c0 (cdecl (tag); name unconfirmed): unless the tag is the one shown [0x7ca090]: with its timers on (byte +0x134), for a dword 6
// other than 0x1b its timers 0x80080 / 0x80081 stopped (Window_StopTimer) and the byte cleared, and its particle system (dword 9) ended
// (ParticleSystem_End) when it is above -1, alive (Particle_IsAlive) and the dword 6 not 0x1b; then the message box [0x7ca098] given its
// dwords 2, 3, 5, 6 and 8 as its 1-5 (Window_SetDwordA0), Dialog7cb2b8_Child21Post of the box (a second argument 1 pushed, unread), the
// tag kept at 0x7ca094 and the advisor told 0x11c (Advisor_PostId). Nothing returned that a caller reads.
void __cdecl MsgTag_Open(u8 *tag);

// 0x48eb20 (cdecl (message); name unconfirmed): with the tags' dialog up [0x7cb2ac]: each tag from [0x7cb2c8] (siblings by +0x54,
// Fn_65d56b) whose dwords 1, 3 (as a word) and 8 match the message's +4, +0x1c and +0x20: for a message whose +0 is its +4 the tag's
// text replaced (the CRT's wcsncpy of 0xff characters into its dword 2, the 0x100th set 0; the text not checked for 0, sic) and, when the
// tag is the one shown [0x7ca090], the message box's child 0x10d32 given it (its slot 9); for others the tags' window [0x7cb2c4] sent
// 0x100002 (tag, 1). Unless one was updated, with fewer tags than allowed ([0x7ca09c] < [0x750610], signed) and the tags' window: its
// size asked (Window_GetSize: the width only, inner; unused), the first record from 0x7c26b0 not in use (word +4) taken (set 1) and its
// window notified of the message's word +0x10 (Window_Notify1, sign-extended); with a window it is [0x7cb2c8] when that is 0, gets the
// text (when the message has one, not empty, and the window a buffer), with a life above 0 the timers 0x80080 (the life) and 0x80081
// (the life less 5000) set and started and its byte +0x134 1, the dwords 1-10 (see the header), the tags' window posted 0x100001 (tag,
// 0), the count [0x7ca09c] raised and the interface's sound 0x1e played (Sound_PlayAt). Returns that tag, else 0.
u8 *__cdecl MsgTags_Add(const u8 *message);

// 0x48ede0 (cdecl (message); name unconfirmed): MsgTags_Add, for a message with its byte +0x24 1 the new tag also opened (MsgTag_Open):
// 1 when a tag was added, else 0.
u32 __cdecl MsgTags_Post(const u8 *message);

// 0x48f090 (cdecl; jumped to from 0x481b70; name unconfirmed): a tag for the game's version: the message 0x17e of kind 0x17e, its text
// "Ver 2.0" (the CRT's swprintf of the format at 0x750694 with 2 and 0, into 16 characters on the stack), the rest 0 and the byte +0x24 1,
// added (MsgTags_Add) and, when added, opened (MsgTag_Open).
void __cdecl MsgTags_AddVersion();

// 0x48ef70 (cdecl (stream); name unconfirmed): a count read (Stream_Read of 4 bytes; nothing when 0), then that many 0x21c-byte records
// (as the save 0x48ee50 writes them: +0 the id, +4 the text, +0x204 the life, +0x208 the word +0x10, +0x20c / +0x210 the dwords +0x14 /
// +0x18, +0x214 the word +0x1c, +0x218 the dword +0x20) each read into one buffer on the stack and added (MsgTags_Add) as a message of
// kind 0x17f (whatever the tag's was, sic) with that text; its byte +0x24 stays 0, so the open after the add never happens (sic). The
// reads' results are not checked: a short record keeps what the buffer held. Returns 0 (the count run down).
u32 __cdecl MsgTags_Load(Stream *stream);

// 0x48ee50 (cdecl (stream); called from UI_Save 0x4816a0; name unconfirmed): the count of the ten records [0x7c26b0] whose word +4 is
// set written (Stream_Write of 4 bytes), then for each such record one 0x21c-byte record from a buffer on the stack (the layout
// MsgTags_Load reads): its window's dwords +0xa0 slots 1 (+0), 7 (+0x204), 4 (+0x208, a word), 5 (+0x20c), 6 (+0x210), 3 (+0x214, a
// word) and 8 (+0x218) stored first, then the record's text +6 copied to +4 (wcscpy, unbounded: a 256-character text's end lands on
// +0x204, sic). The bytes +0x20a-0x20b, +0x216-0x217 and those past the text's end are the stack's (sic). Returns Stream_Write's answer
// (its result pointer, on this frame).
s32 *__cdecl MsgTags_Save(Stream *stream);

// 0x481680 (stdcall (stream); the UI module's load, ECX unused): message 5 to the tags' windows (Windows7c26b0_Message5), then MsgTags_Load;
// its 0 returned.
u32 __stdcall UI_Load(Stream *stream);

// 0x4a11e0 (cdecl (window, message, a, b): the tags' window [0x7cb2c4]'s handler, set by 0x4a1fe5; name unconfirmed). Message (signed)
// 0x14: [0x7cb2c8] = 0. 0x100001 (a the new tag): the window's dwords 0 and 1 read (Window_DwordA0); with a tag: it becomes the window's
// last child (Window_AddChild, depth 1), its slot 5 told 1, posted 6 (1, 0) in 500 ms (Window_PostTimed); its inner rectangle and size
// taken (Window_GetRect, Window_GetSize: the width and height land in the low words of the tag's and the window's argument slots), the
// window's top raised by the height + 10 (Window_AdjustRect; the dword taken with the window pointer's upper half, sic), the tag moved
// (Window_Offset) across to the centre of the window's inner rectangle and down to 8 below its old top (the dwords' upper halves as the
// registers held them), the two dwords put back (Window_SetDwordA0) and the window's child 0x2e, when it runs no script (+0x135),
// given the script 0x752e48 at 50 (Window_RunScript). 0x100002 (a the tag, b at once): a shown tag (flag 0 of +0x48) hidden
// (Window_SetShown 0) and sent 5 (1, 0) when b is set (Window_SendMessage), else posted it in 500 ms. 0x100003: each tag from
// [0x7cb2c8] (by +0x54) posted 5 (1, 0) in 100, 200, ... ms. Every message then goes to the default handler (Window_CallDefault),
// whose answer is returned.
s32 __cdecl MsgTags_WindowHandler(u8 *window, u32 message, u8 *tag, u32 b);

// 0x4a1190 (cdecl (window, message, a, b); the handler 0x4a1d70's HUD build gives one of its buttons; name invented): on 0x10006 with a
// 1 the tags' window [0x7cb2c4] posted 0x100003 (Window_PostMessage(0, 0): each tag shown again, see above); every message then goes to
// the default handler (Window_CallDefault), whose answer is returned.
s32 __cdecl MsgTags_ButtonHandler(u8 *window, u32 message, u32 a, u32 b);
}
