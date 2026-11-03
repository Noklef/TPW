// A track ride's car (a 0xac-byte record of the pool [0x877b68]) choosing what to do next, by its ride's kind (the dword at +0). Reimplementation of
// 0x54a040 (name invented, unconfirmed).
//
// The ride's route is a list of nodes at +0xa8, +0xac of them (+0 flags, +4 the point's number, +0x14 the next), +0xa4 its start; the module's
// other nodes are an array at [0x877b78] (0x1c bytes each, [0x877b7c] of them), which the car's +0x7c indexes. The car's flag 0x100000 means
// that it has only just been put on the ride; 0x7a4610 is the table of instances, 0x877358 the module's own sine table (0x200 s32s, 8.8).
#pragma once

#include "../../../re/types.h"

// What the choice calls (the ports by default; its bench runs them as recorders).
struct TrakCarRouteEnv {
    u32(__cdecl *findPart)(const u8 *holder, u32 mask, u32 key);                        // Model_FindPartByKey4 (0x44b220)
    u32(__cdecl *createRide)(u32 type, u32 flags, const u32 *desc, u32 mask, u32 d4, u32 a5, u32 a6, u32 a7, s32 ride, u32 loaded);   // Ride_Create (0x463060)
    u32(__cdecl *playSound)(s32 id, u32 owner, u32 category, s32 index, s32 x, s32 y, s32 z);   // Sound_PlayRideElement (0x51eeb0)
    u32(__cdecl *setSoundPosition)(u32 sound, s32 x, s32 y, s32 z);                     // Sound_SetPosition (0x51c270)
    u32(__cdecl *areasOverlap)(const s32 *a, const s32 *b);                             // IntArea_Overlaps (0x5491e0)
};
// The calls go to env (nullptr: the ports).
void TrakCarRoute_SetEnv(const TrakCarRouteEnv *env);

extern "C" {
// 0x54a040 (cdecl (ride, car); name invented, unconfirmed; the track rides' 0x54505e, 0x547852, 0x5479fd, 0x547a36, 0x547ab8, 0x549c48 and
// 0x549fff call it). Flag 0x100000 ends cleared. By the ride's kind:
// - -14, -11, -6, -3, -1, new: up to 100 tries (World_Rand) at a place within the ride's area ((+0xc0)->+0xc about +0x74 / +0x78; the sine table, each
//   product / 256 towards zero) until (or the 100th whatever) no other used car (the pool, as many as [0x877b6c]) is nearer than the sum of their
//   sizes +0x64 (fild; fsqrt; __ftol of the halved distances, shifted back); flag 0x4000 cleared; +0x50 = +0x54 = a random 0x1ff; +0x24 / +0x28 the
//   instance's parts keyed 2 / 1 (mask 0x100); kind -1: flags | 0x3000000 and +0xc a ride (Ride_Create of the type +0x4bc of ride's +0x10's object
//   definition; 0x101, -1); +0xa4 = -1. Then, without a sound (+0x20) and with flags 0x404000, the car's ride (+0x9c) in state 2 (+0x50) and no flag
//   0x20: its sound. Then, without flags 0x40004, 3 in 16 (World_Rand) and with the ride's +0x5c above 1: a random one of the other used cars on the same
//   ride, not flagged 0x20, whose areas (+0x98) overlap (IntArea_Overlaps; the list through +0x94, newest first): flags & 0xfffbff85 | 0x8004, +0x80
//   its index, +0x8c = 0x5a, and done. Else flags & 0xfffbff89 | 0x18008, +0x8c = 3 and +0x7c the point of a node: with flag 8, 4 in 16, the
//   next after the node +0x7c; else a random (the ride's +0xac) step along the route (each next, past the end the first).
// - -10, -9: as -7 and -4, a new car gaining flags 0x3c00 first.
// - -7, -4: flags & 0xfffbff81, +0x8c = 0. New: its sound, +0x24 the part keyed 1 and, with flag 0x2000, +0xc a ride as above. Then by the ride's state
//   (+0x50): 1: flags | 0x10048 and +0x7c the point of the first node flagged 2 past the ride's +0x5c - 1 others (a node without the flag spins
//   forever, sic; none for a +0x5c of 0: the first); 2: flags | 0x10008, a +0x7c outside the array the start's point, +0x8c = 0, +0x68 = (3 - a
//   random 7) * (+0x20 / 9).
// - -13, -12, -8, -5: flags & 0xfffbff81, +0x8c = 0. New: its sound and +0x7c the start's point. Flags | 0x10008, a +0x7c outside the array the start's
//   point, +0x68 = 0.
// - -2: flags & 0xfffbff89 | 0x10008, +0x7c the point of the next of (flag 8: the node +0x7c; else the first) (none: the first), +0x8c = 0.
// - Any other: flags & 0xffffff91 | 0x10010 and +0x7c a random node.
// "Its sound": +0x20 = Sound_PlayRideElement (the object of the car's ride's +0x24 (TrakRide_FindThing), or 0; 0, [0x803a3c], 0, 0, 0, 0) and, if any, the
// answer of Sound_SetPosition (it, __ftol of +0x34 and +0x38 by the float 0x700f20, 0 for y).
void __cdecl TrakCar_ChooseRoute(u8 *ride, u8 *car);
}
