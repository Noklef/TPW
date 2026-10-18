// The GUI library's mouse cursors: a table [0xfaa604] of +0 the cursor objects and +4 the word count, the current cursor id the word
// 0x77c484 (its object [0xfaa608]), the default id the word 0xfaa600, and each window's id at its word +0x50 (applied while it is the
// hovered window [0xfaa5e0]). Reimplementation of 0x661d96, 0x6590f7 and 0x65f11d. The names are unconfirmed.
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

}
