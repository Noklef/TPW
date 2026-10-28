// The GUI library's list window (vtable 0x705950, src/game/ui/window705950) drawn: reimplementation of 0x47de30, its slot 4, which lies
// in the game's code rather than the library's.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47de30 (thiscall; vtable 0x705950 slot 4; name unconfirmed): the list's rows drawn as a tree. The rectangle +0x2c's edges read
// (GuiRect_GetEdges, 0x659de5) and then the inner one's (+0x138) over them (sic: the first read is lost); the clip (Display_SetClip) that
// one scaled: x * [0xfaa5c4] / 2048, y * [0xfaa5c0] / 0x600 (signed, toward 0). Palette_Nearest (palette 0xfa71f8) asked for black,
// white and grey for nothing (sic), then white for the text colour (0xffffffff) and (200, 0, 0) for the selection's (0xffc80000). The first
// row shown (+0x150) its node (Window705950_NthNode), the row's y the inner top (+0x13e). Nothing more unless font 1 is selected with the
// shape driver (Text_SelectFont(1, 0)): then the indent is printed from the text at 0x786de4 (sprintf's format) and the clip set; with a
// node, "    " (0x74f8d0) added to it for each parent (+0x4c), and for up to the rows shown (+0x156, signed, re-read) while there is a
// node: the row whose number plus the first shown is the selection (+0x148) filled (Display_FillRect) from the inner left + 2 to the inner
// right - 2 (words), the row's y to y + the row height (+0x154); the indent and the node's name (wide, at +0) printed ("%s%S", 0x74f8c8)
// and drawn (Text_Draw) at the inner left + 2 and the row's y. The next node: the first child (+0x48, the indent one "    " longer), else
// the next (+0x44), else the next of the nearest parent with one, the indent emptied (sic: not cut back), or none. y += the row height
// (words). The clip then cleared (Display_SetClip(0)). The indent is 32 bytes before the 128 of the line (an indent of 8 or more levels
// runs into it, sic); the colours' padding is what the stack held.
void __fastcall Window705950_Draw(u8 *window);
}
