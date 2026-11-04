// The window [0x7cc654] (the mailing lists) left: the marks acted on, the changes sent, the waiting box. Reimplementation of 0x4bc430, 0x4bc500, 0x4bc660; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, the unported by address; tests/fncmp/dialog7cc654lists.py gives recorders).
struct Dialog7cc654ListsEnv {
    u32(__fastcall *nth)(u8 *window, void *, s32 n, u32 *out);                  // Window7059f0_NthItemData
    u8(__fastcall *first)(u8 *lists);                                           // 0x5cc5e0
    u8(__fastcall *next)(u8 *lists);                                            // 0x5cc600
    void(__fastcall *erase68)(u8 *lists);                                       // List68_EraseMatching
    void(__fastcall *erase74)(u8 *lists);                                       // List74_EraseMatching
    void(__fastcall *mark1)(u8 *lists);                                         // 0x5cc640
    void(__fastcall *mark2)(u8 *lists);                                         // 0x5cc770
    u8 *(__cdecl *source)();                                                    // UserSlots_Instance
    u16 *(__cdecl *text)(u32 index);                                            // Language_WideText
    void(__fastcall *prepare)(u8 *lists);                                       // 0x5cd0a0
    u8(__fastcall *connect)(u8 *lists, void *, const char *host, s32 port, u8 *name);   // 0x5cc8a0
    u8 *(__cdecl *pick)();                                                      // Global7cc53c_Pick7cc4f0
    u8 *(__cdecl *box)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);   // 0x47f020
    u16(__fastcall *setHandler)(u8 *window, void *, u32 handler);               // Window_SetHandler
    u32(__fastcall *setShown)(u8 *window, void *, u32 shown);                   // Window_SetShown
    u32(__cdecl *post)(u32 code, u32 param);                                    // CommandQueue_Post
    u32(__cdecl *postMessage)(u8 *window, u32 message, u32 a, u32 b);           // Window_PostMessage
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);                  // 0x47eed0
};
void Dialog7cc654Lists_SetEnv(const Dialog7cc654ListsEnv *env);   // nullptr: the game's

extern "C" {

// 0x4bc430 (cdecl, no arguments): for each row of the list child 0xee8 of [0x7cc654], its data less the marks steps 0x5cc600 past 0x5cc5e0 on UserSlots_Instance +0x64; List68_EraseMatching, List74_EraseMatching; mark 1 (bit 24) set: 0x5cc640 and counted, else mark 2 (bit 25) set: 0x5cc770. Answers the count.
s32 __cdecl Dialog7cc654_ApplyMarks();

// 0x4bc500 (cdecl (changed)): the lists' +0x94 = 0x4bc640, 0x5cd0a0. With the low byte of changed set: 0x5cc8a0 with the host [0x7a14f4] (none or empty: 0x754454, port 25, else the word [0x7a1550] signed), Global7cc53c_Pick7cc4f0; AL set: [0x7cc658], [0x7cc684], [0x7cc664], [0x7cc65c] = -1,
// the box (text 0x13a, colour ff ff ff, kind 4; all zeros) in [0x7cc688] with the handler 0x4bc660. AL 0: with UserSlots_Instance's +0xc4 0 the child 0xbf432 of [[0xf82884] + 4] hidden, else the command (0x39, 0) posted.
void __cdecl Dialog7cc654_SendChanges(u32 changed);

// 0x4bc660 (cdecl (window, message, a, b); the box's handler). 0x1e: with the box [0x7cc688] there and [0x7cc684], [0x7cc65c] other than [0x7cc658], [0x7cc664]: those two copied, the box posted (4, 0, 0). Answer 0. 0x14: [0x7cc688] = 0, with UserSlots_Instance's +0xc4 0 the child 0xbf432 of [[0xf82884] + 4] hidden.
// All but 0x1e: 0x47eed0, its answer.
s32 __cdecl Dialog7cc654_WaitHandler(u8 *window, u32 message, u32 a, u32 b);
}
