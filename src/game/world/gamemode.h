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

// What GameMode_StepBack calls (the benches point them at recorders): agent B's and agent C's unported code, and the ported ones it
// hands the stepped point to, so its bench can run them as recorders.
struct GameModeStepEnv {
    void(__cdecl *endRun)();                                          // 0x444f30 (agent B's; cdecl, no arguments)
    void(__fastcall *thing4de1f0)(u8 *thing);                         // 0x4de1f0 (agent C's; thiscall on a thing, no arguments)
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);  // GameMode_MapClick (0x524960)
    void(__cdecl *set)(u32 mode, u32 record);                         // GameMode_Set (0x52f200)
    void(__cdecl *setOnly)(u32 mode, u32 record);                     // GameMode_SetOnly (0x52f580)
    void(__cdecl *relink)(u32 corners);                               // TrackCell_Relink (0x52fc80)
    u8(__cdecl *updateA)(u8 *thing, u32 flash, u32 start, u32 bank5); // TrackRun_UpdateA (0x52c6f0)
    void(__cdecl *updateB)(u8 *thing, u32 flash, u32 start, u32 bank5); // TrackRun_UpdateB (0x52ccb0)
};
// GameMode_StepBack's calls go to env (nullptr: the game's).
void GameMode_SetStepEnv(const GameModeStepEnv *env);

// 0x52fe50 (cdecl (thing, relink); name invented, unconfirmed): one step of the current mode ([0x81ae2c]) taken back. Mode 0x15: endRun and
// 0. Modes 0xe, 0x13, 4, 0x3b and 0x16: 0. Otherwise the point stack (count [0x820a8c], points at 0x81b738 + 8 * count) popped into
// 0x81adb0 / 0x81adb4 when it has one; when that leaves it empty (or it was), the count is 1 (only when it had one) and nothing is
// clicked: modes 0xb, 0x10 and 0x35 relink (0) and, with relink's low byte and a track run [0x8186ac], update it (TrackRun_UpdateA (run,
// 0, 0, 0), and when that answers 0 TrackRun_UpdateB (run, 1, 0, 0)); then GameMode_SetOnly (mode, 1) and 0. With points left:
// Global816d48_Set(0); modes 1, 3 and 0x34 GameMode_Set(0x34, 1) (the thing's call below armed), modes 0xb, 0x10, 0x19 and 0x35
// GameMode_Set(0x35, 1) and 0x763b00 / 0x763b04 0x80; the popped point clicked (GameMode_MapClick (x, y, 0, 0)); the new top point (if
// any) copied to 0x81adb0 and clicked, unless the mode is 3 and its 0x44-byte cell's +8 is 1, and kept at 0x81ede4 / 0x81ede8 (a count
// of 1 there arms the relink's corners); GameMode_SetOnly (mode, 1); when armed, 0x4de1f0 on the thing (or the run [0x8186ac]) when there
// is one; modes 0xb, 0x10 and 0x35 relink (corners) and update the run as above; 1.
u32 __cdecl GameMode_StepBack(u8 *thing, u32 relink);

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
