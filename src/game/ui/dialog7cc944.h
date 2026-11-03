// The dialog [0x7cc944] (script 0x758f40, handler 0x4bf9f0), the online game's list. Reimplementation of 0x4bff70; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc944Env {
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);                             // Window705a20_SetBuffer
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);                                        // Window705a20_SetCaret
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                                             // Window_SetFocus
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__fastcall *setColumn)(u8 *window, void *, u32 column, u32 value);                           // Window7059f0_SetColumnBottom
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);                                // Window7059f0_SetHighlight
    u32(__cdecl *buttonText)(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value);   // ButtonText_Init
    s32(__fastcall *add)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);      // Window7059f0_AddItem
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry);                                // Charset_FormatWide
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                                              // HoverLabel_Attach
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);                                    // Class702da0_Push
};
void Dialog7cc944_SetEnv(const Dialog7cc944Env *env);

extern "C" {

// 0x4bff70 (cdecl (value)): nothing when open. The script built on [[0xf82884] + 4] as [0x7cc944] (not checked), value in [0x7cc948]. Its child 0xe05a: a TextPart6ff4d0_Ctor part, font 6, the buffer 0x7cc908 of 0xf, the caret "hilight" (the name at 0x74fa84), slot 9 given the session's string
// (Online_Instance, Online_String14), the focus. Its list 0xe05b: rows (List7cc94c_NewRow, InitRow by address), the columns 0-2 (0) and 3-5 (1), the highlight "hilight"; its children 0x10-0x15 the button texts 0x12c-0x131 (font 8, 0xff, 0xef, 0; the values 0x17d-0x182). The row: the text
// 0x133, 0x78a374 twice and three 0, no data, at the end, refreshed. The charset [0x78ba94]'s entry 0x12b into the buffer 0x7ba058 of 0x3fff, the hover label on 0xe05c with it. [[0xf85614] + 0x14]: Class702da0_Push (0x254, 1).
void __cdecl Dialog7cc944_Open(u32 value);
}
