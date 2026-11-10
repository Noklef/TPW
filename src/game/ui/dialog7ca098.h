// The message box [0x7ca098] (made by Dialog7ca098_Create): its handler and the actions its buttons run: reimplementation of 0x486b40, 0x48e0f0 and 0x48e2f0. The names are invented.
#pragma once

#include "../../../re/types.h"

// Their calls (the ports by default; 0x49fb30, 0x4aa480, 0x4b2750 and 0x498d80 raw: unported; the bench's recorders: their own benches have them).
struct Dialog7ca098Env {
    void(__cdecl *centreOnThing)(u32 handle);                                // Camera_CentreOnThing
    void(__cdecl *centreOnCell)(const u16 *cell);                            // Camera_CentreOnCell
    void(__cdecl *openList)(u32 kind, u32 selected);                         // Dialog7cc1f8_Open
    void(__cdecl *openTab)(u32 tab);                                         // Dialog7ca308_Open
    void(__cdecl *open49fb30)();
    void(__cdecl *open4aa480)();
    void(__cdecl *open4a52a0)();                                             // Dialog7cb398_Create
    void(__cdecl *open4b2750)();
    void(__cdecl *open4b7b00)();                                             // Dialog7cc444_Create
    void(__cdecl *open498d80)();
    u32(__fastcall *setFlag64)(u8 *object, void *, u32 on);                  // Object_SetFlag64
    void(__fastcall *accept)(u8 *challenges);                                // Challenges_AcceptOffer
    void(__fastcall *decline)(u8 *challenges);                               // Challenges_DeclineOffer
    void(__fastcall *notify)(u8 *window, void *, u32 a);                     // Window_NotifyImpl100
    void(__cdecl *setButton)(u8 *window, u32 kind);                          // MsgBox_SetButton
    void(__fastcall *setObject)(u8 *window, void *, u8 *object);             // Window_SetObject98
    u32(__cdecl *findByName)(const char *name);                              // GuiWindows_FindByName
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);               // Window_SetDepth
    u16(__fastcall *runScript)(u8 *window, void *, u8 *script, s32 interval);   // Window_RunScript
    u16(__fastcall *stopScript)(u8 *window);                                 // Window_StopScript
    u8(__fastcall *cancelAdvisor)(u8 *advisor, void *, u32 id);          // Advisor_Cancel
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);               // Window_PostMessage
    void(__cdecl *resetChild21)(u32 clear27);                                // Dialog7cb2b8_ResetChild21
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Dialog7ca098_SetEnv(const Dialog7ca098Env *env);

extern "C" {

// 0x486b40 (cdecl (id)): Advisor_Cancel (id) on the game's advisor (GameTable7cfb90_Get1da71c_5194d0 of [0x7cf83c]).
void __cdecl Game_CancelAdvisor(u32 id);

// 0x48e0f0 (cdecl (kind, a, b)): a button's action. 3: Camera_CentreOnThing (a), answer 0. 4: Camera_CentreOnCell (&b's low word), 0. 9, 0xa, 0xb, 0xc: Dialog7cc1f8_Open (0, 1, 2, 3, b), 2. 0xd to 0x11: Dialog7ca308_Open (0 to 4), 2. 0x12 to 0x16 and 0x18: 0x49fb30, 0x4aa480, Dialog7cb398_Create, 0x4b2750, Dialog7cc444_Create, 0x498d80, 2.
// 0x17: Object_SetFlag64 (1) on the object of the table 0x7cfb90 at a's low word, 1 (its assert left out). 0x1a, 0x1b: Challenges_AcceptOffer, _DeclineOffer on the game's (GameTable7cfb90_Get1da736_519610), 2. Else 0.
u32 __cdecl Dialog7ca098_RunAction(u32 kind, u32 a, u32 b);

// 0x48e2f0 (cdecl (window, message, a, b)): 0x1d, a 1: [0x7ca094] = 0. 0x1001d8: [0x7ca090]'s script stopped when there is one, Game_CancelAdvisor (0x11c), [0x7ca090] = 0. 0x1001d9: when the box has its child 0x10d32: that notified (the window's dword 1); then, not checked, the buttons 0x10d30 and 0x10d2f set (MsgBox_SetButton, the box's dwords 3, 4), the button 0x10d2e given no object
// and hidden (slot 5 (0)) when the dword 4 is 0x1b; else "mb_erase" (0x750614) and shown; each of the three one depth up. Then [0x7ca090] = [0x7ca094] whose script 0x7502b8 runs at 100. 0x100: a 0x10d30, 0x10d2f: the action of the window's dwords 3 (4 for the second), 2, 5 (the dwords read 2, 5, then the first): 1: below; 2: [0x7cb2c4] sent 0x100002 ([0x7ca090] as it was, 0) and below; else nothing.
// a 0x10d2e with [0x7ca090]: its slot 5 (0), [0x7cb2c4] posted 0x100002 (it, 0), below. Below: with [0x7ca090] (read again), Dialog7cb2b8_ResetChild21 (0). Every message then goes to Window_CallDefault, whose answer is returned.
s32 __cdecl Dialog7ca098_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[12], gDialog7ca098_750614, 0x00750614, 0xc);
GAME_DATA(u8 *, gDialog7ca098_7ca090, 0x007ca090, 0x4);
GAME_DATA(u8 *, gDialog7ca098_7ca094, 0x007ca094, 0x4);
