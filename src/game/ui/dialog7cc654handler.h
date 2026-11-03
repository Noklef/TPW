// The handler of the window [0x7cc654] (the mailing lists: the list child 0xee8, each row's data bits 24, 25 its two columns' marks). Reimplementation of 0x4bbc20; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, the unported by address; the bench's recorders: its own bench has them).
struct Dialog7cc654HandlerEnv {
    s32(__cdecl *marked)();                                                              // 0x4bc430
    u32(__fastcall *restricted)(const u8 *session);                                      // Online_Restricted
    u32(__cdecl *post)(u32 code, u32 param);                                             // CommandQueue_Post
    s32(__cdecl *send)(u8 *window, u32 message, u32 a, u32 b);                           // Window_SendMessage
    void(__cdecl *leave)(u32 changed);                                                   // 0x4bc500
    void(__fastcall *closeSession)(u8 *session);                                         // 0x5b6660
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                           // Window7059f0_NthItemData
    void(__fastcall *setTexts)(u8 *window, void *, s32 n, const u32 *texts, u32 mask, u32 refresh, u32 unused);   // Window7059f0_SetItemTexts
    u32(__fastcall *setData)(u8 *window, void *, s32 n, u32 data);                       // Window7059f0_SetNthItemData
    void(__fastcall *refresh)(u8 *window);                                               // Window7059f0_RefreshRows
    void(__fastcall *freeSurface)(void *info);                                           // SurfaceInfo_Free
    u32(__cdecl *focus)();                                                               // Focus_Restore
    u8 *(__cdecl *source)();                                                             // UserSlots_Instance
    u8(__fastcall *first)(u8 *lists);                                                    // 0x5cc5e0
    u8(__fastcall *next)(u8 *lists);                                                     // 0x5cc600
    u16 *(__cdecl *toWide)(const char *text, u16 *buffer, s32 size);                     // String_AnsiToWide
    void(__cdecl *fit)();                                                                // Dialog7cc654_FitText
    void(__cdecl *show)();                                                               // 0x4bc270
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                           // Dialog_BaseHandler
};
void Dialog7cc654Handler_SetEnv(const Dialog7cc654HandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x4bbc20 (cdecl (window, message, a, b)). 0x14: [0x7cc654] = 0, SurfaceInfo_Free(0x7cc628), [[0xf82884] + 4] sent (6, 1, 0), Focus_Restore. 0x15: it sent (6, 0, 0).
// 0x100, a -1: 0x4bc430's count; above 0 with Online_Restricted 0: the command (0x4e, 0) posted, nothing more. Else the window sent (4, 0, 0), 0x4bc500(count not 0), 0x5b6660 on the session, answer 0 (no base handler).
// 0x100, a 0xee5, 0xee6, 0xee7: every row of the list child 0xee8 of [0x7cc654] given mark 1 alone, mark 2 alone, none: its texts 2, 3 the marks as 0 or 1 (mask 0xc, no refresh), its data; then the list refreshed.
// 0x404, b above 1 (signed): the row chosen (+0x14c, no test, sic): b 2: mark 1 flipped, b else: mark 2 flipped, a mark newly set clearing the other; texts as above with refresh, its data.
// 0x401 (child a there, b not -1): row b's data less the marks steps the list 0x5cc600 past 0x5cc5e0 on UserSlots_Instance +0x64, [0x7cc660] = that + 0x10; 0x7cb4d0 the text 0x78a374, then each string of the list's nodes
// (+0x24 of [0x7cc660], its +0xc, null the empty one) as wide and 0x7558f0; Dialog7cc654_FitText, 0x4bc270. Last, but for the one above, Dialog_BaseHandler with the arguments as they came; its answer.
s32 __cdecl Dialog7cc654_Handler(u8 *window, u32 message, u32 a, u32 b);
}
