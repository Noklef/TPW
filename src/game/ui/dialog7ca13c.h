// The dialog at 0x7ca13c (the guests' list: the window made from the script 0x7506c8 with the handler 0x493230 on the GUI parent
// [0x7cb2ac]; its list 0x1e497 [0x7ca138], its rows List7ca140's). Reimplementation of 0x493530. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x493530 (cdecl (selected); name unconfirmed): the window [0x7c24c8] closed (Window7c24c8_Close), Global7cb28c_Select(2, 6), the dialog
// built on [0x7cb2ac] from the script 0x7506c8 with the handler 0x493230 (WindowScript_Build; none: nothing more); [0x7ca13c] the dialog,
// made the window [0x7c24c8] (Window7c24c8_Set). Its child 0x1e497, the list, when there is one: its rows List7ca140_NewRow's
// initialised by List7ca140_InitRow (Window7059f0_BuildRows), its six columns' bottoms 1, the "hilight" window its highlight
// (GuiWindows_FindByName, Window7059f0_SetHighlight), sorted by [0x7508bc] (Window7059f0_SortBy, not sorting); the child looked up again
// into [0x7ca138], [0x7ca134] 0, the things of the group 0x40 added by 0x4937e0 (Things_ForEachSelected), the rows refreshed (the second
// lookup's); a selected data (not 0) found among the items (Window7059f0_FindItemData) and scrolled to; the header buttons 0x10-0x15 given
// their texts (Language_WideText 0x71-0x76, the font [0x7508b8], the colour 0xff, 0xef, 0, the values 0x81-0x86: ButtonText_Init). Then
// (list or not) the dialog's timer 0x80083 set to 2000 (Window_SetTimer) and, when that answers (AX), started; the advisor's message 0x11e
// (Advisor_PostId). EAX not kept.
void __cdecl Dialog7ca13c_Open(u32 selected);
}
