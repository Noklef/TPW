// The dialog [0x7cdac4] of the script 0x7595b0 (two edit boxes on the buffers 0x7cd850 and 0x7cd480; its handler 0x4c4480, unported, posts them): reimplementation of its opening. The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

struct String;

// Dialog7cdac4_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cdac4Env {
    u32(__cdecl *closeWindow)();                                          // Window7c24c8_Close
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);           // Dialog_Create
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);            // Window_SetDepth
    u8 *(__fastcall *partCtor)(u8 *part, void *, u32 value);              // TextPart6ff4d0_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);              // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                       // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);              // Window_Set4c
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size); // Window705a20_SetBuffer
    u32(__cdecl *findByName)(const char *name);                           // GuiWindows_FindByName
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);            // Window705a20_SetCaret
    void(__fastcall *setTheme)(u8 *self, void *, const String *theme, u8 *label);   // NamedObjects_SetThemeLabelAnsi
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);       // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);        // Window_NotifyC00
    u16 *(__cdecl *wideText)(u32 index);                                  // Language_WideText
    void(__cdecl *attachLabel)(u8 *parent, u32 id, u32 text);             // HoverLabel_Attach
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);        // Class702da0_Push
};
void Dialog7cdac4_SetEnv(const Dialog7cdac4Env *env);

extern "C" {

// 0x4c4670 (cdecl; name invented): Window7c24c8_Close; the dialog made ([0x7cdac4]) one depth step (+0xe0's word + 4) up. Child 0x11e4 (not checked): a TextPart6ff4d0 part (0x1a4 bytes; none when that fails), the font 7, the buffer 0x7cd850 (0xf), the caret window
// 0x74fa84 and, in slot 9, the theme's label as a local wide string (NamedObjects_SetThemeLabelAnsi on 0x786b68, itself the theme: Obj_Self, inline). Child 0x11e3 likewise: the bytes 1, 0 as 0x3000 and 0, 0 as 0xc00, the buffer 0x7cd480 (0x7e), the caret, the text
// 0xdc. HoverLabel_Attach (0x11e5, the text 0xda); the scene queue [[0xf85614]+0x14] given 0x250 with clear 1.
void __cdecl Dialog7cdac4_Open();
}
