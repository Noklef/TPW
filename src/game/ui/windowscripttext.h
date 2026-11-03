// The GUI library's window scripts in text: an array of lines (CRE, HITREGION, ADDUPBUT, END and others; ui/guitoken's tokens) that builds a window tree, as ui/windowscript's words do. Reimplementation of 0x66080d, 0x66149c and 0x6607ca; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66080d (thiscall (line, out), pops 8; AX only): one line on the window. The answer is the line's command (GuiToken_ReadCommand), -1 for an unknown one or a CRE that made no child, 0 when a child was made (its address in *out, without out only the answer).
// 0 CRE: kind, flags, a name (not used), id, rectangle: a child. 1 DRAWOBJ / 2 CLIENTOBJ: a registered window's name: the window's +0x98 object / +0x9c impl. 3 CLIENTAREA: a rectangle to +0x2c (Window_OffsetByParents of it with flags 0x400). 4 HITREGION: a shape (RECT, CIRCLE,
// ELLIPSE, POLY; another does nothing) and its numbers: the hit area. 6 ADDUPBUT / 7 ADDDNBUT: for a kind 3 without flag 0x40 and without a child of id 1 / 2: a button (2, 0x21). 8 ADDTHUMBBUT: for a kind 3 without a child of id 3: a button (2, 1) with the handler 0x66bb9b.
// 9 ADDSCROLL: for a kind 4 or 7 without a child of id 1: a scroll bar (3, 0x11). 0xa LISTRECT: the inner rectangle of a kind 4 or 7. 0xb SETCOLUMNS: a kind 7's column count and its columns. 0xc ADDCOLUMNBUT: for a kind 7, an id word and a name (not used) and a rectangle:
// a button (2, 1, id + 0x10). 0xd ANG: a kind 11's range (without a pair 0 to 0x1000). 5 END and past: nothing.
u32 __fastcall WindowScriptText_Line(u8 *window, void *, char *line, u8 **out);

// 0x66149c (thiscall (lines, out, start), pops 12; AX only): the lines from `start` on the window until END, returned its index. Each line that makes a child: *out (without out nothing), and the lines after it run on the child, the index going on after that child's END.
u32 __fastcall WindowScriptText_Lines(u8 *window, void *, char **lines, u8 **out, u32 start);

// 0x6607ca (thiscall (lines, handler), pops 8): the lines from the first on the window. The last child made directly on it, and handler (not 0) becomes that child's. Returns that child (0 for none).
u8 *__fastcall WindowScriptText_Build(u8 *window, void *, char **lines, u32 handler);
}
