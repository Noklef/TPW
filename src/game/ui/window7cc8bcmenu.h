// The handler of the window [0x7cc8bc]'s name list: a row's menu. Reimplementation of 0x4be290; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x4ea9a0 unported, agent A's, by address; the bench's recorders).
struct Window7cc8bcMenuEnv {
    u32(__fastcall *itemData)(u8 *window, void *, s32 n, u32 *out);                // Window7059f0_NthItemData
    u16(__fastcall *itemCell)(u8 *window, void *, s32 n, s32 column, u32 *out);    // Window7059f0_NthItemCell
    u8 *(__cdecl *chat)();                                                         // OnlineChat_Instance
    u8(__fastcall *isSelf)(u8 *chat, void *, u32 cell);                            // 0x4ea9a0
    void(__cdecl *place)(u32 unused, u8 *window);                                  // Window7cc8cc_PlaceAtMouse
    u16(__fastcall *setState)(u8 *window, void *, u8 state, u8 notify);            // Window705a50_SetState
    u32(__fastcall *setA0)(u8 *window, void *, u32 index, u32 value);              // Window_SetDwordA0
    s32(__fastcall *callDefault)(u8 *window, void *, u32 message, u32 a, u32 b);   // Window_CallDefault
};
void Window7cc8bc_SetMenuEnv(const Window7cc8bcMenuEnv *env);

extern "C" {

// 0x4be290 (cdecl (window, message, a, b)): message 0x402: the row b of the list (the child 0xc3123's child 0xc3124 of [0x7cc8bc], neither checked), its data and its column 0's cell. Unless 0x4ea9a0 of the chat answers the cell not 0: with the data's bit 0 [0x7cc8cc] = [0x7cc8c4] and its child 0x1789 placed (Window7cc8cc_PlaceAtMouse (row, it)), its child 0x178c
// next; else [0x7cc8cc] = [0x7cc8c8], its child 0x3003 and its child 0x3006 the same way. That button's state the data's bit 3 (Window705a50_SetState (bit, 1)), the placed child's A0 slot 0 the cell. The answer always Window_CallDefault's.
s32 __cdecl Window7cc8bc_ListHandler(u8 *window, u32 message, u32 a, u32 b);
}
