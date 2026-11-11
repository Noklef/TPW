// The dialog whose window is at 0x7ca1c8 (built by 0x498790 from the template 0x751530; 0x7ca1cc set while it is up): its nine check buttons
// (ids 0x18-0x21 in its child 0x18) shown from a flags word, or one of them selected. Reimplementation of 0x498090, 0x498a50, 0x498a80,
// 0x4981d0, 0x498a30, 0x4819f0, 0x4987f0 and 0x498790.
#pragma once

#include "../../../re/types.h"

// Dialog7ca1c8_OpenFor's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca1c8OpenEnv {
    void(__cdecl *focusCell)(u32 record);                  // FocusCell_Update
    void(__cdecl *resetChild)(u32 clear27);                // Dialog7cb2b8_ResetChild21
    u32(__cdecl *childPost)(u32 value);                    // Dialog7cb2b8_Child21Post
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);  // Window_SetFocus
    u32(__cdecl *linkOpen)(const u8 *object);              // Object_LinkOpen
    void(__cdecl *enterTile)();                            // MouseMode_EnterTile15
    void(__cdecl *editRide)(const u8 *object, u32 on);     // CoasterEdit_EditRide
    void(__cdecl *ensureMode)();                           // MouseMode_Ensure6feaa0
    void(__cdecl *cursor)(u32 id);                         // GameCursor_Set
    u32(__cdecl *tools)(u32 *out);                         // CoasterEdit_Tools
    void(__cdecl *setChecks)(u32 flags);                   // Dialog7ca1c8_SetChecks
    void(__cdecl *startTool)(u32 *out);                    // CoasterEdit_StartChosenTool
    void(__cdecl *selectButton)(u32 flags);                // Dialog7ca1c8_SelectButton
    void(__cdecl *updateLoop)();                           // Dialog7ca1c8_UpdateLoopButton
};
void Dialog7ca1c8_SetOpenEnv(const Dialog7ca1c8OpenEnv *env);

// Dialog7ca1c8_ResetMouseMode's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca1c8ResetEnv {
    u32(__cdecl *mode)();                         // Global81ae2c_Get_52f840
    void *(__cdecl *alloc)(u32 size);             // GameHeap_Alloc
    u8 *(__fastcall *baseCtor)(u8 *self);         // MouseModeOnlineBase_Ctor
    void(__stdcall *setMode)(u8 *mode);           // MouseMode_Set
    void(__cdecl *gameMode)(u32 mode, u32 record);   // GameMode_Set
};
void Dialog7ca1c8_SetResetEnv(const Dialog7ca1c8ResetEnv *env);

// Dialog7ca1c8_Handler's and Dialog7ca1c8_Hide's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7ca1c8HandlerEnv {
    void(__cdecl *enterTile15)();                                  // MouseMode_EnterTile15
    void(__cdecl *enterTile16)();                                  // MouseMode_EnterTile16
    void(__cdecl *ensureMode)();                                   // MouseMode_Ensure6feaa0
    void(__cdecl *cursor)(u32 id);                                 // GameCursor_Set
    void(__cdecl *toolButton)(u32 tool, u32 action);               // CoasterEdit_ToolButton
    u32(__cdecl *tools)(u32 *out);                                 // CoasterEdit_Tools
    u8(__cdecl *postId)(u32 id);                                   // Advisor_PostId
    u32(__fastcall *fireDown)(u8 *table, void *, u32 a, u32 b);    // Actions_FireDown
    u32(__fastcall *fireUp)(u8 *table, void *, u32 a, u32 b);      // Actions_FireUp
    void(__cdecl *resetMode)();                                    // Dialog7ca1c8_ResetMouseMode
    void(__cdecl *editRide)(const u8 *object, u32 on);             // CoasterEdit_EditRide
    u32(__cdecl *focusRestore)();                                  // Focus_Restore
    void(__cdecl *cancelAdvisor)(u32 id);                          // Game_CancelAdvisor
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
    void(__cdecl *resetChild)(u32 clear27);                        // Dialog7cb2b8_ResetChild21
};
void Dialog7ca1c8_SetHandlerEnv(const Dialog7ca1c8HandlerEnv *env);

extern "C" {

// 0x4982a0 (cdecl (window, message, a, b); the handler Dialog7ca1c8_Create gives; name invented): 0x14: [0x7ca1c8] = 0. 0x1d: the byte [0x7ca1d4] = a is 1. 0x101, a 0x19 to 0x21: b not 1: CoasterEdit_ToolButton (the button's tool, 2). b 1: 0x1f, 0x20 MouseMode_EnterTile15, 0x1b _EnterTile16,
// the others MouseMode_Ensure6feaa0 and the cursor; CoasterEdit_ToolButton (tool, 1); Advisor_PostId for 0x1f, 0x1b, 0x21 and for 0x20 by CoasterEdit_Tools' flag 0x200. 0x1000a / 0x1000b: Actions_FireDown / _FireUp (a, b's third byte) of [0x787c14], when that answers 0 also of [0x787294].
// 0x1001d8: up [0x7ca1cc]: as Dialog7ca1c8_Hide's end; then the advisor's five messages cancelled. Every message then goes to Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog7ca1c8_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4989d0 (cdecl; name invented): up [0x7ca1cc]: unless [0x785a2c] Dialog7cb2b8_ResetChild21 (0); up still: CoasterEdit_ToolButton (0, 4), Dialog7ca1c8_ResetMouseMode, CoasterEdit_EditRide (0, 0), up cleared, Focus_Restore; up cleared.
void __cdecl Dialog7ca1c8_Hide();

// 0x4988c0 (cdecl (thing); called from 0x487eeb and 0x4af275; name invented): nothing when up [0x7ca1cc] for that thing [0x7ca1d0] already or with the byte [0x7ca1d4] set. Else the thing kept, selected
// (Global7c2654_Set) and the focus cell updated (record 1); unless [0x785a2c] the child 21 reset (1) and posted the window [0x7ca1c8]; up set, the window focused. A thing without an open link enters the tile
// mode 0x15 (MouseMode_EnterTile15); one with: its ride edited (1, the kept thing), the mode 0x6feaa0 ensured, the cursor 0xc, selected and the focus cell updated again. Still up: the edit tools into the
// argument's slot and the checks set from it, the tool started into it and the button selected. Then the loop button updated and up set again.
void __cdecl Dialog7ca1c8_OpenFor(u8 *thing);

// 0x497f00 (cdecl; name invented): the dialog's thing [0x7ca1d0] cleared. Unless the game mode is 3: the mouse mode holder made when missing (inline as MouseModes_EnsureHolder) and, unless the current mode [0x7b05d8] answers 1 from its slot 9 (none:
// 0), a new base mode made current (the holder checked again) and GameMode_Set (0, 1) twice.
void __cdecl Dialog7ca1c8_ResetMouseMode();

// 0x498090 (cdecl (flags); name unconfirmed): in the child 0x18 of the window [0x7ca1c8] (Window_FindChild), slot 5 (bool) of the child
// 0x1f gets bit 0 of flags, 0x20 bit 1 when the byte +0x50 of [0x7ca1d0] is also not 0, 0x1b bit 2, 0x19 bit 3, 0x1a bit 4, 0x1c bit 5,
// 0x21 bit 6, 0x1e bit 7 and 0x1d bit 8. A missing child faults.
void __cdecl Dialog7ca1c8_SetChecks(u32 flags);

// 0x498a50 (cdecl (flags); name unconfirmed): Dialog7ca1c8_SetChecks when [0x7ca1c8] and [0x7ca1cc] are both not 0.
void __cdecl Dialog7ca1c8_SetChecksIfUp(u32 flags);

// 0x481a00 (thiscall (flags), pops 4; name unconfirmed; `this` unused): Dialog7ca1c8_SetChecksIfUp(flags).
void __fastcall Dialog7ca1c8_SetChecksIfUpThis(void *self, void *, u32 flags);

// 0x498a80 (cdecl; name unconfirmed): the window [0x7ca1c8] takes the focus (Window_SetFocus(0), its result returned).
u32 __cdecl Dialog7ca1c8_Focus();

// 0x4981d0 (cdecl (flags); name unconfirmed): in the child 0x18 of the window [0x7ca1c8] (Window_FindChild; a ui/radiogroup), the first
// flag of 4, 1, 2, 8, 0x10, 0x20, 0x40, 0x80, 0x100 that is set selects the button 0x1b, 0x1f, 0x20, 0x19, 0x1a, 0x1c, 0x21, 0x1e, 0x1d
// (RadioGroup_Select); those from 8 on also set the game cursor (GameCursor_Set) 0xe, 0xc, 0xd, 0xa, 0xf, 0x10. No flag: none selected (-1).
void __cdecl Dialog7ca1c8_SelectButton(u32 flags);

// 0x498a30 (cdecl (flags); name unconfirmed): Dialog7ca1c8_SelectButton when [0x7ca1c8] and [0x7ca1cc] are both not 0.
void __cdecl Dialog7ca1c8_SelectButtonIfUp(u32 flags);

// 0x4819f0 (thiscall on the object at 0x7b51f0, this not used (flags), pops 4; name unconfirmed): Dialog7ca1c8_SelectButtonIfUp(flags).
void __fastcall Dialog7ca1c8_SelectButtonIfUpThis(void *self, void *, u32 flags);

// 0x4987f0 (cdecl; name unconfirmed): the dialog's button 0x20 (in its child 0x18; Window_FindChild): when the byte +0x50 of [0x7ca1d0]
// is 0, no object +0x98 (Window_SetObject98) and its slot 5 called with 0; else, by the coaster edit tools on offer (CoasterEdit_Tools),
// its slot 5 called with 1, the object of the GUI window registered as "cb_trough" (tool 0x200; +0x90 and +0x94 0x134) or "cb_loop"
// (0x132) as its +0x98, then slot 5 called again with whether tool 2 is on offer.
void __cdecl Dialog7ca1c8_UpdateLoopButton();

// 0x498790 (cdecl; name invented): the dialog built on the GUI root [0xfaa5d8] from the template 0x751530 with the handler 0x4982a0
// (WindowScript_Build) as [0x7ca1c8], taken out of the root's children (Window_Unlink), its dword +0xa0 index 0 set to 2
// (Window_SetDwordA0) and hidden (Window_SetShown(0)); none of it checked.
void __cdecl Dialog7ca1c8_Create();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gDialog7ca1c8_7ca1d0, 0x007ca1d0, 0x4);
GAME_DATA(u8, gDialog7ca1c8_7ca1d4, 0x007ca1d4, 0x1);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x1d8], gDialog7ca1c8_Create_751530, 0x00751530, 0x1d8);   // a window script (or a text / table before one): to the next named address
