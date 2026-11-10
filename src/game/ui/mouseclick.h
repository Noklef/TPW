// The click on what the help line shows (src/game/ui/mousehelp's [0x7c24f4]). Reimplementation of 0x4879d0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct MouseClickEnv {
    u32(__cdecl *hasSource)();                                                // Global7c2534_HasSource
    u16 *(__cdecl *objectAt)(u16 *out, s32 x, s32 y);                         // Map_ObjectOrEntranceAt
    u8 *(__fastcall *find)(u8 *table, void *, u32 id, u32 quiet);             // ObjectControls_Find
    u32(__fastcall *table26)(u8 *game);                                       // GameTable7cfb90_Get1da726_5195d0
    u32(__fastcall *money)(u32 source);                                       // Source_Field0c
    u32(__cdecl *playAt)(u32 owner, u32 category, u32 sound, s32 x, s32 y, s32 z);  // Sound_PlayAt
    void(__cdecl *setRecorded)(u32 value, u32 force);                         // Global81d7a4_SetRecorded
    void(__cdecl *thingPanel)(u8 *thing);                                     // ThingPanel_Show
    void(__fastcall *showVisitor)(u8 *self, void *, u8 *thing);               // Panel7cc408_Show
    void(__fastcall *showStaff)(u8 *self, void *, u8 *thing);                 // Panel7cc3b0_Show
    void(__cdecl *open7ca210)();                                              // Dialog7ca210_Open
    void(__cdecl *openFor)(u8 *thing);                                        // Dialog7ca1c8_OpenFor
    void(__cdecl *focusCell)(u32 record);                                     // FocusCell_Update
    u16 *(__fastcall *firstObject)(const u8 *cell, void *, u16 *out);         // Cell_FirstObject
};
void MouseClick_SetEnv(const MouseClickEnv *env);

extern "C" {
// 0x4879d0 (cdecl; called by the handler at 0x488768): 0 without a game [0x7cf83c] or with Global7c2534_HasSource. The mouse mode holder made
// when missing; a current mode [0x7b05d8] (none: 0) answering other than 0 or 1 from its slot 9: 1. Globals7c264c_Load, then by [0x7c24f4]
// (the shown kind), with T the tile [0x7b05cc] (0: 1) and thing(h) the dword of 0x7cfb90's 20-byte slot; "tile mode k" is a new tile mode
// (Mem_Alloc(0x14), MouseModeTile_Ctor (k); 0 when that failed), the holder made when missing and the mode set (MouseMode_Set); "pick h" is
// Global7c2654_Set(thing(h)), FocusCell_Update(1), and the answer 1 unless said:
// 1, 2: tile mode 1.
// 3: T's cell (0x44 bytes from the game + 0x294) of type (+8) 3 whose parent's (word +0x10) first object h (Cell_FirstObject) is not 0: pick h,
//    tile mode 0x14.
// 4: h = Map_ObjectOrEntranceAt of T - 1 (x its low 7 bits, y the rest), starting as T; 0: 1. Control not down (GetAsyncKeyState(0x11) bit 15):
//    ThingPanel_Show(Global7c2654_Get()), 1. Else with the thing's word +0xe: the record's +4 (ObjectControls_Find (word, 0) in the game's
//    +0x1d943c) above the money (Source_Field0c of GameTable7cfb90_Get1da726_5195d0; signed): Sound_PlayAt(0, [0x803a2c], 0x1d, 0, 0, 0), 0. Else
//    a tile mode (MouseModeTile_CtorKind (4, word)) as above, Global81d7a4_SetRecorded(thing's +0x10, 1), 0.
// 5: T's record (40 bytes from the game + 0x1102b0), or for type (+8) 0xc its parent's (word +0x10): type 0x19 FocusCell_Update(1), tile mode 0x1a;
//    0xb or 0x10 as 3 with the record's parent's first object, tile mode 0xe or 0x13; else 1.
// 6: T's cell's parent's first object h not 0: pick without the focus cell (Global7c2654_Set(thing(h))), Dialog7ca1c8_OpenFor(thing(h)), 0.
// 7: Global7c2650_Get not 0: Panel7cc408_Show on 0x7cc408. 8 to 12: the same, Panel7cc3b0_Show on 0x7cc3b0. 13, 14: Dialog7ca210_Open. Else 1.
u32 __cdecl MouseClick_OnShown();
}

#include "../core/gamedata.h"

// Globals by address (tools/gamedata_codemod.py; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32, gMouseclick_7c24f4, 0x007c24f4, 0x4);
