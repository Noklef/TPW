// The thing panels' go-to: the map moved to the shown thing's cell, asked first in the game's mode where it must be. Reimplementation of 0x48cd10; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; 0x47f020 agent B's, by address until ported; the bench's recorders: their own benches have them).
struct ThingGotoEnv {
    void(__cdecl *messageBox)(const u8 *record, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g);  // 0x47f020
    u16 *(__cdecl *wideText)(u32 id);                         // Language_WideText
    u32(__cdecl *set2654)(u32 thing);                         // Global7c2654_Set
    void(__cdecl *focusCell)(u32 record);                     // FocusCell_Update
    void(__cdecl *setMode)(u32 mode, u32 record);             // GameMode_Set
    void(__cdecl *mapClick)(s32 x, s32 y, u32 flags, u32 useStored);   // GameMode_MapClick
};
void ThingGoto_SetEnv(const ThingGotoEnv *env);

extern "C" {

// 0x48cd10 (thiscall): with the byte [0x78d90f] the question (0x47f020: text 0x18c, the bytes ff ff ff, 4, the thing [0x7c2658]; 2, 0, 1, Thing_GoToCell_48cde0 as an address, 0, 0, 0), else that function's body inline for the thing: Global7c2654_Set (thing), FocusCell_Update (1), GameMode_Set (0x33, 1),
// GameMode_MapClick of the cell (the thing's byte +7 << 7) + byte +5 + 1 less 1: its low 7 bits, the rest; 0, 0. Either way [0x7c2658] = 0 and the panel's slot 0xb.
void __fastcall ThingGoto_Start(u8 *panel);
}
