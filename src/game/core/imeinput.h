// The IME's work on a window (core/ime has its object): attaching to a window's procedure, the window messages, the composition strings and the
// candidate lists, from 0x478860 to 0x47a950. All names invented, unconfirmed. An ImeContext is { window, context } (core/ime). The map at
// 0x7b4a40 (tree A; guard bit 0x7b4a38; its atexit 0x4787e0) takes a window to its IME. +8 is the composition, +0x18 the result, +0x38 the
// 0x24-byte candidate lists: +0 a counter, +4 the page size, +8 the page start, +0xc the selection, +0x10 a vector of wide strings, +0x20 a hash.
// Imports go through their thunks in the original (0x6b5570 on); here they are read live from their slots.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

void ImeInput_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x478860 (thiscall on an ImeContext; kind, out, size, pops 12): ImmGetCompositionStringA into a 0x40-byte local, a negative answer returned;
// ended at the answer (0x40 would end on the return address, sic) and widened (MultiByteToWideChar, code page 0, up to the first 0) to out and
// size. Out ended at the count unless null. Returns the count.
s32 __fastcall ImeContext_ReadComposition(const u32 *context, void *, u32 kind, u16 *out, s32 size);

// 0x4788c0 (thiscall on an ImeContext; index, text, a size unread, reading, a size unread, pops 20): each of the two wide strings, unless null,
// narrowed into a local string (WideCharToMultiByte, sized by a first call: a negative answer returned, the strings released);
// ImmSetCompositionStringA with them (a string's null pointer as 0x6fe5b0; a null one 0 and 0). Returns its answer.
s32 __fastcall ImeContext_SetComposition(const u32 *context, void *, u32 index, const u16 *text, u32, const u16 *reading, u32);

// 0x478d40 (thiscall; window, wparam, flags, pops 12): WM_IME_COMPOSITION. Flag 0x800: the result into +0x18 and the composition into +8, each
// through its own context; else flag 8: the composition into +8; else, for flags 0, +8 = the wide text at 0x78a374, and that is all. Then with a
// composition longer than the limit +0x58, its first limit characters: none, ImmNotifyIME 0x15 (NI_COMPOSITIONSTR) 4; else put back (index 9,
// SetComposition), after that same notify unless the keyboard layout's description starts "Microsoft IME 98" (0x74f8b0, 16 bytes).
void __fastcall Ime_OnComposition(u8 *ime, void *, u32 window, u32 wparam, u32 flags);

// 0x479380 (thiscall; window, pops 4): with a context that is not open: the lists cleared (the original's std::copy runs from the last to the
// last, sic), +8 = the text at 0x78a374 and ImmSetCompositionWindow style 2, position 0.
void __fastcall Ime_ResetIfClosed(u8 *ime, void *, u32 window);

// 0x47a570 (thiscall): with a window +0x4c: its context closed, the procedure +0x50 put back, the window off the map, +0x4c and +0x50 cleared and
// +8 = the text at 0x78a374.
void __fastcall Ime_Detach(u8 *ime);

// 0x47a3a0 (thiscall; out, window, pops 8): *out = -1 when already attached, with no context to make (+0x5c, ImmCreateContext) or the window
// on the map. Else +0x60's 0x100 bytes cleared, the limit 12 and the window mapped to the IME; its procedure replaced by Ime_WindowProc, the old
// one +0x50. No old one with a last error: the window off the map and *out = -1. Else +0x4c = the window, its composition cancelled
// (ImmNotifyIME 0x15, 4) and *out = 0. Returns out.
u32 *__fastcall Ime_Attach(u8 *ime, void *, u32 *out, u32 window);

// 0x47a0c0 (thiscall; an ImeContext, the list, pops 8): list index's strings emptied, its counter raised and it read again
// (ImmGetCandidateListA, its size asked first; 0: returns 0): page size, page start, selection, then each candidate widened and appended, or
// when that fails the text at 0x78a374; +0x20 = the hash of the widened ones (rotate left, xor each character). The local strings' allocator
// bytes are the top byte of the context's pointer (sic), and the vector's strings copy them. Returns 1.
u32 __fastcall Ime_ReadCandidates(u8 *ime, void *, const u32 *context, u32 index);

// 0x479d80 (thiscall; window, mask, pops 8): with a context, the lists brought to 32 (a blank list's first dword and strings zero, its allocator
// the window's low byte, its other dwords unset in the original), and every list of the mask read: for each that has one, +4 = 2 and +0x54 =
// its index. 0x479f10 (thiscall; window, mask, pops 8): the same with or without a context, stopping at the first that has one.
void __fastcall Ime_OpenCandidates(u8 *ime, void *, u32 window, u32 mask);
void __fastcall Ime_ChangeCandidates(u8 *ime, void *, u32 window, u32 mask);

// 0x4796b0 (cdecl; layout): the keyboard layouts (GetKeyboardLayoutList) searched from the last for layout: not there, 0. Else taken backwards
// round from it, the first without ImmGetProperty(4) bits 0x30000 (0 if none).
u32 __cdecl Ime_PickLayout(u32 layout);

// 0x4798e0 (thiscall; window, message, wparam, lparam, pops 16): the IME's window messages; what it does not take goes to the old procedure
// +0x50 or, without one or for a key up whose down it did not see, DefWindowProcA.
u32 __fastcall Ime_OnMessage(u8 *ime, void *, u32 window, u32 message, u32 wparam, u32 lparam);

// 0x47a880 (stdcall; the window procedure): Ime_OnMessage of the window's IME on the map, or DefWindowProcA.
u32 __stdcall Ime_WindowProc(u32 window, u32 message, u32 wparam, u32 lparam);
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gImeinput_7b4a38, 0x007b4a38, 0x1);
