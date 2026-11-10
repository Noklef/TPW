// The dialog [0x7cb3f8] of the four user slots: its opening and its handlers. Reimplementation of 0x4a5fc0, 0x4a6000, 0x4a61b0, 0x4a61d0, 0x4a6290 and 0x4a6580; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports and agent B's by default; the bench's recorders).
struct Dialog7cb3f8Env {
    void(__cdecl *resetKeys)();                                                  // KeyMaps_ResetAll
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                    // Window_SetShown
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);        // WindowScript_Build
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                        // Window_SetFocus
    u8 *(__fastcall *child)(u8 *window, void *, u32 id);                         // Window_FindChild
    void *(__cdecl *alloc)(u32 size);                                            // Mem_Alloc
    u8 *(__fastcall *textPart)(u8 *part, void *, u32 value);                     // TextPart6ff530_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                     // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                              // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                     // Window_Set4c
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);               // Window_NotifyC00
    void(__cdecl *yellow)(u8 *window);                                           // HoverLabel_SetYellow
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    u32(__fastcall *setDwordA0)(u8 *window, void *, u32 index, u32 value);       // Window_SetDwordA0
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);                // Window_SetHandler
    u16 *(__cdecl *text)(u32 index);                                             // Language_WideText
    void(__cdecl *refresh)();                                                    // Dialog7cb3f8_RefreshSlots
    u8 *(__fastcall *records)(u8 *memory);                                       // 0x5c7420 (agent B's)
    u32(__fastcall *countUsed)(const u8 *records);                               // Records24_CountUsed
    void(__fastcall *push)(u8 *object, void *, u32 id, u32 clear);               // Class702da0_Push
    void(__fastcall *play)(u8 *self);                                            // Class702cb0_PlayFirst
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b); // Window_CallDefault
    u32(__fastcall *dwordA0)(u8 *window, void *, u32 index);                     // Window_DwordA0
    u8 *(__cdecl *slots)();                                                      // UserSlots_Instance
    u8 *(__fastcall *ifUsed)(u8 *records, void *, u32 i);                        // Records24_IfUsed
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const u32 *tag);   // Charset_FormatWide
    void(__cdecl *box)(const void *text, u32 kind, u32 a, u32 b, u32 handler, u32 c, u32 d, u32 e);   // 0x47f020 (agent B's)
    void(__cdecl *openSlot)(u32 slot);                                           // Dialog7cb400_Open
    void(__fastcall *useSlot)(u8 *records, void *, u32 slot);                    // 0x5c83b0 (agent B's)
    u32(__cdecl *managerThree)();                                                // GuiManager_CallSlot3
    u32(__cdecl *managerFour)();                                                 // GuiManager_CallSlot4
    void(__fastcall *clearSlot)(u8 *records, void *, u32 slot);                  // 0x5c7e90 (agent B's)
    u32(__cdecl *setMode)();                                                     // Global786b80_SetAndMode3
    void(__cdecl *close)(u32 cancelled);                                         // Dialog7cb400_Close
};
void Dialog7cb3f8_SetEnv(const Dialog7cb3f8Env *env);

extern "C" {

// 0x4a5fc0 (cdecl (window, message, a, b)): the dialog's handler: message 5: closed (0), 0. Else the default's answer; message 0x14 first clears [0x7cb3f8].
s32 __cdecl Dialog7cb3f8_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a6000 (cdecl (window, message, a, b)): a slot's handler, the slot its dword A0 0. Message 0x100: the slot's name (+8 of its record) formatted (the text 0x18f of [0x78ba94] into 0x7ba058) and asked about (0x47f020, on yes 0x4a61b0 with the slot), 0. 0x10006: an empty slot
// (+4 of its 24-byte record 0) opened (Dialog7cb400_Open), else used (0x5c83b0), the dialog closed and the GUI manager's slot 3 ([0x78d90e] set) or 4 called, 0. 0x10001 / 0x10002: the colour of [0x7540cc], [0x7540d0], [0x7cb3f4] / of [0x7540c0-c8] (bytes, sign-extended) notified, then as any other message the default's answer.
s32 __cdecl Dialog7cb3f8_SlotHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a61b0 (cdecl (slot)): the slot cleared (0x5c7e90), the slots shown again. 1.
u32 __cdecl Dialog7cb3f8_ClearConfirmed(u32 slot);

// 0x4a61d0 (cdecl (window, message, a, b)): the handler of the child 0x7a1c. 0x10006: the text 9 asked about (0x47f020, on yes 0x4a6290), 0. 0x10001 / 0x10002: red / the colour of [0x7540c0-c8] notified, then as any other message the default's answer.
s32 __cdecl Dialog7cb3f8_DoneHandler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a6290 (no arguments): Global786b80_SetAndMode3, the dialog closed (1). 1.
u32 __cdecl Dialog7cb3f8_DoneConfirmed();

// 0x4a6580 (no arguments): KeyMaps_ResetAll, [0xf82884]'s +4 hidden, the dialog built on [0xfaa5d8] from the script 0x753c68 into [0x7cb3f8]. When made: the focus; the child 0x7a15 of its children 0x7a14, 0x7a19, 0x7a1a, 0x7a1b: a text part, font 5, the bytes 1, 1 as 0xc00, no text, HoverLabel_SetYellow,
// the colour of [0x7540c0-c8], its dword A0 0 the slot, the handler 0x4a6000; its child 0x7a1c the same with the text 8 and 3 as dword A0 and the handler 0x4a61d0; the slots shown. Then the records [0xf81ebc] made when none (0x110 bytes); with none used, [0xf85614]'s +0x14 pushed 0x186 (1) and 0x187 (0) and [0x7cb3fc] 1, else
// pushed 0x18e (1) and [0x7cb3fc] 0; its +4 played.
void __cdecl Dialog7cb3f8_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gDialog7cb3f8_7540c0, 0x007540c0, 0x1);
GAME_DATA(u8, gDialog7cb3f8_7540c4, 0x007540c4, 0x1);
GAME_DATA(u8, gDialog7cb3f8_7540c8, 0x007540c8, 0x1);
GAME_DATA(u8, gDialog7cb3f8_SlotHandler_7540cc, 0x007540cc, 0x1);
GAME_DATA(u8, gDialog7cb3f8_SlotHandler_7540d0, 0x007540d0, 0x1);
