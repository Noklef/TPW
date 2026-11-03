// The staff member's rename dialog (built from the script 0x7572e8 with this handler; its dwords +0xa0: 0 the staff member, 1 an
// object of the TrackedSource kind). Reimplementation of 0x4b6320. The name is unconfirmed.
#pragma once

#include "../../../re/types.h"

// StaffRename_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct StaffRenameEnv {
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);    // WindowScript_Build
    void(__fastcall *setDepth)(u8 *window, void *, u32 depth);               // Window_SetDepth
    u32(__fastcall *setDwordA0)(u8 *window, void *, u32 index, u32 value);   // Window_SetDwordA0
    void *(__cdecl *alloc)(u32 size);                                        // GameHeap_Alloc
    u8 *(__fastcall *labelPart)(u8 *part, void *, u32 value);                // TextPart6ff530_Ctor
    u8 *(__fastcall *editPart)(u8 *part, void *, u32 value);                 // TextPart6ff4d0_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                 // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                          // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                 // Window_Set4c
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);           // Window_NotifyC00
    u16 *(__cdecl *text)(u32 index);                                         // Language_WideText
    void(__cdecl *hoverYellow)(u8 *window);                                  // HoverLabel_SetYellow
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);    // Window705a20_SetBuffer
    u32(__cdecl *findByName)(const char *name);                              // GuiWindows_FindByName
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);               // Window705a20_SetCaret
    u16 *(__fastcall *staffName)(u8 *staff);                                 // Staff_GetName
    void(__fastcall *selectAll)(u8 *window);                                 // Window705a20_SelectAll
    void(__cdecl *keysOff)();                                                // KeyMaps_ResetAllAndSetFlag
    u32(__fastcall *setFocus)(u8 *window, void *, u32 a);                    // Window_SetFocus
};
void StaffRename_SetEnv(const StaffRenameEnv *env);

extern "C" {

// 0x4b6110 (thiscall; a vtable's slot, no direct callers; name invented): nothing when this +0xc (the dialog) is set, without a shown thing [0x7c2658] or when the thing's byte +2 is not 4-8. Else the dialog built in this +0x10 with the
// handler below, its depth that window's word +0xe0 + 10, its dwords 0 the thing and 1 this. The child 0x5daa (Window_FindChild each time, not checked): a new TextPart6ff530_Ctor part, font 5, Window_NotifyC00 (1, 1), slot 0x24 the text
// 0xc3, HoverLabel_SetYellow. The child 0x5dab: a new TextPart6ff4d0_Ctor part, font 7, all colours 0xff, the buffer 0x7cc360 of 0x20, the caret window "0x74fa84", slot 0x24 the staff member's name, all selected, the key maps reset,
// the focus (0).
void __fastcall StaffRename_Open(u8 *self);

// 0x4b6320 (cdecl (window, message, a, b)): 0x14: the key maps enabled (KeyMaps_EnableAllAndClearFlag), the dword 1's position cleared
// (TrackedSource_Clear), the focus restored (Focus_Restore), then the default handler (Window_CallDefault), whose result is returned.
// 0x100 with a -1: the staff member (dword 0) named from the buffer 0x7cc360 (Staff_SetName), the dword 1's slot 1 called, the window sent
// 4 (Window_SendMessage(4, 0, 0)), 0 returned; with a -2: sent 4, 0 returned. 0x804: sent 4, 0 returned. 0x802: the window posted 0x100
// with -1 (Window_PostMessage(0x100, -1, 0): the confirmation), then the default handler's result. Any other message (0x100 with another
// a too): the default handler's result.
s32 __cdecl StaffRename_Handler(u8 *window, u32 message, s32 a, u32 b);

}  // extern "C"
