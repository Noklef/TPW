// The mouse as the game's input layer sees it: the mouse object at [0x7b05d8] (a recorder-friendly vtable of button and move handlers), the last position
// ([0x7b05d4], [0x7b05e0]) and the button flags ([0x7b05e4] left, [0x7b05f0] right). Reimplementation of 0x46c210, 0x46c2a0 and 0x486c70.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x46c210 (cdecl; event 0xf0 / 0xf1, state, a, b): through the mouse object at [0x7b05d8] the left (0xf0, slots 1 and 2 at +4 and +8) or right (0xf1, +0xc and +0x10) button goes
// down (state 1) or up with (a, b), and the flag [0x7b05e4] / [0x7b05f0] is set to 1 / 0. Returns 1.
u32 __cdecl Mouse_HandleButton(u32 event, u32 state, u32 a, u32 b);

// 0x46c2a0 (stdcall; x, y, pops 8): when the position differs from the last one ([0x7b05d4], [0x7b05e0]) the mouse object (slot 6 at +0x18) gets (x, y, [0x7b05e4], [0x7b05f0]);
// then slot 7 (+0x1c) with (x, y) while the left button flag is set, else slot 8 (+0x20) while the right one is; both last positions are then set to x (sic).
void __stdcall Mouse_Moved(s32 x, s32 y);

// 0x486c70 (cdecl; event 0xf0-0xf2, state, unused, x, y; the hook the front end keeps at 0x7b9ff0; name unconfirmed): the point scaled to
// the GUI's 2048 x 1536 (x << 11 / the word [0xfaa5c4], y * 0x600 / the word [0xfaa5c0]: 32-bit products, signed divisions, made for every
// event); the button 0xf0 - event (0-2) with its flag 0x7c24dc + 4 * button goes down (state set, the flag 0: the flag 1 and
// Gui_MouseDown, ui/window) or up (state 0, the flag set: the flag 0 and the release); other events nothing. Returns 1.
u32 __cdecl Mouse_HandleGuiButton(u32 event, u32 state, u32 unused, s32 x, s32 y);

}  // extern "C"
