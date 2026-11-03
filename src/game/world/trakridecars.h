// A track ride's cars added, removed, started and broken down (the script VM's calls; the rides and cars as trackrides.h and trakcarroute.h have them).
// Reimplementation of 0x544c80, 0x544f90, 0x549b80, 0x549db0 and 0x54ad90 (names invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

// What they call (the ports by default; their bench runs them as recorders). IntArea_Contains and ObjectDefs_Find run as they are.
struct TrakRideCarsEnv {
    void(__cdecl *chooseRoute)(u8 *ride, u8 *car);                                     // TrakCar_ChooseRoute (0x54a040)
    s32(__cdecl *spawn)(s32 effect, s32 x, s32 y, s32 z);                              // Particle_Spawn (0x521e60)
    s32(__cdecl *clearQueue)(u8 *animator, u32 index);                                 // Layer_ClearQueue (0x473270)
    u32(__cdecl *play)(u8 *animator, u32 bank, u32 entry, u32 mode, float speed, u32 index);  // Animator_Play (0x4732a0)
    void(__cdecl *load)(u8 *car, u32 effect);                                          // TrakRide_Load (0x549c60)
    u32(__cdecl *createRide)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);   // Ride_Create (0x463060)
    s32(__cdecl *remove)(u8 *car);                                                     // TrakCar_Remove (0x54ae50)
};
// The calls go to env (nullptr: the ports).
void TrakRideCars_SetEnv(const TrakRideCarsEnv *env);

extern "C" {
// 0x544c80 (cdecl (handle); name invented, unconfirmed; "Ride broken"): a ride the handle names gets +0x54 = 2. Kinds -14 to -3 and -1: every used car of it with flag 0x4000, a part +0x24 (not -1)
// and no effect yet (+0x2c is -1): +0x2c = Particle_Spawn (2, the part's place * 1024; __ftol z first; the instance [0x7a4610 + 4 * (+8)]'s holder's part states +4's entry's +4 -> +0x30, or zero);
// kind -1: motion 0xc (+0x10; with an instance: Layer_ClearQueue (0), Animator_Play (0xc, 0, 2, 1.0, 0)).
void __cdecl TrakRide_Break(u32 handle);

// 0x544f90 (cdecl (handle); name invented, unconfirmed; "Start Bump Ride"): a ride the handle names of kind -14, -11, -10, -9, -7, -6, -4, -3 or -1 in state 1 (+0x50) goes to state 2: each used car of it
// loses flags 0x80040, +0x88 = the ride's +4, TrakCar_ChooseRoute; kind -1 with flag 0x4000: motion 5 (+0x10; with an instance: Layer_ClearQueue (0), Animator_Play (5, 0, 1, 1.0, 0)).
void __cdecl TrakRide_StartBump(u32 handle);

// 0x549b80 (cdecl (handle); name invented, unconfirmed): a ride the handle names with a state, its first used car with no node list (+0x30) takes the ride's list (+0xc4, then 0): flags 0x84000, +0x88 = the
// ride's +4, TrakRide_Load (car, 1), TrakCar_ChooseRoute: 1. Else 0.
u32 __cdecl TrakRide_LoadCar(u32 handle);

// 0x549db0 (cdecl (handle, flags); name invented, unconfirmed): a ride the handle names with room (+0x5c below +0x64 and 0x40) and a state: a free car of the pool (TrakPool_FirstFree; [0x877b6c] counted
// up), zeroed, at the ride's +0x74, +0x78, +0x64 = the ride's +8, the ids -1, flag 1 | 0x4000, the ride's +0x5c counted, +0x88 = the ride's +4, +0x98 the first of the module's areas holding it (flag 1, not 0x40;
// or 0), +0x30 the ride's +0xc4 (then 0). With flags: +0x7c and +0x80 zero, those flags, done. Otherwise flag 0x100000, its type +4 = count % +0x18 + +0x14, +8 a ride (Ride_Create of the definition's
// +0x4bc[type]; the definition by the ride's word +0x10; 0x101, -1) playing motion 0xc (+0x10), the first of the ride's cars gets +0x58 and flag 0x400000, TrakRide_Load (car, 0), TrakCar_ChooseRoute, flag
// 0x4000000. Returns the car, or 0.
u8 *__cdecl TrakRide_AddCar(u32 handle, u32 flags);

// 0x54ad90 (cdecl (handle, empty); name invented, unconfirmed): a ride the handle names: its first used car (with empty: with no node list +0x30) is removed (TrakCar_Remove): 1. Else, with empty, once more with
// no such condition (its answer dropped), 0.
u32 __cdecl TrakRide_RemoveCar(u32 handle, u32 empty);
}
