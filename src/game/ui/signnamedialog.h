// The dialogs that name a thing of model 3 (a sign?), a pair of edit fields of 0xe characters at 0x7c2668 and 0x7c2688: one per panel object (0x48d370, handler 0x48d640) and the global one [0x7ca214] (0x4990f0, handler 0x4993e0, whose
// names also go to the save and may turn the cheats on). The names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48d370 (thiscall; name invented, unconfirmed): with no window at self + 0xc and the thing [0x7c2658] there, of model 3: the window built from the script 0x750090 (handler 0x48d640) on self + 0x10, kept at self + 0xc, its depth the
// parent's word +0xe0 + 0x11, its dwords A0 (0) the thing and (1) self; then as below.
// The label 0x3943e: a new TextPart6ff530 (none on a failed allocation), the font 5, Window_NotifyC00 (1, 1), the text 0xc2 in slot 9, yellow. The edit 0x3943c: a new TextPart6ff4d0, the font 7, white, the buffer 0x7c2668 of 0xe, the caret
// of the name 0x74fa84, the thing's first name (0x4e2860 into 0x7ba058, 0x3fff) in slot 9, all selected. The edit 0x3943d the same with 0x7c2688 and 0x4e2920, not selected. The key maps reset, the focus on 0x3943c.
void __fastcall SignNameDialog_Open(u8 *self);

// 0x4990f0 (cdecl; name invented, unconfirmed): as 0x48d370 with [0x7ca214] for the window, on the parent [0x7ca210] (script 0x751928, handler 0x4993e0, depth + 7; no dwords), the thing the table entry 0x7cfb90 of the word [[0x7cf83c] + 0x1da732].
void __cdecl SignNameDialog7ca214_Open();

// 0x48d640 (cdecl; window, message, a, b; name invented, unconfirmed). 0x14: the key maps enabled, the owner's (dword A0 (1)) +0xc cleared, the focus restored. 0x100 with a = -1: the thing (dword A0 (0)) told the two names (0x4e2950, 0x4e2a50,
// then 0x4682b0), each font of the vector [0x789c1c] removed (RemoveFontResourceA, File_Delete) and destroyed (slot 3 (0); the block kept, its end = its begin), the owner's slot 1, the window sent (4, 0, 0), 0 returned. 0x100 with a = -2 and
// 0x804: the window sent (4, 0, 0), 0 returned. 0x802: a = 0x3943c the focus on 0x3943d, else posted (0x100, -1, 0). 0x803: a = 0x3943c the focus on 0x3943d, else on 0x3943c. Otherwise and after these Window_CallDefault's answer.
s32 __cdecl SignNameDialog_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4993e0 (cdecl; name invented, unconfirmed): as 0x48d640, 0x14 clearing [0x7ca214], and a = -1 with the table's thing: after the fonts the save told the names (GlobalSave_SetSignNameA, B), both buffers scrambled in place (each unit ^= its index,
// the length read again each time, so a unit that turns 0 ends it, sic). Both equal to the strings [0x751a34] and [0x751a38]: the cheats (0xfb3520, guard 0xfb34c4 bit 0, atexit 0x5d1750) on. The window sent (4, 0, 0), [0x7ca210]'s child 0x4f3af
// given the thing's name in slot 9, 0 returned.
s32 __cdecl SignNameDialog7ca214_Handler(u8 *window, u32 message, u32 a, u32 b);
}

// The calls of the two Open functions (the ports by default; the bench's recorders: their own benches have them). Window_FindChild and Window_SetDepth run. 0x4e2860 and 0x4e2920 are unported (agent A's range).
struct SignNameDialogOpenEnv {
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);    // WindowScript_Build
    u32(__fastcall *setDword)(u8 *window, void *, u32 index, u32 value);     // Window_SetDwordA0
    void *(__cdecl *alloc)(u32 size);                                        // GameHeap_Alloc
    u8 *(__fastcall *labelPart)(u8 *part, void *, u32 value);                // TextPart6ff530_Ctor
    u8 *(__fastcall *editPart)(u8 *part, void *, u32 value);                 // TextPart6ff4d0_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                 // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                          // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                 // Window_Set4c
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);           // Window_NotifyC00
    u16 *(__cdecl *text)(u32 index);                                         // Language_WideText
    void(__cdecl *yellow)(u8 *window);                                       // HoverLabel_SetYellow
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);    // Window705a20_SetBuffer
    u32(__cdecl *findByName)(const char *name);                              // GuiWindows_FindByName
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);               // Window705a20_SetCaret
    void(__fastcall *nameA)(u8 *thing, void *, u16 *to, u32 size);           // 0x4e2860
    void(__fastcall *nameB)(u8 *thing, void *, u16 *to, u32 size);           // 0x4e2920
    void(__fastcall *selectAll)(u8 *window);                                 // Window705a20_SelectAll
    void(__cdecl *resetKeys)();                                              // KeyMaps_ResetAllAndSetFlag
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                    // Window_SetFocus
};
void SignNameDialog_SetOpenEnv(const SignNameDialogOpenEnv *env);

// The calls of the two handlers. Window_FindChild, Window_DwordA0 and Window_CallDefault run. 0x4e2950, 0x4e2a50 (agent A's range) and 0x4682b0 (agent B's) are unported.
struct SignNameDialogHandlerEnv {
    void(__cdecl *enableKeys)();                                             // KeyMaps_EnableAllAndClearFlag
    u32(__cdecl *restoreFocus)();                                            // Focus_Restore
    void(__fastcall *setNameA)(u8 *thing, void *, const u16 *name);          // 0x4e2950
    void(__fastcall *setNameB)(u8 *thing, void *, const u16 *name);          // 0x4e2a50
    void(__cdecl *announce)(u8 *thing, const u16 *a, const u16 *b);          // 0x4682b0
    s32(__stdcall *removeFont)(const char *path);                            // RemoveFontResourceA, the import at 0x6fd058
    s32 *(__cdecl *deleteFile)(s32 *result, const char *path);               // File_Delete
    u8 *(__cdecl *save)();                                                   // GlobalSave_Instance
    void(__fastcall *saveNameA)(u8 *info, void *, const u16 *name);          // GlobalSave_SetSignNameA
    void(__fastcall *saveNameB)(u8 *info, void *, const u16 *name);          // GlobalSave_SetSignNameB
    u8 *(__fastcall *cheatsCtor)(u8 *object);                                // CheatSystem_Construct
    s32(__cdecl *atExit)(void(__cdecl *stub)());                             // crt atexit 0x67a7d0
    void(__fastcall *cheatsOn)(u8 *cheats, void *, u32 on);                  // CheatSystem_SetOn
    u8 *(__cdecl *thingName)(u8 *thing);                                     // Thing_NameBuffer7ba058
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);               // Window_PostMessage
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);               // Window_SendMessage
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                    // Window_SetFocus
};
void SignNameDialog_SetHandlerEnv(const SignNameDialogHandlerEnv *env);

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(const u16 *, gSignNameDialog7ca214_Handler_751a34, 0x00751a34, 4);
GAME_DATA(const u16 *, gSignNameDialog7ca214_Handler_751a38, 0x00751a38, 4);
