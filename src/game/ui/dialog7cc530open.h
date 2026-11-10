// The online dialog's opening: its dial-up page built. Reimplementation of 0x4ba980; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc530OpenEnv {
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);                                       // Dialog_Create
    void(__cdecl *yellow)(u8 *window);                                                                // HoverLabel_SetYellow
    u16(__fastcall *select)(u8 *group, void *, u32 id);                                               // RadioGroup_Select
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__fastcall *setColumn)(u8 *window, void *, u32 column, u32 value);                           // Window7059f0_SetColumnBottom
    u32(__cdecl *findByName)(const char *name);                                                       // GuiWindows_FindByName
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);                                // Window7059f0_SetHighlight
    u32(__cdecl *buttonText)(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value);   // ButtonText_Init
    void(__cdecl *fill)(u8 shown);                                                                    // DialupList_Fill
    u16 *(__cdecl *wideText)(u32 id);                                                                 // Language_WideText
};
void Dialog7cc530_SetOpenEnv(const Dialog7cc530OpenEnv *env);

extern "C" {

// 0x4ba980 (cdecl (value), SEH dropped): value in [0x7cc52c], the dialog of the script at 0x758218 and handler 0x4ba530 (not shaded) in [0x7cc530], the bytes
// [0x7cc53c] and [0x7cc540] 0. Its child 0xb: a TextPart6ff530 (0 when the allocation fails), font 5, centred, the text 0xe1, the yellow hover labels. Its group 0xd's
// buttons 0xe and 0xf: a TextPart, centred both ways, font 6, white, the texts 0xe5 and 0xe4; 0xe selected. Its list 0xc: rows (List7cc530_NewRow, InitRow by
// address), the columns 0 (0) and 1 (1), the highlight named at 0x74fa84, its headers 0x10 and 0x11 the texts 0xe2 and 0xe3 (font 8, 0xff, 0xef, 0, -1). Then the list filled, not shown.
void __cdecl Dialog7cc530_Open(u32 value);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7cc530open_7cc52c, 0x007cc52c, 0x4);
GAME_DATA(u8 *, gDialog7cc530open_7cc530, 0x007cc530, 0x4);
GAME_DATA(u8, gDialog7cc530open_7cc540, 0x007cc540, 0x1);
