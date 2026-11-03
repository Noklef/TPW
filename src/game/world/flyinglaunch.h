// A flying ride's record made, and a new flyer in a free slot of it. Reimplementation of 0x55a620 and 0x55a720 (names invented, unconfirmed).
// The records are the table 0x8791f8 (flyingrides.h); their 20 slots are 0xe4 bytes each from +0x5c, free with +0x80 zero. 0x7660b8 to 0x7660c4: the
// playfield's greatest and least x and z.
#pragma once

#include "../../../re/types.h"

// What it calls (the ports by default; its bench runs them as recorders).
struct FlyingLaunchEnv {
    u32(__cdecl *findPart)(const u8 *holder, u32 mask, u32 key);                                                                      // Model_FindPartByKey4 (0x44b220)
    u32(__cdecl *createRide)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);   // Ride_Create (0x463060)
    u32(__cdecl *playSound)(s32 id, u32 owner, u32 category, s32 index, s32 x, s32 y, s32 z);                                         // Sound_PlayRideElement (0x51eeb0)
    void(__cdecl *playMotion)(s32 index, u32 slotIndex, u32 bank, u32 entry, u32 loop);                                               // FlyingRide_PlaySlotMotion (0x55aa80)
    u32(__cdecl *launch)(s32 index);                                                                                                  // FlyingRide_Launch (0x55a720)
};
// The calls go to env (nullptr: the ports).
void FlyingLaunch_SetEnv(const FlyingLaunchEnv *env);

extern "C" {
// 0x55a720 (cdecl (index); name invented, unconfirmed; 0x55a6e7 and 0x55ac19 call it): 0 when no slot is free, else 1. The first free slot, its state +0x80 = 4,
// starts afresh: +0 the index, +0x7c a random 0-99 + the record's +0x28, height 0x36b0, and a place just off the playfield (World_Rand): a random x and z
// off one side (0xafc8 past, the side and axis random; the other axis random inside: 100 * random mod the span). That is +4 to +0xc and +0x4c to +0x54, and
// +0x58 to +0x60 with x less 0x4e20. The record's +0xc gains 1. +0xe0 the sound (Sound_PlayRideElement (the record's +0x54, 0, [0x803a3c], 0, the place / 300)),
// +0xd8 a random. The only slot in use: state 1, +0x7c = 0, at the record's +0x14 to +0x1c, +0x10 the record's +0x40, the record's +0x3c = 1. +0xc0 a ride
// (Ride_Create of +0x4bc of the object definition with +0xe of the object (the table 0x7cfb90, 0x14 bytes each) of the record's +0x58; 0x101, -1).
// FlyingRide_PlaySlotMotion (index, slot, 0, 0, 0); +0xdc the count of the ride's parts keyed 1 on (mask 0x80) before the first that is missing.
u32 __cdecl FlyingRide_Launch(s32 index);

// 0x55a620 (cdecl (object, sound, x, y, z, turn, quick); name invented, unconfirmed): the first empty entry 1 to 99 of the table 0x8791f8 gets a record (Mem_GlobalAlloc 0x122c; none: the entry stays 0 and the
// read of its +0x4c faults, as the original's): +0x58 the object's low word, +0x54 the sound, +0x14 to +0x1c the place, +0x20 and +0x24 0x400, +0x40 0x400 - turn, +0x4c quick, +0 +4 +0x2c +0x38 1, +0xc
// +0x3c +0x44 +0x48 0, +0x10 and +0x28 300, +0x50 5, the 20 slots free, FlyingRide_Launch (index); +0x34 1 and +0x30 0x9c4 (quick) or 0x1388. Returns the index; -1 with no empty entry.
s32 __cdecl FlyingRide_Create(u32 object, u32 sound, u32 x, u32 y, u32 z, u32 turn, u32 quick);
}
