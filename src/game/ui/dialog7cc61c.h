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

// 0x4bb800 (cdecl; name invented, unconfirmed): unless [0x7cc61c] is up or [0xf82884] is 0: the dialog made (Dialog_Create: the script 0x7586f0, the handler 0x4bb690, not shaded; not checked), its depth its word +0xe0 + 4. The
// panel 0x5b7f: a new TextPart (none on a failed allocation), the font [0x758870], Window_NotifyImpl100 (0), Window_Notify3000 (1, 0), Window_NotifyC00 (0, 0); [0x7cc620] = 1, the text fitted, the handler 0x4bb720. Its radio
// group 0x5b81 selects 0x5b82. The buttons 0x5b82 and 0x5b83: a new TextPart, Window_NotifyC00 (1, 1), the font 6, the black (0, 0, 0, 0xff), Window_NotifyImpl100 (0), the text 0x10c and 0x10d in slot 9. The hover label 0x5b84 has the text 0x10b.
// The focus (0).
void __cdecl Dialog7cc61c_Open();

// 0x4bb720 (cdecl; window, message, a, b; name invented, unconfirmed): the panel's handler. Message 0x800: the panel scrolled (Window_NotifyC000 (0, -(line height * b * 3) * 0x200 / [0xfaa5c0]), the font's
// slot 1 a word; a scale of 0 faults, sic), its slot 9 the text as Dialog7cc61c_FitText reads it. Message 0x101 with b = 1: [0x7cc620] = (a is 0x5b82), the text fitted. Always ends in Window_CallDefault, whose answer it returns.
s32 __cdecl Dialog7cc61c_PanelHandler(u8 *window, u32 message, u32 a, u32 b);
}

// The calls of Dialog7cc61c_PanelHandler that the bench records.
struct Dialog7cc61cPanelEnv {
    u8 *(__cdecl *font)(u32 index);                             // Language_Font
    void(__fastcall *scroll)(u8 *window, void *, u32 a, u32 b); // Window_NotifyC000
    u8 *(__cdecl *online)();                                    // Online_Instance
    void(__cdecl *fitText)();                                   // Dialog7cc61c_FitText
};
void Dialog7cc61c_SetPanelEnv(const Dialog7cc61cPanelEnv *env);

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

// Dialog7cc61c_Open's calls (the ports by default; the bench's recorders: their own benches have them). Window_FindChild and Window_SetDepth run.
struct Dialog7cc61cOpenEnv {
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);                 // Dialog_Create
    void *(__cdecl *alloc)(u32 size);                                           // Mem_Alloc
    u8 *(__fastcall *part)(u8 *part, void *, u32 value);                        // TextPart_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                    // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                             // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                    // Window_Set4c
    void(__fastcall *notify100)(u8 *window, void *, u32 a);                     // Window_NotifyImpl100
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);             // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);              // Window_NotifyC00
    void(__cdecl *fitText)();                                                   // Dialog7cc61c_FitText
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);               // Window_SetHandler
    u16(__fastcall *select)(u8 *group, void *, u32 id);                         // RadioGroup_Select
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    u16 *(__cdecl *text)(u32 index);                                            // Language_WideText
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);                        // HoverLabel_Attach
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                       // Window_SetFocus
};
void Dialog7cc61c_SetOpenEnv(const Dialog7cc61cOpenEnv *env);

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gDialog7cc61c_758870, 0x00758870, 0x4);
