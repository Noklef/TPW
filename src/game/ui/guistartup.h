// The game's display and GUI brought up and taken down. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// What the bringing up calls (the benches point these at recorders): the ports; the unported by address (0x5f55c0, 0x5f5600, 0x47e7d0, 0x4891d0, 0x4893c0 agent B's or skipped).
struct GuiStartupEnv {
    s32 *(__cdecl *screenOpen)(s32 *out, const void *guid, s32 exclusive, void *window, void *listWindow);   // Screen_Open 0x5fa1e0
    void(__cdecl *setDirectDraw)(void *directDraw);                                                          // Surface_SetDirectDraw 0x5f8f80
    void(__cdecl *setWindow)(void *window);                                                                  // App_SetWindow 0x5f86d0
    void(__cdecl *fn5f55c0)(void *local);                                                                    // 0x5f55c0
    void(__cdecl *fn5f5600)(void *local);                                                                  // 0x5f5600
    void(__cdecl *fn47e7d0)();                                                                                 // 0x47e7d0
    u8 *(__fastcall *fontInit)(u8 *font, void *, u32 count, u32 width, u32 spacing, u32 height, u32 b);      // GuiFont_Init 0x658243
    u32(__fastcall *isHebrew)(const u8 *language);                                                           // Language_IsHebrew 0x41a850
    u32(__cdecl *guiInit)(u32 unused, u32 capacity, u32 flag);                                               // Gui_Init 0x6584df
    void(__cdecl *fn659674)(u32 palette);                                                                  // 0x659674
    u32(__cdecl *fn6590ea)(u32 handler);                                                                   // Fn_6590ea
    u32(__cdecl *poolInit)(s32 longLength, s32 longRows, s32 shortLength, s32 shortRows);                    // TextPool_Init 0x669440
    u32(__cdecl *outOne)(u32 a, u32 b, u32 c, u32 *out);                                                     // OutOne_44e600
    u16(__fastcall *tableInit)(u8 *table, void *, u32 count);                                                // CursorTable_Init 0x661cca
    void(__cdecl *loadAni)();                                                                                // GameCursors_LoadAni 0x489720
    void(__cdecl *loadTga)();                                                                                // GameCursors_LoadTga 0x4899c0
    void(__cdecl *setTable)(u8 *table);                                                                      // Gui_SetCursorTable 0x659248
    void(__cdecl *fn6591d4)(u32 shown);                                                                   // 0x6591d4
    void(__cdecl *setCursor)(u32 id);                                                                        // Gui_SetCursor 0x6590f7
    void(__cdecl *fn6591e3)();                                                                          // 0x6591e3
    void(__cdecl *fn4891d0)();                                                                             // 0x4891d0
    void(__cdecl *fn4893c0)();                                                                               // 0x4893c0
    void(__fastcall *setLayout)(u8 *part, void *, u32 flags, const u8 *info);                                // TextPart_SetLayout 0x48fc40
    void(__cdecl *fn492470)();                                                                              // 0x492470
};
// What the taking down calls: the same.
struct GuiTeardownEnv {
    void(__fastcall *tableClear)(u8 *table);     // CursorTable_Clear 0x661c36
    void(__cdecl *fn5f5620)();                  // 0x5f5620
    void(__cdecl *fn5f55e0)();                    // 0x5f55e0
    void(__cdecl *screenClose)();                // Screen_Close 0x5fa060
    void(__cdecl *listenEnd)();                  // Global7b4b84_Delete 0x47e990
    u32(__cdecl *guiShutdown)();                 // Gui_Shutdown 0x658708
    void(__cdecl *poolFree)();                   // TextPool_Free 0x669586
    void(__fastcall *fn657dff)(u8 *font);        // 0x657dff
    u32(__cdecl *fn424ec0)(u32 value);              // Const1_424ec0
};
// The two's calls go to the envs (nullptr: the defaults).
void GuiStartup_SetEnv(const GuiStartupEnv *startup, const GuiTeardownEnv *teardown);

// 0x489ca0 (cdecl; name invented): the screen opened, its DirectDraw 0x879a24's first dword given to Surface; with the flag bit 0x800 of 0x7a1a8c and no 0x788270 the window 0x7a130c
// set, 0x5f55c0, 0x5f5600 on a local (their failure only Debug_Printf, left out with it), the 5 dwords 0x7b9fe0 cleared with +0x10 0x486c70, 0x47e7d0. The font 0x786de8 sized by the
// screen's words 0xfaa5c4 and 0xfaa5c0; Gui_Init(0x80, 0x800, Hebrew or not); 0x659674(0x7b9864), Fn_6590ea(0x485780), the text pool, OutOne_44e600 into 0x7c249c, the import
// 0x6fd30c (0); the two cursor tables (0x16 each, loaded), the one set by the same test; the cursor; 0x4891d0, 0x4893c0; the text part 0x7c2118's +0x10 word 1 and its layout (flags
// 0xc00, the local info: +0x10 the local inner: +4, +5 1, +9 0, +0x10 to +0x12 0xff, the rest not set); 0x492470; the import Dialup::Initialise of 0x7c24c0 into 0x7c24d4.
void __cdecl Gui_Startup();

// 0x489ec0 (cdecl; name invented): both cursor tables cleared; 0x5f5620, 0x5f55e0, the screen closed, Global7b4b84_Delete, Gui_Shutdown; the objects 0x7b9a04, 0x7c231c, 0x7b9804 deleted
// (slot 0 (1)) when there; the text pool freed, 0x657dff on the font, Const1_424ec0([0x7c249c]); the import 0x6fd5b0 of 0x7c24c0 tail-called, its EAX.
u32 __cdecl Gui_Teardown();
}
