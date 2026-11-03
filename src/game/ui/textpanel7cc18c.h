// The text [0x7cbf48] drawn on the panel [0x7cc18c] and shrunk into a window's image: reimplementation of 0x4a84c0. The name is invented.
#pragma once

#include "../../../re/types.h"

// Its calls (the ports by default; the bench's recorders: their own benches have them).
struct TextPanel7cc18cEnv {
    u32(__fastcall *open)(u8 *window, void *, u32 a, u32 b);                // Window_ImplSlot7
    void(__fastcall *reset)(u8 *panel);                                     // Panel_Reset
    void(__cdecl *draw)(const u16 *text, u32 a, u32 c, u32 d, s32 left, s32 top, s32 right, s32 bottom, u32 rgb);   // GuiText_DrawRgb
    u8(__fastcall *shrink)(u8 *panel, void *, u8 *dest, s32 width, s32 height, u32 format);   // Panel_ShrinkInto
    u32(__fastcall *close)(u8 *window);                                     // Window_ImplSlot6
};
void TextPanel7cc18c_SetEnv(const TextPanel7cc18cEnv *env);

extern "C" {

// 0x4a84c0 (cdecl (window); called by 0x4a7f2e and 0x4a8070): the window's image opened (Window_ImplSlot7, width and height out). With an image and a panel: the panel reset, the text drawn on it in the rectangle
// 0x142, 0x5e - 0x27c, 0xd7 in 0xff0000cc (its pixels +0x5c, +0x50, +0x54), shrunk into the image for the mode [0x8bd554] 2 (format 2) or 3 (format 1), nothing else for other modes, and the image closed
// (Window_ImplSlot6). Without either it is not closed.
void __cdecl TextPanel7cc18c_Show(u8 *window);
}
