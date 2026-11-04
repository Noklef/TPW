// The front end's entry points: a jump, or a short forward, to the function that does the work. Each answers what that function answers.
// Reimplementation of 0x4813b0-0x4813e0, 0x481420-0x481460, 0x4814f0, 0x481510, 0x4816d0, 0x481750-0x481780, 0x4817c0, 0x4817e0, 0x4819e0, 0x481b50, 0x481b60, 0x48a700, 0x48a720, 0x498780, 0x4b6490,
// 0x4b6b80, 0x4b7a20, 0x4b8ea0, 0x4bf9d0, 0x4cf3d0 and 0x4da5a0. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The functions they reach (the game's by default, whether ported or not; the bench's recorders: their own benches have them).
struct FrontEndEntriesEnv {
    using Plain = u32(__cdecl *)();
    using One = u32(__cdecl *)(u32 value);
    using Method = u32(__fastcall *)(u8 *self);
    Plain guiFrame, guiStartup, guiTeardown, panelOpen, clearGlobals, panelShowWindow, panelExit, openOnGui, restartTimers, closeTimers, toggle, resumeTimer;
    Plain dialog7ca2e8, dialog7cb25c, dialog7cc2ec, dialog7ca210, research, hide7ca1c8, ensureMode, signDestroy, previewHide, openShown, child7cc4b4;
    One mouseHelp, panelMenu, browserOpen, open74fb20, open74faf0, fillList;
    Method other5f0b40, message4, gatesStep, tickSpeed;
    u32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);   // Window_SendMessage
};
void FrontEndEntries_SetEnv(const FrontEndEntriesEnv *env);

extern "C" {

u32 __cdecl FrontEnd_GuiFrame();             // 0x4813b0: jumps to Gui_Frame
u32 __cdecl FrontEnd_GuiStartup();           // 0x4813c0: jumps to Gui_Startup
u32 __cdecl FrontEnd_GuiTeardown();          // 0x4813d0: jumps to Gui_Teardown
u32 __cdecl FrontEnd_OpenPanels();           // 0x4813e0: Panel7b9940_Open, then jumps to Globals7c264c_Clear
u32 __cdecl FrontEnd_ShowWindow();           // 0x481420: jumps to Panel7b9940_ShowWindow
u32 __cdecl FrontEnd_ExitPanel();            // 0x481430: jumps to Panel7b9940_Exit
u32 __cdecl FrontEnd_OpenOnGui();            // 0x481440: jumps to Window7cb2cc_OpenOnGui
u32 __cdecl FrontEnd_Pause();                // 0x481450: jumps to FrontEnd_PauseTimers
u32 __cdecl FrontEnd_Resume();               // 0x481460: jumps to FrontEnd_ResumeTimers
u32 __stdcall FrontEnd_MouseHelp(u32 forced);   // 0x4814f0: MouseHelp_Update, pops 4
u32 __cdecl FrontEnd_Other5f0b40();          // 0x481510: jumps to 0x5f0b40 on 0xf86db8
u32 __cdecl FrontEnd_PanelMenu();            // 0x4816d0: Panel7c2534_Open(0)
u32 __cdecl FrontEnd_Dialog7ca2e8();         // 0x481750: jumps to Dialog7ca2e8_Open
u32 __cdecl FrontEnd_Dialog7cb25c();         // 0x481760: jumps to Dialog7cb25c_Open
u32 __cdecl FrontEnd_Dialog7cc2ec();         // 0x481770: jumps to Dialog7cc2ec_Open
u32 __cdecl FrontEnd_Dialog7ca210();         // 0x481780: jumps to Dialog7ca210_Open
u32 __cdecl FrontEnd_Browser();              // 0x4817c0: with [0x7cc418] ThingBrowser_Open([0x7c2658]), else jumps to 0x4b4280
u32 __cdecl FrontEnd_Research();             // 0x4817e0: jumps to ResearchPanel_Open
u32 __cdecl FrontEnd_Hide7ca1c8();           // 0x4819e0: jumps to Dialog7ca1c8_Hide
u32 __stdcall FrontEnd_Open74fb20(u32 thing);   // 0x481b50: Dialog74fb20_OpenFor, pops 4
u32 __stdcall FrontEnd_Open74faf0(u32 thing);   // 0x481b60: Dialog74faf0_OpenFor, pops 4
u32 __cdecl FrontEnd_PauseTimers();          // 0x48a700: with a root [0xfaa5d8] WindowTimers_RestartAll, then jumps to Window7cb2cc_Close; without, the root's 0
u32 __cdecl FrontEnd_ResumeTimers();         // 0x48a720: Window7cb2cc_Toggle, then with [0x7cb2cc] set jumps to Fn_662411, else to WindowTimers_RestartAll
u32 __cdecl FrontEnd_EnsureMode();           // 0x498780: jumps to MouseMode_Ensure6feaa0
u32 __cdecl FrontEnd_SignDestroy();          // 0x4b6490: jumps to Sign_Destroy
u32 __cdecl FrontEnd_PreviewHide();          // 0x4b6b80: jumps to ThingPreview_Hide
u32 __fastcall FrontEnd_Message4(u8 *object);   // 0x4b7a20: jumps to WindowAt10_Message4
u32 __cdecl FrontEnd_Dialog7cc4b4Child();    // 0x4b8ea0: message 6 (1, 0) to [0x7cc4b4] (Window_SendMessage), then jumps to DialogF82884_Child
u32 __cdecl FrontEnd_ReplyList(u32 a, u32 b, u32 reply);   // 0x4bf9d0: a when it is set, else Dialog7cc944_FillList(reply)
u32 __fastcall FrontEnd_GatesStep(u8 *gates);   // 0x4cf3d0: jumps to ParkGates_Step
u32 __fastcall FrontEnd_TickSpeed(u8 *person);  // 0x4da5a0: jumps to Person_TickSpeed
}
