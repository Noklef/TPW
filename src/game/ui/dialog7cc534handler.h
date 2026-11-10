// The handler of the dial-up progress box [0x7cc534]. Reimplementation of 0x4b9f70; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default, agent B's unported 0x47eed0 by address; tests/fncmp/dialog7cc534handler.py gives recorders).
struct Dialog7cc534HandlerEnv {
    u16 *(__cdecl *text)(u32 index);                                 // Language_WideText
    u32(__cdecl *post)(u8 *window, u32 message, u32 a, u32 b);       // Window_PostMessage
    void(__cdecl *create)(u32 owner);                                // Dialog7cc5f4_Create
    s32(__cdecl *base)(u8 *window, u32 message, u32 a, u32 b);       // 0x47eed0
};
void Dialog7cc534Handler_SetEnv(const Dialog7cc534HandlerEnv *env);   // nullptr: the game's

extern "C" {

// 0x4b9f70 (cdecl (window, message, a, b)). 0x1e, with the stage [0x7cc504] other than [0x7cc508]: that copied, then by stage (-4..6; 0 and others nothing): the state [0x7cc538] set and the box [0x7cc534], when there, its slot 9 given the text: -4: 3, 0x75852c; -3: 1, text 0x108; -2: 1, text 0x10a; -1: 0, text 0x109; 1: 1, text 0x103; 3..6: 1, text 0x104..0x107.
// 2: the state 2, [0x7cc540] = 1, the box (when there) posted (4, 1, 0). Answer 0 always. 4 with a 1: Dialog7cc5f4_Create([0x7cc52c]), the import [0x6fd5bc] on 0x7c24c0 fills the entry [0x7cc50c]'s properties, their wide name at +0x212 printed "%S" to 0x7cc4f0, [0x7cc53c] = 1. 0x14: [0x7cc534] = 0.
// All but 0x1e: 0x47eed0, its answer.
s32 __cdecl Dialog7cc534_Handler(u8 *window, u32 message, u32 a, u32 b);
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(s32, gDialog7cc534handler_7cc508, 0x007cc508, 0x4);
GAME_DATA(u8 *, gDialog7cc534handler_7cc534, 0x007cc534, 0x4);
GAME_DATA(u32, gDialog7cc534handler_7cc538, 0x007cc538, 0x4);
