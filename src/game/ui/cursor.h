// The GUI library's mouse cursors: a table [0xfaa604] of +0 the cursor objects and +4 the word count, the current cursor id the word
// 0x77c484 (its object [0xfaa608]), the default id the word 0xfaa600, and each window's id at its word +0x50 (applied while it is the
// hovered window [0xfaa5e0]). The game sets its cursor on the window [0x7cb2ac], or with none on the two at 0x7cc8c0 and 0x7cc8bc.
// Reimplementation of 0x661d96, 0x6590f7, 0x65f11d, 0x65f150, 0x4a2aa0, 0x4814a0 and 0x52f950. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x661d96 (thiscall (index), pops 4): the entry at the index (its low word) when that is below the count, else 0.
u8 *__fastcall CursorTable_Get(const u8 *table, void *, u32 index);

// 0x6590f7 (cdecl (id); only its low word is read): nothing with no table or when the id is the current one (both signed); -1 means the
// default id. The current cursor's slot 4 (0) and its byte +4 (Fn_66190a) taken; then the new one (or the default one when it has none)
// stored at 0xfaa608 and its id at 0x77c484, and when it is set its slot 4 (1), then its slot 2 when that byte was nonzero. The table
// is read again for each lookup.
void __cdecl Gui_SetCursor(u32 id);

// 0x65f11d (thiscall (id), pops 4): the word +0x50 = id; when the window is the hovered one [0xfaa5e0], Gui_SetCursor(word +0x50).
void __fastcall Window_SetCursor(u8 *window, void *, u32 id);

// 0x65f150 (thiscall): Gui_SetCursor(word +0x50).
void __fastcall Window_ApplyCursor(const u8 *window);

// 0x4a2aa0 (cdecl (id)): with the window [0x7cb2ac] set, Window_SetCursor on it, then the dword 0x7cb2d8 = the id's low word sign-extended.
void __cdecl GameCursor_Set(u32 id);

// 0x4814a0 (thiscall on the object at 0x7b51f0, this not used (id), pops 4): GameCursor_Set with the window [0x7cb2ac] set; otherwise
// Window_SetCursor on each of the windows [0x7cc8c0] and [0x7cc8bc] that is set.
void __fastcall Obj7b51f0_SetCursor(void *self, void *, u32 id);

// 0x52f950 (cdecl): the cursor for the game mode [0x81ae2c] (GameCursor_Set): mode 1, 3, 0xb and 0x10 give 5 with the dword 0x816d60
// set, else 0x12 with 0x816d4c set, else 3, 4, 6 and 6; mode 0x15 and 0x16 give 0x12 with 0x816d4c set, else 0xb and 0x11; mode 4 and
// 0x3b give 7, mode 0x39 gives 3; other modes none. Then 8 when Global816d48_Get() is nonzero and 0x14 when the dword 0x816d5c is set.
void __cdecl GameCursor_Update();

}
