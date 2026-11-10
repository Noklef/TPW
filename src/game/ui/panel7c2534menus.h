// The panel [0x7c2534]'s two menus built, and the choice between them. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the builders call (the benches point these at recorders): the ports; 0x492e80, 0x5b50b0 (agent B's) and the CRT's atexit by address.
struct Panel7c2534MenusEnv {
    u8 *(__fastcall *ctor)(u8 *object);                                                                                   // Class492d80_Ctor 0x492e40
    void(__fastcall *setHandlers)(u8 *panel, void *, u32 handler, u32 second);                                            // 0x492e80
    u8 *(__fastcall *readMode)(u8 *object);                                                                               // Setting_ReadMode 0x550ca0
    u32(__cdecl *atExit)(u32 function);                                                                                   // crt_atexit 0x67a7d0
    const u16 *(__cdecl *wideText)(u32 id);                                                                               // Language_WideText 0x485b00
    s32(__fastcall *addRow)(u8 *self, void *, s32 top, const u16 *text, u32 id, u32 font, u32 red, u32 green, u32 blue);  // Panel7c2534_AddRow 0x492f60
    u8 *(__fastcall *onlineCtor)(u8 *block);                                                                              // 0x5b50b0
    u32(__fastcall *loggedIn)(const u8 *session);                                                                         // Online_LoggedIn 0x5b6450
    u32(__fastcall *hasActivity)(const u8 *object);                                                                       // Obj_HasActivity 0x5b6230
};
// The builders' calls go to env (nullptr: the defaults).
void Panel7c2534Menus_SetEnv(const Panel7c2534MenusEnv *env);

// What the choice calls: the ports; 0x4092a0 (agent B's) by address.
struct Panel7c2534OpenEnv {
    void(__cdecl *buildMain)();                                     // Panel7c2534_BuildMain 0x48c150
    void(__cdecl *buildOnline)();                                   // Panel7c2534_BuildOnline 0x48c600
    void(__cdecl *resetChild)(u32 clear27);                         // Dialog7cb2b8_ResetChild21 0x4a25f0
    void(__fastcall *leave)(u8 *object, void *, u32 a, u32 b);      // 0x4092a0
    void(__cdecl *refresh)();                                       // Fn_662411
    void(__fastcall *show)(u8 *object, void *, u32 flag);           // WindowAt10_Show 0x493130
};
// The choice's calls go to env (nullptr: the defaults).
void Panel7c2534Open_SetEnv(const Panel7c2534OpenEnv *env);

// 0x48c150 (cdecl; name invented): a new panel in 0x7c2534 (handlers 0x48b6a0, 0x48b2a0). Grey rows (0x80), each 5 below the last from 0xa: for the setting 0xfb3b7c 1 (read once, under
// the guard bit 0xfb34c6) 7/0x10, 0xc/0x13, 8/0x14 (text/id); for the game's mode 4 3/0x16, 0xa/0x17, 7/0x15, 0xc/0x18, 8/0x19. Else the gap 5 (0 when logged in), the first at twice it:
// 3/1, 4/2, 0xa/3, 5/4, logged in 2/7, without activity 6/5, then 7/0, 0xc/6, 8/8. The online holder 0xf7d88c made when none before each use.
void __cdecl Panel7c2534_BuildMain();

// 0x48c600 (cdecl; name invented): a new panel in 0x7c2534 (handlers 0x48bd40, 0x48b2a0). Black rows, 5 below the last from 5: with the holder's +0x108 0xe0/0xd; else logged in 2/0xa or
// out 1/9, without activity 6/0xb, with the object [0xf85614]+4's slot 9 answering 0xd/0xc. Then 7/0xe, 8/0xf.
void __cdecl Panel7c2534_BuildOnline();

// 0x48c830 (cdecl (flag); name invented): nothing with a panel. With the flag the online menu; without, the main one and Dialog7cb2b8_ResetChild21(1), and with 0x786ba4 1 that left
// (0x4092a0 of 0x786b68 (0, 0)), 0x786ba4 and the byte 0x7c2518 0 and Fn_662411, shown. Else the byte 1 and shown (WindowAt10_Show(flag)). EAX not kept.
void __cdecl Panel7c2534_Open(u32 flag);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gPanel7c2534menus_7c2518, 0x007c2518, 0x1);
GAME_DATA(u8 *, gPanel7c2534menus_7c2534, 0x007c2534, 0x4);
GAME_DATA(u8, gPanel7c2534_BuildMain_fb34c6, 0x00fb34c6, 0x1);
