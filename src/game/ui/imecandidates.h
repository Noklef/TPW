// The IME's candidate list (the window built from the script 0x74f9e0 for the IME host at 0x7b4e38, core/ime's ImeHost: its list
// 0x7b79e3 with the "hilight" list's rows). Reimplementation of 0x480b00 and 0x481100. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x480b00 (thiscall on the host (parent, second), pops 8; name unconfirmed): the window built on the parent from the script 0x74f9e0
// with the handler 0x480d10 (WindowScript_Build) into +0; placed by the host's window +4: both inner rectangles (Window_GetRect 1), the
// new one's size kept, put at the host window's left and bottom; when that bottom runs past 0x5f8 above its top instead, and when that
// is above 0 at its top and right (y 0 when that runs past 0x5f8 too); the left moved back to 0x7f8 less the width when it runs past
// 0x7f8 (Window_SetRectTo). Its depth the GUI root [0xfaa5d8]'s top one over its inner rectangle (Gui_TopDepth, Window_SetDepth). Its
// child 0x7b79e3 (Window_FindChild; not checked) given the "hilight" list's rows (HilightList_NewRow / HilightList_InitRow:
// Window7059f0_BuildRows), column 0's bottom 0, the GUI window "hilight" as its highlight; +8 that list, +0x10-+0x18 0, the list
// cleared (Window7059f0_Clear). Then for each of the IME's (+0x1c) cached strings (Ime_CacheCurrent's count) an item added
// (Window7059f0_AddItem: data its index, parent -1, no refresh) whose texts are the parent's stack slot (sic), holding the string's
// pointer (the 16-byte entry's +4, "" (0x6fd6c4) for none; 0 past the strings or uncached) and then the second argument; the rows
// refreshed (Window7059f0_RefreshRows of +8).
void __fastcall ImeHost_OpenCandidates(u8 *host, void *, u8 *parent, u32 second);

// 0x481100 (thiscall on the host; name unconfirmed): nothing unless the byte +0xc is set. The byte +0xd becomes whether the IME's (+0x1c)
// result (Obj_GetCOr6fd6c4) has characters, and the text window +4 told (Fn_668564). Without a candidate window +0: with cached strings
// (Ime_CacheCurrent) the list opened (ImeHost_OpenCandidates: the text window's parent +0x60 (Fn_65d55a) and its word +0xe0 + 2), else
// with a result now or before ImeHost_InsertResult. With one: no strings sends it 4 (0, 0) (Window_SendMessage); else the IME's current
// entry (Ime_Current) cached again as Ime_CacheCurrent does (inlined: the dwords, the vector, +0x180, the mark +0x184; none clears the
// mark), +0x188 = +0x180. When marked with the entry's +0x20 at +0x180: +0x10-+0x18 cleared, the list +8 cleared (Window7059f0_Clear),
// an item for each cached string (Window7059f0_AddItem: data its index, parent -1, no refresh; the texts a local pair: the string ("" for
// none, 0 past the strings or unmarked) and the count, sic) and the rows refreshed. Otherwise nothing more when +0x10 is already the
// position. Then +0x10 = the position (the cache's +0x16c when marked, else 0), the list scrolled there (Window7059f0_ScrollTo) and
// ImeHost_InsertResult.
void __fastcall ImeHost_Update(u8 *host);
}
