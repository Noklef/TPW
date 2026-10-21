// The key maps' action handlers (the up handler at +0xc of a map's 0x14-byte entries, src/game/core/keymap; Actions_FireUp calls it
// with no arguments): reimplementation of those whose callees are ported. Each is cdecl and returns 1. Names unconfirmed; the maps'
// keys are noted (a word: the virtual key, modifiers 3 shift, 0xc control, 0x30 alt).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40c120 / 0x40c150 (',' / '.'): the dword 0x81d7a4 (Global81d7a4_Get_52f1f0) plus 90 / 270, signed remainder by 360 (idiv), set
// through Global81d7a4_SetRecorded with force 1.
u32 __cdecl KeyAction_Turn90();
u32 __cdecl KeyAction_Turn270();

// 0x40c3b0 / 0x40c3c0 / 0x40c3d0 (numpad '-' / '+' / '*'): the game clock (0x785970) slower (GameClock_Slower) / faster
// (GameClock_Faster) / at rate 1.0 (GameClock_SetRate).
u32 __cdecl KeyAction_ClockSlower();
u32 __cdecl KeyAction_ClockFaster();
u32 __cdecl KeyAction_ClockNormal();

// 0x40c450 (alt '1'): 10000 credited (Money_Credit) to the world's money record (GameTable7cfb90_Get1da726_5195d0 of [0x7cf83c]).
u32 __cdecl KeyAction_Credit10000();

// 0x40c480 (F8 with every modifier): the dword 0x787d1c = 1 - itself, then Global878128_Set_550510 of it.
u32 __cdecl KeyAction_Toggle787d1c();

// 0x40c830 (control 'T'): bits 0xc of the render view's flags (0x8bcbc8) stepped 0 -> 0xc -> 4 -> 0 (8 counts as 0: | 0xc; 4: cleared;
// 0xc: 4).
u32 __cdecl KeyAction_ViewFillStep();

// 0x40c430 (alt 'R'): the research lab (GameTable7cfb90_Get1da722_519550 of the game [0x7cf83c]) flags its projects' object controls
// (ResearchLab_FlagProjectControls).
u32 __cdecl KeyAction_FlagResearchControls();

// 0x40c4b0 (F3): the overlay toggled (Obj7b51f0_ToggleOverlay on 0x7b51f0).
u32 __cdecl KeyAction_ToggleOverlay();

// 0x40c5d0 (control 'H'): the GUI manager's slot 4 or 3, its word to the byte 0x78d90e (Obj7b51f0_GuiManagerSlotAndFlag on 0x7b51f0).
u32 __cdecl KeyAction_GuiManagerSlotAndFlag();

// 0x40c820: the version's message tag added and opened (the thunk 0x481b70 to MsgTags_AddVersion, ECX 0x7b51f0 not used).
u32 __cdecl KeyAction_ShowVersion();
}
