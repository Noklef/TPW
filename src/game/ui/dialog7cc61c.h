// The window at 0x7cc61c (made by 0x4bb800 from the script 0x7586f0 with the handler 0x4bb690, when the object at 0xf82884 is there).
// Reimplementation of its handler 0x4bb690. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4bb690 (cdecl; window, message, a, b): on 0x14 the global 0x7cc61c cleared, the window at +4 of the object at 0xf82884 sent (6, 1, 0)
// (Window_SendMessage) and the focus given back (Focus_Restore); on 0x15 that window sent (6, 0, 0); on 0x100 with a = -1 the window at
// 0x7cc61c sent (4, 0, 0) and 0 returned. Every other case ends in Dialog_BaseHandler (window, message, a, b), whose result it returns.
s32 __cdecl Dialog7cc61c_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4bb460 (cdecl; name invented, unconfirmed): fits the window's text into its panel. The panel is child 0x5b7f of [0x7cc61c], its scroll
// bar that one's child 0x5b80. The text: the holder [0xf7d88c] (made when none: Mem_Alloc(0x404), 0x5b50b0; none on a failed allocation, and
// the read below then faults, sic), its +0xa0 when [0x7cc620] is set, else +0xbc. It is measured (font [0x758870]) in the panel's saved
// rectangle scaled by [0xfaa5c4] / 0x800 and [0xfaa5c0] / 0x600; the height and the font's line height (slot 1, a word) are scaled back
// by 0x600 / [0xfaa5c0] (a scale of 0 faults, sic). Taller than the panel: the bar shown, its range (excess / line height, 0) (a line height
// of 0 faults, sic); else the bar hidden. When the bar's slot 7 (0) answers a word of 0 the panel's slot 9 gets the text.
void __cdecl Dialog7cc61c_FitText();
}

// The calls of Dialog7cc61c_FitText that the bench records: Mem_Alloc, the holder's constructor (0x5b50b0), GuiText_Measure (the font as dwords pushed), Language_Font,
// Window_SetShown, Window705b20_SetRange.
struct Dialog7cc61cTextEnv {
    void *(__cdecl *alloc)(u32 size);
    u8 *(__fastcall *holderCtor)(u8 *memory, void *);
    void(__cdecl *measure)(const u16 *text, const s32 *rect, u32 font, u32 hAlign, u32 vAlign, u32 fit, s32 *box);
    u8 *(__cdecl *font)(u32 index);
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);
    void(__fastcall *setRange)(u8 *window, void *, s32 from, s32 to);
};

void Dialog7cc61c_SetTextEnv(const Dialog7cc61cTextEnv *env);
