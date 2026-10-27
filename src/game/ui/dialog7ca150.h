// The dialog whose page [0x7ca150] holds the objects' lists (its child 0x12c4b8 the page they are built on; the list [0x7ca14c], its rows
// List7ca158's by the tab [0x7ca154]). Reimplementation of 0x494250, 0x494c70, 0x494ec0 and 0x495110. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x494ec0 / 0x494c70 / 0x494250 / 0x495110 (cdecl (selected); the tabs 0-3; names unconfirmed): the page (the child 0x12c4b8 of
// [0x7ca150]) has its child 400 sent 4 (0, 0) when there is one (sic: looked for three times, the same search); then a list is built on the
// page from the tab's script (WindowScript_Build: 0x750ab8 for tabs 0 and 1, 0x750ba0 for 2, 0x750ca0 for 3; no handler), [0x7ca154] the
// tab, the list's handler 0x4955e0, its rows List7ca158_NewRow's initialised by List7ca158_InitRow (Window7059f0_BuildRows), its columns'
// bottoms 0 for the first and 1 for the rest (5 columns, 6 for tab 2, 2 for tab 3), the "hilight" window as its highlight
// (GuiWindows_FindByName, Window7059f0_SetHighlight), sorted by the tab's dword (0x750d5c-0x750d68: Window7059f0_SortBy, not sorting);
// [0x7ca14c] the list; the tab's things added by their callbacks (Things_ForEachSelected: 0x80 0x493e90 then 0x2000 0x493eb0, 0x200
// 0x493f30, 0x100 0x494070, 0x400 0x4941e0); the rows refreshed; a selected data (not 0) found among the items (Window7059f0_FindItemData)
// and scrolled to; then each column's header button (the list's children 0x10 on) given its text (Language_WideText of the tab's ids, the
// font [0x750d58], the colour 0xff, 0xef, 0 and the tab's value: ButtonText_Init). EAX not kept.
void __cdecl Dialog7ca150_ShowTab0(u32 selected);
void __cdecl Dialog7ca150_ShowTab1(u32 selected);
void __cdecl Dialog7ca150_ShowTab2(u32 selected);
void __cdecl Dialog7ca150_ShowTab3(u32 selected);
}
