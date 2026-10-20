// The ten message tags' windows (the 0x208-byte records from 0x7c26b0: +0 the window, +4 a word (in use), +6 their text) and the tags
// put up from messages, the one shown in the message box [0x7ca098]: reimplementation of 0x48ea00, 0x48eaa0, 0x48d9c0, 0x48eb20, 0x48ede0
// and 0x48f090. The names are unconfirmed.
//
// A tag's message (on the stack or a save's record): +0 an id, +4 its kind (the same as +0: an update of the tag of that kind), +8 its
// text (wide; 0 none), +0xc its life in ms (0 or less: none), +0x10 a word (signed: the tag's notification), +0x14 / +0x18 dwords, +0x1c
// a word, +0x20 a dword, +0x24 a byte (1: shown at once). A tag window keeps them in its dwords +0xa0 (Window_SetDwordA0): 1 the id, 2
// the text buffer (the record's +6), 3 the word +0x1c, 4 the word +0x10, 5 / 6 the dwords +0x14 / +0x18 (6 0x1b: a tag that stays), 7
// the life, 8 the dword +0x20, 9 a particle system (-1 none), 10 0.
#pragma once

#include "../../../re/types.h"

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
}
