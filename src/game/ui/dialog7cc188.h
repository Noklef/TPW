// The list of the dialog [0x7cc188] (its child 0x26). Reimplementation of 0x4a7a80; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc188ListEnv {
    u8 *(__cdecl *source)();                                                             // 0x48bcd0 (its constructor agent B's)
    s32(__cdecl *swprintf)(u16 *to, const u16 *format, const char *text);                // 0x67af00
    s32(__cdecl *atExit)(void(__cdecl *f)());                                            // 0x67a7d0
    void(__fastcall *clear)(u8 *list);                                                   // Window7059f0_Clear
    s32(__fastcall *add)(u8 *list, void *, const u32 *texts, u32 data, s32 parent, u32 refresh);   // Window7059f0_AddItem
    void(__fastcall *refresh)(u8 *list);                                                 // Window7059f0_RefreshRows
    u16 *(__cdecl *wideText)(u32 id);                                                    // Language_WideText
};
void Dialog7cc188_SetListEnv(const Dialog7cc188ListEnv *env);

// The calls of Dialog7cc188_Open (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc188OpenEnv {
    u8 *(__fastcall *build)(u8 *window, void *, u8 *script, u32 handler);    // WindowScript_Build
    void *(__cdecl *alloc)(u32 size);                                        // GameHeap_Alloc
    u8 *(__fastcall *labelPart)(u8 *part, void *, u32 value);                // TextPart6ff530_Ctor
    u8 *(__fastcall *editPart)(u8 *part, void *, u32 value);                 // TextPart6ff4d0_Ctor
    void(__fastcall *setImpl)(u8 *window, void *, u8 *impl);                 // Window_SetImpl
    u8 *(__cdecl *font)(u32 index);                                          // Language_Font
    void(__fastcall *set4c)(u8 *window, void *, u8 *object);                 // Window_Set4c
    void(__fastcall *notifyC00)(u8 *window, void *, u32 a, u32 b);           // Window_NotifyC00
    u16 *(__cdecl *text)(u32 index);                                         // Language_WideText
    void(__cdecl *hoverYellow)(u8 *window);                                  // HoverLabel_SetYellow
    void(__fastcall *buildRows)(u8 *window, void *, u8 *(__cdecl *make)(), void(__cdecl *release)(u8 *));   // Window7059f0_BuildRows
    void(__fastcall *columnBottom)(u8 *window, void *, u32 column, u32 value);   // Window7059f0_SetColumnBottom
    u32(__cdecl *findByName)(const char *name);                              // GuiWindows_FindByName
    void(__fastcall *setHighlight)(u8 *window, void *, u8 *highlight);       // Window7059f0_SetHighlight
    void(__cdecl *fillList)(u32 clear);                                      // Dialog7cc188_FillList
    void(__fastcall *notify40000)(u8 *window, void *, u32 a, u32 b, u32 c, u32 d);   // Window_Notify40000
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);            // Window_SetHandler
    u32(__fastcall *setBuffer)(u8 *window, void *, u32 buffer, u32 size);    // Window705a20_SetBuffer
    void(__fastcall *setCaret)(u8 *window, void *, u8 *caret);               // Window705a20_SetCaret
    u8(__cdecl *post)(u32 id);                                               // Advisor_PostId
};
void Dialog7cc188_SetOpenEnv(const Dialog7cc188OpenEnv *env);

extern "C" {

// 0x4a7a80 (cdecl (clear)): the list child 0x26 of [0x7cc188], emptied when clear is not 0, given the strings of the list +0x100 of 0x48bcd0's object up to the first empty one (a null text is the empty string 0x6fe5b0), each formatted "%.63S" on the stack, its data its index. The mode setting (the
// guard 0xfb34bf bit 0, Setting_ReadMode, atexit 0x5d1750) 1, and the session's Obj_Get3a4's wide string +0x9c set and not empty: one more, the text 0x121, data -1. No refresh by the adds: the rows refreshed last.
void __cdecl Dialog7cc188_FillList(u32 clear);

// 0x4a8140 (cdecl; name invented, unconfirmed): nothing when [0x7cc188] is set. Else the window built from the script 0x7542b8 under [0x7cc184], the handler 0x4a73d0, into
// [0x7cc188] (no test: a failed build faults, sic). The child 0x27 (Window_FindChild each time): a new TextPart6ff530_Ctor part (none on a failed allocation), font 5,
// 1, 1 to Window_NotifyC00, the text 0xd4 in slot 9, yellow; the window's depth [0x7cc184]'s word +0xe0 + 10. The list child 0x26: the rows 0x4a8380 / 0x4a8420, its column
// bottom (0, 0), the highlight of the name 0x74fa84, the list filled (no clearing). The edit child 0x25: a new TextPart6ff4d0_Ctor part, font 7, colours 0xff, (0, 0) to
// Window_NotifyC00, the handler 0x4a77c0, the buffer 0x7cb8d8 of 0x3f, the caret of the name 0x74fa84, the text 0x78a374 in slot 9. Last the advisor's post 0x134.
void __cdecl Dialog7cc188_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[0x8], gText_Dialog7cc188_74fa84, 0x0074fa84, 0x8);
GAME_DATA(u8[372], gDialog7cc188_Open_7542b8, 0x007542b8, 0x174);
GAME_DATA(u8[128], gDialog7cc188_7cb8d8, 0x007cb8d8, 0x80);
