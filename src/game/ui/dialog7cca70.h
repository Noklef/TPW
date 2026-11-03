// The dialog [0x7cca70] (script 0x7593e8, handler 0x4c1a10), an online game's list. Reimplementation of 0x4c2110; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cca70Env {
    void(__fastcall *request)(u8 *object, void *, u32 a, u32 callback, u32 b);                        // 0x5bcd10 (agent B's)
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                                             // Window_SetFocus
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__fastcall *setColumn)(u8 *window, void *, u32 column, u32 value);                           // Window7059f0_SetColumnBottom
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);                                // Window7059f0_SetHighlight
    u32(__cdecl *buttonText)(u8 *window, u32 text, u32 font, u32 red, u32 green, u32 blue, u32 value);   // ButtonText_Init
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *arg);                // Charset_FormatWide
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                                              // HoverLabel_Attach
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);                                    // Class702da0_Push
};
void Dialog7cca70_SetEnv(const Dialog7cca70Env *env);

extern "C" {

// 0x4c2110 (cdecl (value)): the script built on [[0xf82884] + 4] as [0x7cca70] (not checked and never looked at first), value in [0x7cca8c]. Its list 0x14beb: rows (List7cca90_NewRow, InitRow by address), the columns 0, 1 (0) and 2-4 (1; 5 is left), the highlight "hilight" (the name at 0x74fa84); its
// children 0x10-0x14 the button texts 0x126-0x12a (font 8, 0xff, 0xef, 0; the values 0x177-0x17b). The session's +0xe8: 0x5bcd10 ([[0xf82888] + 8], the callback 0x4c20c0, 0). The charset [0x78ba94]'s entry 0x1bf into the buffer 0x7ba058 of 0x3fff with the argument {0x6ff174, 'MRAV', 0xf,
// [[0xf82888] + 0x14]}; the hover label on 0x14bec with it; the focus; [[0xf85614] + 0x14]: Class702da0_Push (0x253, 1).
void __cdecl Dialog7cca70_Open(u32 value);
}
