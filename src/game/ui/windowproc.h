// The game window's message procedure: reimplementation of 0x46b600. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// The procedure's calls go to these (the game's own by default; the bench points them at recorders).
void WindowProc_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x46b600 (stdcall; window, message, wparam, lparam). With the width word 0xfaa5c4 still 0 the screen's size is set from 0x7a1968, 0x7a1930 (Gui_SetScreenSize). Unless the flag 0x7b4b88 is on,
// lparam gives x = low word * 0x800 / width and y = high word * 0x600 / height 0xfaa5c0 for every message, and 0x200 goes to Gui_MouseMove(x, y), 0x201 / 0x202 to Gui_MouseDown / Up(0, x, y),
// 0x204 / 0x205 with 1, 0x207 / 0x208 with 2. Then:
// 0x1c: with wparam the texture cache [0x8c4b44] (when there is one) releases its items (1), the bit 8 of 0x7a1a14 is set, Gui_TellActive, with [0xf82884] Sound_Post700b68, and with the flag
// ShowCursor(0); without it the bit is cleared, Movie_Stop, Gui_TellInactive, with [0xf82884] Sound_Post700b64, else GameOwner_Pause(1, 1). EventQueue_PostKind4(wparam), and DefWindowProcA answers.
// 0x14, 0x20: 1.
// 0x100, 0x104 (0x101, 0x105): the key: the numeric pad's * + - . / (0x6a, 0x6b, 0x6d, 0x6e, 0x6f) wparam's low byte moved up a byte; another MapVirtualKeyA(the low word signed, 2) when that word is
// not 0, else wparam, and moved up the same with lparam's bit 24. Gui_KeyDown (Gui_KeyUp)(key, lparam's low word | the modifiers << 16 | bit 30 << 24), then Actions_FireDown (Actions_FireUp) on
// [0x78718c] (key, the modifiers): 3, 0xc and 0x30 for the shift, control and alt keys held (GetKeyState, asked anew for each). Answers 0.
// 0x102: wparam's bytes (on the stack, in lparam's place) made wide (MultiByteToWideChar, ACP, no flags, -1, 2 units): nothing when that answers <= 0 or the low unit is under 0x20. In Hebrew
// (Language_IsHebrew on 0x78b530) Code_Remap46b460 on it. Gui_KeyChar(it, lparam's low word | bit 30 << 24). Answers 0.
// 0x20a, and with 1 as the answer the message [0x7a1c5c]: Gui_MouseWheel(wparam's high word signed / 120, x, y, 0) with x and y as above (whatever the flag).
// 0x410: Gui_KeyChar(wparam, 0). Answers 0. All else: 1.
u32 __stdcall GameWindow_Proc(u32 window, u32 message, u32 wparam, u32 lparam);

// 0x46bcb0 (stdcall): the window class's procedure (gamewindow.cpp registers it by address). 2 (WM_DESTROY): PostQuitMessage(0), answers 0. Else GameWindow_Proc; its 1 goes to
// DefWindowProcA, anything else answers 0.
u32 __stdcall GameWindow_ClassProc(u32 window, u32 message, u32 wparam, u32 lparam);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gWindowproc_7a1a14, 0x007a1a14, 0x4);
