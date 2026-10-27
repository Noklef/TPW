// The input method editor support (vtable 0x6fef04; "Microsoft IME 98" is singled out at
// 0x478fc9): reimplementation of its constructor 0x478b60. Four vectors (a one-byte allocator,
// then first / last / end) at +8, +0x18, +0x28 and +0x38 (the last of 0x24-byte entries), +0x48
// a flag, +0x4c / +0x50, +0x54 (-1 when none) and +0x5c a handle the destructor closes.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x478b60: the vectors empty, the rest cleared (+0x54 = -1); returns ime. The allocator bytes
// take an uninitialised local, which is the top byte of ime as pushed (sic).
u8 *__fastcall Ime_Ctor(u8 *ime);

// Its u32 vectors' helpers (thiscall, the vector unused), constructing in place (a null slot
// skipped, still counted). 0x47c910: [first, last) to dest; returns dest's end.
u32 *__fastcall ImeVector_Copy(void *vec, void *, const u32 *first, const u32 *last, u32 *dest);
// 0x47c940: n copies of *value at dest (*value read for each).
void __fastcall ImeVector_Fill(void *vec, void *, u32 *dest, u32 n, const u32 *value);
// 0x47db00 (cdecl): *slot = *value unless slot is null.
void __cdecl ImeVector_Construct(u32 *slot, const u32 *value);
// 0x47c720 / 0x47c700: a u32 vector's size (+8) / capacity (+0xc) from its first (+4); 0 when
// that is null. 0x47ae40 / 0x47b300: the same for 16-byte entries.
u32 __fastcall ImeVector_Size(const u8 *vec);
u32 __fastcall ImeVector_Capacity(const u8 *vec);
u32 __fastcall ImeVector16_Size(const u8 *vec);
u32 __fastcall ImeVector16_Capacity(const u8 *vec);

// 0x47a960: the current entry (+0x54) of the 0x24-byte list (first at +0x3c), or null for -1.
u8 *__fastcall Ime_Current(const u8 *ime);
// 0x47a9b0: the current entry cached at +0x160 (a 0x24-byte entry: its dwords, the strings at +0x170 and +0x180): with none, +0x188
// and byte +0x184 cleared; else unless cached (byte +0x184) with the same first dword it is assigned over the cache
// (CountedVector14_Assign, core/buffers, inlined), +0x188 set to +0x180 when 0, and marked cached. Returns the cached strings' count
// (0 when none is cached).
u32 __fastcall Ime_CacheCurrent(u8 *ime);
// 0x47a980: +0x58 = limit, at most the constant at 0x6feee0 (12).
void __fastcall Ime_SetLimit(u8 *ime, void *, u32 limit);
// 0x480e70: the constructor of the global at 0x7b4e38, which holds an IME at +0x1c: that
// constructed, +0x17c, +0x190-+0x198, +0 and +8 zeroed, byte +0xc too, +0x10 = -1, and byte
// +0x18c the top byte of host (an unset local, sic). Returns host.
u8 *__fastcall ImeHost_Ctor(u8 *host);

// 0x480f90 (thiscall on the host; window, pops 4; name unconfirmed): only for the host's own window (+4): the words +0x1a8 = 0 and +0x1aa = 1,
// and the IME's limit (Ime_SetLimit on +0x1c) = what the text window has room for: its limit word +0x13c (signed) less its text's length
// (Window705a50_TextLength, ui/window705a50; signed word).
void __fastcall ImeHost_OnWindow(u8 *host, void *, u8 *window);

// 0x47c740 (thiscall; position, pointer to the value, pops 8): inserts one value before the position. With room it is built at the end when the position is the end
// (ImeVector_Copy / ImeVector_Fill), else the last is copied up to the end, the rest shifted up and the value assigned over the position; with none the vector is rebuilt in
// a new block of twice the size (one when it was empty), the old block freed. Returns where the value went (first + the position's index).
u32 *__fastcall ImeVector_InsertOne(u8 *vec, void *, u32 *position, const u32 *value);

// 0x47cf20 (thiscall; position, count, pointer to the value, pops 12): inserts count copies. With room: when fewer elements follow the position than count they are copied
// up past the end, the rest of the new room filled from the value and the position's span assigned; otherwise the last count are copied up, the rest shifted and the count
// values assigned. Without: a new block of the larger of count and the size plus the size. Nothing for a count of 0 (with room).
void __fastcall ImeVector_InsertN(u8 *vec, void *, u32 *position, u32 count, const u32 *value);

// 0x479260 (thiscall; window, character, lparam, pops 12; name unconfirmed): 0 unless the character (the whole dword passed to
// IsDBCSLeadByte, 0x6fd154) is a lead byte. Then up to 10 PeekMessageA (0x6fd354; the window's WM_CHAR, removed) for the trail byte: 1
// when none comes. The two bytes (lead, then the message's wParam low byte) to one wide character (MultiByteToWideChar 0x6fd174, code
// page 0): 1 unless exactly one. The character (its low word) sent as message 0x410 with lparam to the window through the IME's handler
// +0x50 (stdcall) or, without one, DefWindowProcA (0x6fd340); 1.
u32 __fastcall Ime_OnChar(u8 *ime, void *, u32 window, u32 character, u32 lparam);

// A window's input context: { window, context }. 0x4791a0 (thiscall; window, pops 4; name unconfirmed): window stored, then the context
// from ImmGetContext (0x6fd084, through its thunk 0x6b55a6); returns context. 0x478d20 (thiscall; name unconfirmed): with a context,
// ImmReleaseContext (0x6fd080, thunk 0x6b55a0) of window and context; returns what EAX holds (the release's answer, else 0).
u32 *__fastcall ImeContext_Get(u32 *context, void *, u32 window);
u32 __fastcall ImeContext_Release(const u32 *context);

// 0x478be0 (thiscall; the destructor): the vtable back to 0x6fef04, the context +0x5c (when set) destroyed (ImmDestroyContext 0x6fd07c,
// thunk 0x6b559a); each 0x24-byte entry of the vector +0x38 (first +0x3c up to last +0x40, read once) has its strings' vector +0x10
// cleared (CountedVector4_Clear), the block freed (null too) and +0x3c-+0x44 cleared; then the wide strings +0x28, +0x18 and +0x8 tidied
// (inlined: released or their count lowered, emptied). 0x478bc0 (thiscall; flags, pops 4): that, then the IME freed for flags bit 0;
// returns ime.
void __fastcall Ime_Dtor(u8 *ime);
u8 *__fastcall Ime_DeletingDtor(u8 *ime, void *, u32 flags);

// 0x480960 (thiscall; the host's destructor, name unconfirmed): the wide strings of the vector +0x190 (first +0x190 up to last +0x194, read
// once; 0x10 bytes each) reset (CountedBuf_Reset, releasing), its block freed (null too) and +0x190-+0x198 cleared; then the IME +0x1c
// destroyed (Ime_Dtor).
void __fastcall ImeHost_Dtor(u8 *host);

// 0x47a830 (thiscall (value), pops 4; name unconfirmed): the window +0x4c's input context (ImmGetContext 0x6fd084); with one, ImmNotifyIME
// (0x6fd068) 0x12 (NI_SELECTCANDIDATESTR) with the index +0x54 and value, then 0x11 (NI_CLOSECANDIDATE) with it and 0, and the context
// released (ImmReleaseContext 0x6fd080). EAX not kept.
void __fastcall Ime_SelectCandidate(u8 *ime, void *, u32 value);

// 0x47a6d0 (thiscall (first, last, a third unread), pops 12; name unconfirmed): with the window +0x4c's input context: ImmNotifyIME 0x17
// (NI_SETCANDIDATE_PAGESIZE) with the index +0x54 and last - first, then 0x16 (PAGESTART) with first; the keyboard layout's
// (GetKeyboardLayout(0) 0x6fd304) description (ImmGetDescriptionA 0x6fd088, 0x100 bytes) widened (MultiByteToWideChar 0x6fd174, code page
// 0) into the wide string +0x28: when it has a length, the string assigned that many less one 'X's (a local string: CountedBuf_Reset,
// StdWString_AssignFill, StdWString_AssignSub of all of it (npos [0x6fef00]), released), frozen (StdWString_Freeze) and written over;
// then compared with the wide text 0x6feee8 (wcscmp; the answer unused, sic) and the context released. EAX not kept.
void __fastcall Ime_SetCandidatePage(u8 *ime, void *, u32 first, u32 last, u32);

// 0x480fd0 (thiscall on the host; name unconfirmed): the IME's result (+0x1c's string +8: Obj_GetCOr6fd6c4) put into the text window +4
// (Window705a20) at its selection's start. When the host's text +0x1a8 is still unfetched (its first word 0 and the second 1, as
// ImeHost_OnWindow leaves it): the window's text (Window705a20_GetText, 0x100) into it, the IME's limit (Ime_SetLimit) the room left (the
// window's word +0x13c less its text's length, signed), and with no room and a result OutputDebugStringA (0x6fd1e0) of the text 0x74fa8c
// and nothing more. Else the text cut at the start (Window705a20_SelectionStart), the result put between (wcscpy / wcscat into two
// 0x102-character locals), set (Window705a20_SetTextQuiet) and selected from the start over the result's length (Window705a20_SetCursor).
// EAX not kept.
void __fastcall ImeHost_InsertResult(u8 *host);

// 0x4809f0 (thiscall on the host (window, message, a, b), pops 16; name unconfirmed): the candidate window's handler. 0x14: +0 and +8
// cleared. 0x400: Ime_SelectCandidate(b) on the IME +0x1c, its byte +0x184 0. 0x405: +0x14 = a, +0x18 = b, and unless the IME's byte +0x184
// is set with +0x168 a and +0x16c the page +0x10 (as read before), Ime_SetCandidatePage(a, b, page). 0x401 with b not +0x10: +0x10 = b,
// Ime_SetCandidatePage(+0x14, +0x18, b) likewise unless the IME has them, then ImeHost_InsertResult. Then (all) Window_CallDefault: its
// EAX.
s32 __fastcall ImeHost_OnMessage(u8 *host, void *, u8 *window, u32 message, u32 a, u32 b);

// 0x480d10 (cdecl (window, message, a, b); name unconfirmed): the candidate window's raw handler: ImeHost_OnMessage on the host 0x7b4e38.
s32 __cdecl ImeHost_WindowProc(u8 *window, u32 message, u32 a, u32 b);
}
