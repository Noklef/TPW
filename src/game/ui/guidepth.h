// The GUI library's top depth: the depth (word +0xe0) just above the shown windows of a tree, or of those of them a shape overlaps, kept in
// the dword 0xfaa594 (the walk's last window flags in 0xfaa570 and its last depth in 0xfaa5c8). Reimplementation of 0x659409 and 0x659437.
// The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x659437 (cdecl (window, shape)): for the GUI root [0xfaa5d8] [0xfaa594] starts at its depth (Fn_659730, sign-extended). Then each child
// (Fn_65d547, then Fn_65d56b) whose flags (Fn_65d57c, kept in [0xfaa570]) have bit 0 and not bit 1: its depth (Window7059f0_Item148Depth
// for kind 7 (its slot 3), else Fn_659730; sign-extended, kept in [0xfaa5c8]) when above [0xfaa594] (signed) makes [0xfaa594] that + 1,
// with a shape only when the child's inner rectangle (Window_GetRect(.., 1) into a GuiRect on the stack) overlaps it at all
// (GuiRect_Overlap's AX not 0); then the child's own children the same way. Returns [0xfaa594].
s32 __cdecl Gui_TopDepthIn(u8 *window, u8 *shape);

// 0x659409 (cdecl (window, shape)): Gui_TopDepthIn of the window, the GUI root [0xfaa5d8] for 0; -1 with neither.
s32 __cdecl Gui_TopDepth(u8 *window, u8 *shape);
}
