// The dialog [0x7cc2ec] (script 0x756400, handler 0x4b24b0): five staff bars and the park's figures. Reimplementation of 0x4b2750; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// Dialog7cc2ec_Open's calls (the ports by default; the bench's recorders: their own benches have them).
struct Dialog7cc2ecEnv {
    s32(__cdecl *atExit)(void(__cdecl *f)());              // 0x67a7d0
    void(__cdecl *attach)(u8 *parent, u32 id, u32 text);   // HoverLabel_Attach
    s32(__stdcall *average)(u32 kind);                     // Things_AverageOfKind
};
void Dialog7cc2ec_SetEnv(const Dialog7cc2ecEnv *env);

extern "C" {

// 0x4b2750 (cdecl; name invented, unconfirmed): open already: nothing. Window7c24c8_Close, Global7cb28c_Select (3, 9), the dialog built (none: nothing more), made [0x7c24c8]. The children 0x2e7, 0x2ec, 0x2ea, 0x2eb, 0x2ed (not checked) in turn:
// panelparts::rangedNumber (0..10000, step 25), slot 7 the dword +0xc, +0x10, +0x14, +0x18, +0x1c of GameTable7cfb90_Get1da718_519450's, their child 0x2e8 a mesh of [0x750e00] and slot 7 the average of the kind 5, 4, 6, 7, 8 << 10 / 100 (signed).
// The figures 0x2de when there: headings 0x2e3, 0x2e5 and in red 0x2e6, 0x2e1 (texts 0x165, 0x169, 0x166, 0x167); [0x7cc2d4], [0x7cc2e0], [0x7cc2dc] the newest of the analyser's rings +0x1fc94, +0x1f7f4, +0x1f5a4 (0 when none); the numbers 0x2e2 [0x7cc2d4],
// 0x2df [0x7cc2e0] red, 0x2e0 [0x7cc2e4] = [0x7cc2dc] - [0x7cc2e0] red, 0x2e4 [0x7cc2d4] - [0x7cc2dc]. The label 0x2da the text 0x93. The mode setting 0xfb3b7c, read the first time (the guard 0xfb34c9 & 1), 2: the child 0x2dd's slot 5 (0).
void __cdecl Dialog7cc2ec_Open();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[16], gDialog7cc2ec_750e00, 0x00750e00, 0x10);
GAME_DATA(u8 *, gDialog7cc2ec_7cc2ec, 0x007cc2ec, 0x4);
GAME_DATA(u8, gDialog7cc2ec_fb34c9, 0x00fb34c9, 0x1);

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0x550], gDialog7cc2ec_Open_756400, 0x00756400, 0x550);   // a window script (or a text / table before one): to the next named address
