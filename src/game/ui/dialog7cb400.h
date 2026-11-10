// The dialog [0x7cb400] of the script 0x753f50 (an edit box on the buffer 0x7cb3b8, two radio buttons; its handler 0x4a6d00, unported): its opening. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7cb400_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cb400Env {
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);          // Class702da0_Push
    u8 *(__cdecl *create)(u8 *script, u32 handler, u32 shaded);             // Dialog_Create
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);   // Window705a20_SetBuffer
    u8 *(__fastcall *partEdit)(u8 *part, void *, u32 value);                // TextPart6ff4d0_Ctor
    u8 *(__fastcall *partText)(u8 *part, void *, u32 value);                // TextPart_Ctor
    u8 *(__fastcall *partTitle)(u8 *part, void *, u32 value);               // TextPart6ff530_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                // Window_SetImpl
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                // Window_Set4c
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    void(__fastcall *notify3000)(u8 *window, void *, u32 a, u32 b);         // Window_Notify3000
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);          // Window_NotifyC00
    void(__fastcall *notifyImpl100)(u8 *window, void *, u32 value);         // Window_NotifyImpl100
    u32(__cdecl *findByName)(const char *name);                             // GuiWindows_FindByName
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);              // Window705a20_SetCaret
    u16 *(__cdecl *wideText)(u32 index);                                    // Language_WideText
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                   // Window_SetFocus
    void(__fastcall *selectAll)(u8 *window);                                // Window705a20_SelectAll
    u16(__fastcall *selectRadio)(u8 *group, void *, u32 id);                // RadioGroup_Select
    void(__cdecl *yellow)(u8 *window);                                      // HoverLabel_SetYellow
};
void Dialog7cb400_SetEnv(const Dialog7cb400Env *env);

extern "C" {

// 0x4a6e40 (cdecl (value); name invented): unless [0x7cb3f8] is 0: the scene queue ([0xf85614] +0x14) pushed 0x23a (clear 1), the dialog [0x7cb400] made (Dialog_Create, the script 0x753f50, handler 0x4a6d00 by address, shaded), its saved rectangle read into
// a local GuiRect (not used). Its child 0x70a an edit box on the buffer 0x7cb3b8 (0x10): an 0x6ff4d0 part, font 6, 0xff x 4, Notify3000 (0, 0), the caret the "hilight" window, its +0x144 0x752588, the text 0xf3 through its slot 9, the focus, all selected;
// the dialog's dword 0 the value. The group 0x70c's children 0x70d and 0x70e: a TextPart_Ctor part, NotifyC00 (1, 1), font 6, (0, 0, 0, 0xff), no text then the text 0xf1 / 0xf2 through slot 9; the radio button 0x7a17. The child 0x709 (found for each step): an
// 0x6ff530 part, font 5, NotifyC00 (1, 1), the text 0xef, HoverLabel_SetYellow.
void __cdecl Dialog7cb400_Open(u32 value);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[368], gDialog7cb400_Open_753f50, 0x00753f50, 0x170);
