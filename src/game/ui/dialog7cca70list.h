// The dialog [0x7cca70]'s list filled from a session's answer. Reimplementation of 0x4c1e40 and its callback 0x4c20c0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/stdwstring.h"
#include "../core/string.h"

// The calls (the ports by default; agent B's unported 0x5d3230, 0x5b50b0, 0x5b6070 by address; the bench's recorders).
struct Dialog7cca70ListEnv {
    void(__fastcall *clear)(u8 *window);                                                       // Window7059f0_Clear
    s32(__fastcall *addItem)(u8 *window, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *window);                                                     // Window7059f0_RefreshRows
    void(__fastcall *censor)(u8 *save, void *, StdWString *text, u32 flag);                    // 0x5d3230
    u8 *(__fastcall *holderCtor)(u8 *block);                                                   // 0x5b50b0
    u32(__fastcall *valueByKey)(u8 *owner, void *, u32 key);                                   // Owner2c_ValueByKey
    void(__fastcall *setTheme)(u8 *self, void *, const String *theme, u8 *label);              // NamedObjects_SetThemeLabelAnsi
    u8 *(__cdecl *online)();                                                                   // Online_Instance
    void(__fastcall *closeSession)(u8 *session);                                               // 0x5b6070
    void(__fastcall *clearScene)(u8 *scene);                                                   // Class702da0_Clear
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                                 // Window_SendMessage
};
void Dialog7cca70List_SetEnv(const Dialog7cca70ListEnv *env);

extern "C" {

// 0x4c1e40 (cdecl (answer); name invented): the list 0x14beb of [0x7cca70] cleared (Window7059f0_Clear) and, for each node of the list at [answer + 4] (+0 the item, +4 the next), a row: the item's
// wide text +0x14 copied to a local string and, when the global save info's byte 0x25 is set, censored (0x5d3230 (&text, 1)); the holder [0xf7d88c] (made when none: Mem_Alloc(0x404), 0x5b50b0; none on a
// failed allocation) +0xf8's value by the item's key +0x4c (Owner2c_ValueByKey) as a label through NamedObjects_SetThemeLabelAnsi on 0x786b68; the row {the text or the empty string 0x6fd6c4, the label,
// the item's +0xb8, +0x50, +0x54} added with the item as its data, the parent -1, and refresh set only for the row answer's +0xc counts down to (the first is +0xc == 1). Last the rows refreshed and the string released.
void __cdecl Dialog7cca70_FillList(const u8 *answer);

// 0x4c20c0 (cdecl (error, unused, answer); name invented): 0x5bcd10's callback. Without an error the list filled; with one the online session closed (0x5b6070), the scene's list cleared
// ([[0xf85614] + 0x14], Class702da0_Clear) and [0x7cca70] told 4.
void __cdecl Dialog7cca70_ListCallback(u32 error, u32 unused, const u8 *answer);
}
