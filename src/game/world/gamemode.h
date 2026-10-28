// The game mode [0x81ae2c] changed (0x52f200, 0x52f580; names invented, unconfirmed): the previous mode kept at 0x81af64, the advisor's
// messages about the old mode taken back, and the new mode's set-up (the online mouse mode for mode 0, the cursor, an advisor message,
// the track run shown).
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the mode change calls in the game that is not ported yet (the benches point it at recorders).
struct GameModeEnv {
    void(__fastcall *advisorCancel)(u8 *advisor, void *, u32 id);   // 0x59aa70 (agent B's; thiscall on the advisor (id), pops 4)
    void(__cdecl *showRun)(u32 run, u32 on);                       // 0x4474d0 (agent B's; cdecl (run, on))
    void(__cdecl *resetTools)();                                   // 0x446050 (agent B's; cdecl)
};
// The calls go to env (nullptr: the game's).
void GameMode_SetEnv(const GameModeEnv *env);

// 0x52f580 (cdecl (mode, record); name invented, unconfirmed; the replay of action 0x19, 0x404dd0, calls it without record). With record
// (its low byte) the action 0x19 of the mode written (ActionRec_WriteW on 0x785a08). A mode other than the current one ([0x81ae2c]): the
// advisor of the world [0x7cf83c] (GameTable7cfb90_Get1da71c_5194d0, asked each time) cancels the messages 0xb8, 0xcb, 0xd6, 0xcd, 0xc9, 0xce,
// 0xc1, 0x159, 0x15a and 0x151 (agent B's unported 0x59aa70 through GameModeEnv). Then the mode stored and the old one kept at 0x81af64.
// Mode 0, unless the old one was 0x15 or 0x16: a new CMMOnline base (Mem_Alloc(8), MouseModeOnlineBase_Ctor; 0 when that failed), the
// holder [0x7b05e8] made when missing (MouseModes_EnsureHolder, inlined) and the mouse mode set (MouseMode_Set). Mode 3: the advisor
// message 0xcb posted (Advisor_PostId, inlined) and the cursor 4 (GameCursor_Set). Modes 0x15 and 0x16: the track run [0x8186ac] shown
// (agent B's unported 0x4474d0 (run, 1)). Mode 1: the cursor 3.
void __cdecl GameMode_SetOnly(u32 mode, u32 record);

// 0x52f200 (cdecl (mode, record); name invented, unconfirmed): GameMode_SetOnly with the action 0 recorded, and 0x81ede4 and 0x81ede8 set to
// -1 after the mode is stored. Mode 0 also resets the tools first: the words 0x818694 and 0x818684 0, 0x7639b0 and 0x7639b4 -1, the dwords
// 0x763b00 and 0x763b04 0x80, 0x7639a0 -1 and 0x81b604 0; with [0x81b134] set the action 0xa of 0 written; [0x81b134] = [0x81d7a4] and
// [0x81d7a4] 0; then (whatever the old mode) after the mouse mode agent B's unported 0x446050 and Global816d48_Set(0). Mode 0x35: 0x763b00
// and 0x763b04 0x80.
void __cdecl GameMode_Set(u32 mode, u32 record);
}
