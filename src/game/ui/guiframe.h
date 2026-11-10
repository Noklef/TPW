// The GUI's work for one frame. The name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the frame calls (the benches point these at recorders): the ports; 0x47f620, 0x5f5590 (agent B's), 0x4a9180, 0x658f6c and the CRT's atexit by address.
struct GuiFrameEnv {
    void(__cdecl *begin)();                                       // 0x47f620 (agent B's)
    void(__cdecl *fireTimers)();                                  // WindowTimers_FireAll 0x6622d2
    u32(__fastcall *count)(const u8 *o);                          // Fn_669c30
    u32(__cdecl *countdownStep)();                                // Countdown7cc150_Step 0x4a9350
    void(__cdecl *countdownDone)();                               // 0x4a9180
    s32(__cdecl *pump)(s32 most);                                 // MsgQueue_Pump 0x658e42
    s32(__cdecl *fireWindowTimers)();                             // Window_FireTimers 0x658f4b
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);    // Window_PostMessage 0x658ccc
    void(__cdecl *afterPost)();                                   // 0x658f6c
    u32(__cdecl *active)();                                       // Global7b4b88_Get_47eaa0
    u32(__cdecl *clock)();                                        // GuiClock_Now 0x65968e
    u32(__cdecl *atExit)(u32 function);                           // crt_atexit 0x67a7d0
    u32(__cdecl *mouse)();                                        // Wrap_Obj_Get64Plus10_5f5640
    u32(__cdecl *mouseMove)(u32 x, u32 y);                        // Gui_MouseMove 0x6588ef
    void(__cdecl *update)(u32 thing, u32 a, u32 b, u32 c);        // 0x5f5590 (agent B's)
    s32(__cdecl *pumpSecond)(s32 most);                           // MsgQueue_PumpSecond 0x658f0b
    void(__fastcall *ime)(u8 *host);                              // ImeHost_Update 0x481100
};
// The frame's calls go to env (nullptr: the defaults).
void GuiFrame_SetEnv(const GuiFrameEnv *env);

// 0x48a580 (cdecl; name invented): 0x47f620; the window timers; the counts of the lists 0xfaa5f0, 0xfaa5f4 kept in 0x7b9a4c, 0x7b9bac; the countdown's step (0x4a9180 when it ends); up
// to 0x400 messages; the windows' timers; the window 0xfaa5d8 posted 2; 0x658f6c. Then when active: the cursor 0xfaa608's slot 6 gets the clock; the mouse (two dwords, once-only
// statics 0x7b9d00, 0x7b9d04 under the guard bit 0x7c20e4 with an atexit of 0x48a6d0) when it moved scales to 2048 by 1536 by the words 0xfaa5c4 and 0xfaa5c0 for Gui_MouseMove;
// 0x5f5590(0x7b9fe0, 0, 0, 0). Up to 0x1000 messages of the second queue; ImeHost_Update(0x7b4e38).
void __cdecl Gui_Frame();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gGui_Frame_7b9a4c, 0x007b9a4c, 0x4);
GAME_DATA(u32, gGui_Frame_7b9d00, 0x007b9d00, 0x4);
GAME_DATA(u32, gGui_Frame_7b9d04, 0x007b9d04, 0x4);
GAME_DATA(u8, gGui_Frame_7c20e4, 0x007c20e4, 0x1);
