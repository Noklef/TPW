// The handler of the dialog [0x7cc188], the address book, and the callback of its delete question. Reimplementation of 0x4a73d0, 0x4a7bc0; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, 0x47f020 by address; the bench's recorders: its own bench has them).
struct Dialog7cc188HandlerEnv {
    u8 *(__cdecl *source)();                                                             // UserSlots_Instance
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                           // Window7059f0_NthItemData
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    void(__cdecl *cancel)(u32 id);                                                       // Game_CancelAdvisor
    u16 *(__cdecl *wideText)(u32 index);                                                 // Language_WideText
    s32(__cdecl *format)(u8 *charset, u16 *dest, u32 size, u32 entry, const void *arg);  // Charset_FormatWide
    void(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    s32(__fastcall *handler)(u8 *window, void *, u32 message, u32 a, u32 b);             // Window_CallDefault
    u8(__fastcall *erase)(u8 *object, void *, u32 index);                                // List_EraseNthCounted
    void(__cdecl *fillList)(u32 clear);                                                  // Dialog7cc188_FillList
};
void Dialog7cc188Handler_SetEnv(const Dialog7cc188HandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x4a73d0 (cdecl (window, message, a, b)). A row's name: the row's data (Window7059f0_NthItemData); not negative: that many strings past the first of the list +0x100 of
// UserSlots_Instance, a null text the empty one, as wide text in 0x7cbeb8 ("%.63S"); negative: the wide string +0x9c of the session's Obj_Get3a4.
// 0x14: [0x7cc188] = 0, the advisor's 0x134 cancelled. 0x100: a -1: [0x7cc188] sent (4, 0, 0), answer 0 (no default). a 0x24: the row chosen (+0x14c) of its child 0x26 (no test of
// the child, sic), when not negative: the question 0xd7 on the name, in the box 0x47f020 with colour ff ff ff, kind 4 and the row's data, buttons 2, 0, 1 and the callback 0x4a7bc0.
// 0x400 (child a there, b not -1): the name of row b added to the recipients 0x7cb4d0, unless in them: ";name", the whole of them when they were the text 0xd3, else on their end;
// child 0x5da5 of [0x7cc184] given them in slot 9. 0x401 (child a there): child 0x25 given in slot 9 the text 0x78a374 (b -1) or the name of row b. Last, but for the one above,
// Window_CallDefault; its answer.
s32 __cdecl Dialog7cc188_Handler(u8 *window, u32 message, u32 a, u32 b);

// 0x4a7bc0 (cdecl (index)): the string index gone from the list +0x100 of UserSlots_Instance (List_EraseNthCounted), Dialog7cc188_FillList(1). Answer 1.
u32 __cdecl Dialog7cc188_EraseName(u32 index);
}
