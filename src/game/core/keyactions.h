// The key maps' action handlers (the up handler at +0xc of a map's 0x14-byte entries, src/game/core/keymap; Actions_FireUp calls it
// with no arguments): reimplementation of those whose callees are ported. Each is cdecl and returns 1. Names unconfirmed; the maps'
// keys are noted (a word: the virtual key, modifiers 3 shift, 0xc control, 0x30 alt).
#pragma once

#include "../../../re/types.h"

struct AddrCall;
void KeyActions_SetCalls(const AddrCall *table, u32 count);   // tests/fncmp/keyactioncalls.py

extern "C" {

// 0x40bf70: GameOwner_TogglePause (1) on 0x786b68. 0x40c3a0: FrontEnd_Hide7ca1c8 on 0x7b51f0. 0x40c3f0 / 0x40c410: Game_OpenPark (1, 0) / (0, 0) on the game [0x7cf83c]. 0x40c800: Advisor_PostHeld on
// the game's 0x5194d0 object. 0x424c10: the dword 0x78d96c = 0. Names invented, unconfirmed.
u32 __cdecl KeyAction_TogglePause();
u32 __cdecl KeyAction_HideFrontEnd();
u32 __cdecl KeyAction_OpenParkNew();
u32 __cdecl KeyAction_OpenParkLast();
u32 __cdecl KeyAction_AdvisorPostHeld();
u32 __cdecl KeyAction_Clear78d96c();

// 0x40c120 / 0x40c150 (',' / '.'): the dword 0x81d7a4 (Global81d7a4_Get_52f1f0) plus 90 / 270, signed remainder by 360 (idiv), set
// through Global81d7a4_SetRecorded with force 1.
u32 __cdecl KeyAction_Turn90();
u32 __cdecl KeyAction_Turn270();

// 0x40c180 (name invented, unconfirmed): 1 when Obj7b51f0_ResetDialogChild21 answers nonzero. Else the mouse mode holder made when missing (inlined), 0 when the current mode [0x7b05d8] answers 0
// from slot 0x24 (none: 0), the holder again, 0 when it answers 1. Else a new CMMOnline base (Mem_Alloc(8), MouseModeOnlineBase_Ctor; 0 when that failed) made current (the holder again;
// MouseMode_Set), GameMode_Set (0, 1); returns 1.
u32 __cdecl KeyAction_CancelMouseMode();
// 0x40bda0 (name invented, unconfirmed): with game mode 0 (Global81ae2c_Equals) and the current mouse mode's kind 0 (none too) or 1: the cell under the cursor (id, the word 0x7b05cc: column (id - 1) & 0x7f,
// row (id - 1) >> 7 of the zero-extended word; the world's cell at [0x7cf83c] + id * 0x44 + 0x294): when its type is 1, sound 0x5f in category [0x803a2c] at (column * 10, 0, row * 10), game mode 0x32, a map click
// on it, game mode 0. Otherwise with kind 3: GameMode_StepBack (0, 1). Always returns 1.
u32 __cdecl KeyAction_ClickCell();
// Their calls go to env (nullptr: the ports).
struct KeyCancelEnv {
    u32(__fastcall *reset)(void *self);        // Obj7b51f0_ResetDialogChild21 (0x4816b0)
    void(__cdecl *setMode)(u32 mode, u32 record);  // GameMode_Set (0x52f200)
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt (0x51bfc0)
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);  // GameMode_MapClick (0x524960)
    u32(__cdecl *stepBack)(u8 *thing, u32 relink);  // GameMode_StepBack (0x52fe50)
};
void KeyCancel_SetEnv(const KeyCancelEnv *env);

// 0x40c3b0 / 0x40c3c0 / 0x40c3d0 (numpad '-' / '+' / '*'): the game clock (0x785970) slower (GameClock_Slower) / faster
// (GameClock_Faster) / at rate 1.0 (GameClock_SetRate).
u32 __cdecl KeyAction_ClockSlower();
u32 __cdecl KeyAction_ClockFaster();
u32 __cdecl KeyAction_ClockNormal();

// 0x40c450 (alt '1'): 10000 credited (Money_Credit) to the world's money record (GameTable7cfb90_Get1da726_5195d0 of [0x7cf83c]).
u32 __cdecl KeyAction_Credit10000();

// 0x40c470 (F8; name invented, unconfirmed): the screen grab's next free name taken
// (ScreenGrab_NextName); returns 1.
u32 __cdecl KeyAction_ScreenGrab();

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

// 0x40c6c0 / 0x40c6e0 / 0x40c700 / 0x40c720 / 0x40c740 / 0x40c760 (cheat keys: their action-table entries from 0x748504 on; names
// invented, unconfirmed): theme 0-5 marked (GlobalSave_MarkTheme (i) on GlobalSave_Instance); return 1.
u32 __cdecl KeyAction_MarkTheme0();
u32 __cdecl KeyAction_MarkTheme1();
u32 __cdecl KeyAction_MarkTheme2();
u32 __cdecl KeyAction_MarkTheme3();
u32 __cdecl KeyAction_MarkTheme4();
u32 __cdecl KeyAction_MarkTheme5();

// 0x40c780 / 0x40c7a0 / 0x40c7c0 / 0x40c7e0 (cheat keys, the entries after those; names invented, unconfirmed): a score of 100 on
// theme 0-3 (GlobalSave_MarkScore (i, 100) on GlobalSave_Instance); return 1.
u32 __cdecl KeyAction_MarkScore0();
u32 __cdecl KeyAction_MarkScore1();
u32 __cdecl KeyAction_MarkScore2();
u32 __cdecl KeyAction_MarkScore3();

// 0x40c4c0-0x40c5c0, the shortcut table 0x748378's entries (names from its key names "postcard" .. "camcorder"; invented, unconfirmed):
// each calls one entry point with ECX 0x7b51f0 (the front end; the entry points are cdecl and do not read it) and returns 1.
u32 __cdecl KeyAction_Postcard();      // 0x40c4c0: 0x481500, a JMP to GamePause_Hide 0x4a9380
u32 __cdecl KeyAction_Menu();          // 0x40c4d0: FrontEnd_PanelMenu 0x4816d0
u32 __cdecl KeyAction_Buy();           // 0x40c4e0: 0x4816e0 (Dialog7cc1f8_Open (-1, 0))
u32 __cdecl KeyAction_Hire();          // 0x40c4f0: 0x481700 (Dialog7ca308_Open (-1))
u32 __cdecl KeyAction_ParkStatus();    // 0x40c500: 0x481710 (Dialog7cb398_Create)
u32 __cdecl KeyAction_AllItems();      // 0x40c510: 0x481720 (Dialog7ca150_Open (-1, 0))
u32 __cdecl KeyAction_AllStaff();      // 0x40c520: 0x481730 (Dialog7ca168_Open (-1, 0))
u32 __cdecl KeyAction_AllPeeps();      // 0x40c530: 0x481740 (Dialog7ca13c_Open (0))
u32 __cdecl KeyAction_FinanceInfo();   // 0x40c540: FrontEnd_Dialog7ca2e8 0x481750
u32 __cdecl KeyAction_Loans();         // 0x40c550: FrontEnd_Dialog7cb25c 0x481760
u32 __cdecl KeyAction_StaffCosts();    // 0x40c560: FrontEnd_Dialog7cc2ec 0x481770
u32 __cdecl KeyAction_EntryPrice();    // 0x40c570: FrontEnd_Dialog7ca210 0x481780
u32 __cdecl KeyAction_Map();           // 0x40c580: FrontEnd_Other5f0b40 0x481510
u32 __cdecl KeyAction_StaffLoc();      // 0x40c590: ThingBrowser_OpenShown 0x481790
u32 __cdecl KeyAction_PeepLoc();       // 0x40c5a0: FrontEnd_Browser 0x4817c0
u32 __cdecl KeyAction_Research();      // 0x40c5b0: FrontEnd_Research 0x4817e0
u32 __cdecl KeyAction_Camcorder();     // 0x40c5c0: MouseMode_Enter6fead0 0x481a10
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gKeyactions_78d96c, 0x0078d96c, 0x4);
