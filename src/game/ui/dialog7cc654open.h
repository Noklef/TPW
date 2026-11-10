// The window [0x7cc654] (the mailing lists) opened. Reimplementation of 0x4bc710; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct SurfaceInfo;

// The calls (the ports by default, 0x5cc5e0 and 0x5cc600 by address; the bench's recorders: its own bench has them).
struct Dialog7cc654OpenEnv {
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);                          // Dialog_Create
    u8 *(__fastcall *textPart)(u8 *part, void *, u32 value);                             // TextPart_Ctor
    u8 *(__fastcall *rendererPart)(u8 *part, void *, u32 value);                         // RendererPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                             // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                                      // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                             // Window_Set4c
    void(__fastcall *notifyImpl100)(u8 *window, void *, u32 a);                          // Window_NotifyImpl100
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);                      // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);                       // Window_NotifyC00
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);                        // Window_SetHandler
    s32 *(__fastcall *surface)(SurfaceInfo *info, void *, s32 *out);                     // SurfaceInfo_Alloc
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *init)(u8 *));   // Window7059f0_BuildRows
    void(__fastcall *columnBottom)(u8 *window, void *, u32 column, u32 value);           // Window7059f0_SetColumnBottom
    u32(__cdecl *findByName)(const char *name);                                          // GuiWindows_FindByName
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);                   // Window7059f0_SetHighlight
    u16 *(__cdecl *wideText)(u32 index);                                                 // Language_WideText
    u32(__cdecl *buttonText)(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value);   // ButtonText_Init
    void(__cdecl *hover)(u8 *parent, u32 id, u32 text);                                  // HoverLabel_Attach
    u8(__fastcall *first)(u8 *lists);                                                    // 0x5cc5e0
    u8(__fastcall *next)(u8 *lists);                                                     // 0x5cc600
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const void *a, const void *b, const void *c);   // Charset_FormatWide
    s32(__fastcall *add)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *window);                                               // Window7059f0_RefreshRows
    u32(__fastcall *focus)(u8 *window, void *, u32 a);                                   // Window_SetFocus
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);                       // Class702da0_Push
};
void Dialog7cc654Open_SetEnv(const Dialog7cc654OpenEnv *env);   // nullptr: the game's

extern "C" {

// 0x4bc710 (cdecl, no arguments; answer 0 or 1). Answer 0 when UserSlots_Instance's +0xc4 is 0. Else [0x7cc654] the script 0x758878 (Dialog_Create, handler 0x4bbc20, shaded). Its child 0xee2: a new text part (0x1a4 bytes; none on a failed
// allocation), font 6, 0 to the part, (1, 0) to Notify3000, (0, 0) to NotifyC00, the handler 0x4bcb70. Child 0xee4: a new renderer part (0xd8 bytes). The surface 0x7cc628 allocated. The list child 0xee8: rows 0x4bcbc0 / 0x4bcca0, column bottoms
// 0, 0, 1, 1, the highlight of the name 0x74fa84, its children 0x10-0x13 the texts 0x136-0x139, font 8, colour ff ef 00, values 0x187-0x18a. The hover label 0xee9 the text 0x135. One row for each list of UserSlots_Instance +0x64
// (0x5cc5e0, 0x5cc600): the name +0x24 (null the empty one) as wide (String_AnsiToWide's fixed buffer), the text 0x1c0 (0x1f cells) on the three numbers +0x4c, +0x50, +0x54 of it (arguments number 0x10, 0x11, 0x12, in that order, with the third
// first on the stack: sic), a 1 and a 0, data the row's index with bit 24. The list refreshed, the window given the focus (0), [[0xf85614] + 0x14] pushed (0x255, 1). Answer 1.
s32 __cdecl Dialog7cc654_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[624], gDialog7cc654_Open_758878, 0x00758878, 0x270);
