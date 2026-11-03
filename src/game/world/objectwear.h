// A ride or shop wearing down as it runs. Reimplementation of 0x4df670; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct ObjectWearEnv {
    u8 *(__fastcall *findDef)(u8 *defs, void *, u16 id);                  // ObjectDefs_Find
    u8 *(__fastcall *readMode)(u8 *setting);                              // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                             // 0x67a7d0
    u8 *(__cdecl *ride)(s32 id);                                          // Rides_FindById
    u32(__cdecl *element)(const u8 *record, s32 index);                   // Rides_GetElement
    s32(__cdecl *wordC0)(const u8 *record);                               // Rides_GetWordC0
    double(__fastcall *score)(const u8 *object, void *, float a, float b, u32 unused, u32 useB);   // Object_DefRowScore
    void(__cdecl *setElement)(u8 *record, s32 index, u32 value);          // Rides_SetElement
    u16 *(__fastcall *handle)(const u8 *thing, void *, u16 *out);         // Thing_GetHandle
    u32(__fastcall *advisor)(u8 *game);                                   // GameTable7cfb90_Get1da71c_5194d0
    u8(__fastcall *post)(u8 *self, void *, const u8 *message);            // Advisor_Post
};
void ObjectWear_SetEnv(const ObjectWearEnv *env);

extern "C" {

// 0x4df670 (thiscall; called from 0x4e0c49; name invented, unconfirmed): with the definition's (word +0xe) dword at the level's row (byte +0x50 * 0x40 + 0x1b0) not 0 and the mode setting (0xfb3b7c, read under the guard 0xfb34b8 & 1) not 1, the ride +0x24's element 9 above 0 or bit 0 of the byte +0x33, and on every 0x40th tick
// ([0x80239c] + 0x1da70c): the wear (0 with no element 5; else Object_DefRowScore of the ride's word 0xc0, its elements 2 and 3 as floats, 0) taken from the float +0x44 and 0.02 of it from +0x48, each kept in 0..100 (NaN 0). Under 20 (the integer part's low byte), element 8 set. That part under 10 with the wear reaching 10 again: the advisor message 0x42 (type the thing's handle, a its word +0xe, b -1, flag 0, scored).
void __fastcall Object_WearTick(u8 *self);
}
