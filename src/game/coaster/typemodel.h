// What a coaster type learns from its model: the parts it can use and the heights of its stances. Reimplementation of 0x430590 (name invented,
// unconfirmed).
//
// A model (the table 0x7ae6a8, by type) has parts (holder +8, +4, +0x7c, 0x14 bytes each: +0 flags, +4 a kind); an instance's parts' objects are at its
// +8, +0x28, +4, 0x14 bytes each, +4.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders).
struct TypeModelEnv {
    u32(__cdecl *createRide)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);   // Ride_Create (0x463060)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u8 *layer, u32 mode, float speed);                                          // Layer_Play (0x472f60)
    s32(__cdecl *update)(u8 *animator, const u8 *block, u32 mode);                                                                    // Animator_Update (0x473c70)
    void(__cdecl *destroyRide)(u32 ride);                                                                                             // Ride_Destroy (0x463c80)
};
// The calls go to env (nullptr: the ports).
void TypeModel_SetEnv(const TypeModelEnv *env);

extern "C" {
// 0x430590 (cdecl (-, block, type); name invented, unconfirmed; 0x42fcd1 and 0x42fd48 call it). Nothing without the model. The block (the coaster type's
// car, +0 its flags):
// - Flags from the model's motions (Animator_HasMotion, bank and entry): 0x80 (2, 0), 0x20 (3, 0), 0x100 (9, 0), 0x40 (10, 0), 0x200 (10, 1).
// - The parts flagged 0x1000000: two or more: flag 8, +0x40 = 2, +0x54 = 1; one: +0x40 = 1. Each is +0x34, or (flag 8, kind 1) +0x48, the last winning.
// - The first flagged 0x800000 is +0x2c; flag 8: +0x30 = 0, +0x44 = 1; else +0x30 = its kind - 1.
// - The parts flagged 0x2000000: each +0x38, or (flag 8, kind 3) +0x4c; none: +0x3c = +0x50 = 0.
// - +0x18 the first flagged 0x100000, or -1.
// - The first four flagged 0x80000: by kind 1 to 4, +0x1c to +0x28 and flags 0x400 to 0x2000.
// - Flag 8: +0x3c = 1, +0x50 = 3; else +0x3c the count flagged 0x400000.
// Then a stand-in ride (Ride_Create (type, 0x3fa, the desc 0x791ce8: 0, 0, 0, 0x791050, with +0xb4 and +0xb8 at 0x791050; -1)). Its layers (+0x10, 0x38 bytes
// each) play what the flags 0x20 to 0x200 say, each with its bank's flags & 3 (Layer_Play at speed 1), the flag 0x1000 goes on its +4, and its
// layer 0's +0x20 = +0x1c * the block's +0xdc clamped to +0xd0 and +0xd4, Animator_Update (it, 1, 0): +0xbc is the height of part +0x2c's object (+0x34;
// none 0), +0x68 is raised to it (a NaN either way keeps +0x68), +0xc4 is part +0x18's height less it (no part: 0). The same with +0xd8 + +0xdc:
// +0xc0 is part +0x2c's height. Ride_Destroy.
void __cdecl CoasterType_ReadModel(u32 unused, u8 *block, u32 type);
}
