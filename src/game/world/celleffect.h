// A map cell's sprite effect started. Reimplementation of 0x4d93b0; the name is invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

// The calls (the ports by default; the bench's recorders: their own benches have them).
struct CellEffectEnv {
    u8 *(__fastcall *readMode)(u8 *setting);                                         // Setting_ReadMode
    s32(__cdecl *atExit)(void(__cdecl *f)());                                        // 0x67a7d0
    void(__fastcall *spread)(u8 *world, void *, u32 index, u32 cell);                // Map_SpreadAdd
    s32(__cdecl *start)(u32 code, u32 folder, u32 sprite, u32 anim, u32 x, u32 y, u32 z);   // SpriteScript_Start
    void(__fastcall *setSeed)(u8 *world, void *, u32 seed);                          // World_SetSeed
    s32(__fastcall *rand)(u8 *world);                                                // World_Rand
    void(__cdecl *setPosition)(s32 slot, u32 x, u32 y, u32 z, u32 r15);              // SpriteScript_SetPosition
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z);                            // Particle_Spawn
};
void CellEffect_SetEnv(const CellEffectEnv *env);

extern "C" {

// 0x4d93b0 (thiscall (kind), pops 4; called from 0x4fed5d and three more; name invented, unconfirmed): nothing in the mode 1 (the setting 0xfb3b7c, read under the guard 0xfb34b9 & 1) or with an effect up (+0x2c). The script of the kind: 1-5 0x74f050 + 0x28 * (kind - 1), 7 0x74f140 and 8 0x74f168 each with Map_SpreadAdd ([0x8023a0]; 2 or 5, the cell's word +0), any other 0x74f118 (a debug print left out).
// Started into +0x28 (folder 0xb); the world's seed the word; at x = (word - 1) & 0x7f, y = (word - 1) >> 7, its position x * 10 + the 1st rand % 9 + 1, y * 10 + the 2nd's + 1. The word +0x30 = 0, +0x2c = the kind; for 8, +0x38 the particle 0x32 at the position * 1024.
void __fastcall MapCell_StartEffect(u8 *cell, void *, u32 kind);
}
