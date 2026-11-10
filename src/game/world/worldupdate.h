// The world's tick and a thing's: reimplementation of 0x50b360, 0x516380 and 0x5166b0. Thing_Update's name is the one its callees' notes use, the other two are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the game's by default; the bench's recorders).
struct WorldUpdateEnv {
    using Method = void(__fastcall *)(u8 *self);
    Method visitor[2], object[2], mechanic[2], handyman[2], entertainer[2], guard[2], researcher[2], chat[2];   // the update and the step of the kinds 1, 3, 4, 5, 6, 7, 8, 18 (0x4da5a0 agent C's)
    Method kind11, weather, kind19;                                     // 0x59a550 (agent B's), Weather_Update, 0x4d1e90 (agent C's)
    Method settingReadMode;                                             // Setting_ReadMode
    s32(__cdecl *atexit)(void *exit);                                   // CRT_atexit
    u8 *(__cdecl *findRide)(s32 id);                                    // Rides_FindById
    u32(__cdecl *getElement)(const u8 *ride, s32 index);                // Rides_GetElement
    void(__cdecl *setElement)(u8 *ride, s32 index, u32 value);          // Rides_SetElement
    u32(__fastcall *countVisitors)(u8 *analyser);                       // Things_CountKind1OnPlainCell (cdecl with no arguments: the thiscall's ECX unread)
    u32(__fastcall *typePlain)(u8 *thing);                              // AnalyserCell_TypePlain
    Method tick;                                                        // 0x4d7b20 (agent C's)
    Method actionTick;                                                  // 0x4041d0 (agent B's)
};

void WorldUpdate_SetEnv(const WorldUpdateEnv *env);

extern "C" {
// 0x50b360 (thiscall on a thing): by its kind (byte +2): 1 Visitor, 3 Object, 4 Mechanic, 5 Handyman, 6 Entertainer, 7 Guard, 8 Researcher, each its update and its step; 15 Weather_Update; with the setting 0xfb3b7c
// (read once, guard bit 0 of 0xfb34ab, 0x5d1750 at exit) 1: kind 18 its two calls; not 1: kind 11 0x59a550, kind 19 0x4d1e90. Other kinds nothing.
void __fastcall Thing_Update(u8 *thing);

// 0x516380 (thiscall on the world; the empty Debug_Printf left out; name invented, unconfirmed): the tick +0x1da70c raised. Unless +0x1da738 is 4: each thing in use from [0x7cf56c] (a thing null: its handle read faults, sic), Thing_Update unless null or
// deleted (byte +3). The gates word +0x1da732 set, every thirtieth tick, +0x1da710 set, element 1 of the gates' ride (the thing's +0x24) 1 and the count of the thing of +0x1da720 0: unless a thing in use not deleted of kind 4-8 has
// AnalyserCell_TypePlain 0, that ride's element 1 read again and its element 0 set to 0. Then the delete queue released (Things_ReleaseDeleted), 0x4d7b20 on the world, 0x4041d0 on 0x785a08.
void __fastcall World_Update(u8 *world);

// 0x5166b0 (thiscall on the world; name invented, unconfirmed): the setting 0xfb3b7c read once (guard bit 0 of 0xfb34a9), World_Update when it is 1.
void __fastcall World_UpdateInMode1(u8 *world);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8, gWorldUpdate_fb34ab, 0x00fb34ab, 0x1);   // a function-local static's guard
