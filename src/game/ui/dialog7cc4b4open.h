// The dialog [0x7cc4b4] (script 0x757f60, handler 0x4b8b70) built. Reimplementation of 0x4b8ca0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, this range's unported 0x4bbbd0 by address; tests/fncmp/dialog7cc4b4open.py gives recorders).
struct Dialog7cc4b4OpenEnv {
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);       // WindowScript_Build
    u8 *(__fastcall *textPart)(u8 *part, void *, u32 value);                    // TextPart_Ctor
    u8 *(__fastcall *textPart530)(u8 *part, void *, u32 value);                 // TextPart6ff530_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                    // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                             // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                    // Window_Set4c
    void(__fastcall *notify100)(u8 *window, void *, u32 a);                     // Window_NotifyImpl100
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);              // Window_NotifyC00
    void(__fastcall *notify1)(u8 *window, void *, u32 value);                   // Window_Notify1
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                   // Window_SetShown
    void(__cdecl *update)();                                                    // Dialog7cc4b4_Update
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    void(__cdecl *finish)();                                                    // 0x4bbbd0
};
void Dialog7cc4b4Open_SetEnv(const Dialog7cc4b4OpenEnv *env);   // nullptr: the game's

extern "C" {

// 0x4b8ca0 (cdecl, no arguments; SEH dropped): nothing with [0x7cc4b4] there or [[0xf82884] + 4] none. Built on that into [0x7cc4b4]; its child 0x1e0ec's 0x1e0ee gets a text part (0x1a4 bytes, none when refused; value 0), font 2, 100 (0), C00 (2, 1); that child's 0x1e0ed 1 (3), the child hidden; child 0x1e0ea 1 (1), its 0x1e0eb 1 (5); [0x7cc4b8] = [0x7cc4bc] = 0; Dialog7cc4b4_Update;
// child 0x1e0ef gets a 0x6ff530 text part likewise, font 0, C00 (1, 1), slot 9 (0x75817c), 40000 (0xfc, 0xe8, 7, 0xff); 0x4bbbd0.
void __cdecl Dialog7cc4b4_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8 *, gDialog7cc4b4open_7cc4b4, 0x007cc4b4, 0x4);
GAME_DATA(s32, gDialog7cc4b4_7cc4b8, 0x007cc4b8, 0x4);
