// The dialog [0x7cdac4] of the script 0x7595b0 (two edit boxes on the buffers 0x7cd850 and 0x7cd480; its handler 0x4c4480 posts them): reimplementation of its opening and handler. The names are invented, unconfirmed.
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

// Dialog7cdac4_Handler's calls.
struct Dialog7cdac4HandlerEnv {
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);            // Window_SendMessage
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);            // Window_PostMessage
    void(__cdecl *keysEnable)();                                          // KeyMaps_EnableAllAndClearFlag
    void(__cdecl *keysReset)();                                           // KeyMaps_ResetAllAndSetFlag
    u32(__cdecl *focusRestore)();                                         // Focus_Restore
    s32(__cdecl *baseHandler)(u8 *window, u32 message, u32 a, u32 b);     // Dialog_BaseHandler
    u8 *(__fastcall *readMode)(u8 *setting);                              // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *fn)());                            // crt_atexit
    u8 *(__cdecl *chat)();                                                // Online_Instance
    void(__fastcall *chatSend)(u8 *chat, void *, u32 a, u32 b, u32 first, u32 second, u32 modeTwo, u32 callback, u32 zero);   // 0x5b7ba0, unported (agent B's range)
    void(__fastcall *clearQueue)(u8 *object);                             // Class702da0_Clear
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                 // Window_SetFocus
};
void Dialog7cdac4_SetHandlerEnv(const Dialog7cdac4HandlerEnv *env);

extern "C" {

// 0x4c4670 (cdecl; name invented): Window7c24c8_Close; the dialog made ([0x7cdac4]) one depth step (+0xe0's word + 4) up. Child 0x11e4 (not checked): a TextPart6ff4d0 part (0x1a4 bytes; none when that fails), the font 7, the buffer 0x7cd850 (0xf), the caret window
// 0x74fa84 and, in slot 9, the theme's label as a local wide string (NamedObjects_SetThemeLabelAnsi on 0x786b68, itself the theme: Obj_Self, inline). Child 0x11e3 likewise: the bytes 1, 0 as 0x3000 and 0, 0 as 0xc00, the buffer 0x7cd480 (0x7e), the caret, the text
// 0xdc. HoverLabel_Attach (0x11e5, the text 0xda); the scene queue [[0xf85614]+0x14] given 0x250 with clear 1.
void __cdecl Dialog7cdac4_Open();

// 0x4c4480 (cdecl (window, message, a, b); name invented): 0x14: [0x7cdac4] = 0, KeyMaps_EnableAllAndClearFlag, [[0xf82884]+4] sent 6 (1, 0), Focus_Restore. 0x15: that window sent 6 (0, 0), KeyMaps_ResetAllAndSetFlag. 0x100, a -1: the mode setting 0xfb3b7c read once (the guard 0xfb34bb & 1: Setting_ReadMode, atexit 0x5d1750),
// 0x5b7ba0 on 0x48b390's object with [0xf82840], [[0xf82888]+8], the buffers 0x7cd850 and 0x7cd480, whether the setting is 2, 0x4c4470 and 0, then the scene queue [[0xf85614]+0x14] cleared and the dialog [0x7cdac4] sent 4 (0, 0): 0 answered. a -2: the queue cleared and sent 4: 0 answered.
// 0x802, a 0x11e4: the child 0x11e3 takes the focus. 0x803: the child 0x11e3 (a 0x11e4) or 0x11e4 takes it. 0x804: the window posted 4 (0, 0). All but those answered 0 go on to Dialog_BaseHandler, whose answer is returned.
s32 __cdecl Dialog7cdac4_Handler(u8 *window, u32 message, u32 a, u32 b);
}
