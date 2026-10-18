// A train's cars: a count at +0x30 and pointers from +0x34. A car (+8 flags) has its part
// (+0x100 -> +0x18 -> a type whose flag 8 (+0) marks a rear coupling) and two couplings
// (+0x78 and +0x84), the one in use at +0x90 with its type part at +0x74.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x432d50: unless 0x790afc is set, marks each car but the last 0x100 (+8), clearing it on
// the last, and picks its coupling: +0x84 with the type's +0x44 for a marked car without
// flag 0x2000 whose type has flag 8, else +0x78 with the type's +0x30.
// (Callers pass the car's +0x18 first; it is unused.)
void __cdecl Train_SetCouplings(u32 unused, u8 *train);

// 0x433280: the length of car's train (0 for no car): over the first car+4 slots of its
// train's cars (+0x100 -> +0x34), each car's length bc + (c0 - bc) * its +0xe8 plus the gap
// +0xc4 (from the train's type, +0x18), then car's own length without a gap.
// sic: the count is car+4, not the train's +0x30; empty slots are skipped.
double __cdecl Car_TrainLength(const u8 *car);

// 0x436750: picks car's coupling as Train_SetCouplings does, without touching its flags.
void __cdecl Car_PickCoupling(u8 *car);

// 0x447790: ride index's (table 0x7a4610) train (+0xd4 -> +0x100) and its car count (+0x30,
// less one when the train has flag 0x20 at byte +4) to *cars, when the ride has flag
// 0x80000 (+4); else 0 and 0.
u8 *__cdecl Ride_Train(u32 index, u32 *cars);

// 0x4477d0: the +0x70 of the train's car n - counting only cars without flag 0x2000 (+8),
// and 0 when there are not enough, when the train has flag 0x20 at byte +4.
u32 __cdecl Train_CarValue(const u8 *train, u32 n);

// 0x432bd0: fills the static record at 0x791ce8 with c, d, 0 and a pointer to the static
// pair {a, b} at 0x791050, and returns it.
u32 *__cdecl CoasterEvent_Make(u32 a, u32 b, u32 c, u32 d);

// 0x434650: where length lies between the type's shortest (+0xbc) and longest (+0xc0) car,
// (length - shortest) / (longest - shortest).
double __cdecl CarType_Unlerp(float length, const u8 *type);

// 0x434680: length / (longest - shortest).
double __cdecl CarType_Scale(float length, const u8 *type);

// 0x4346a0: (longest - shortest) * t + shortest.
double __cdecl CarType_Lerp(float t, const u8 *type);

// 0x4346c0: sets flag 1 of *flags, and 0x1000 too when also is set.
// (The original first passes flags to the empty Debug_Printf.)
void __cdecl Flags_SetChanged(u32 *flags, u32 also);

// 0x433220: sets flag 0x10 of *flags, and 0x80000000 too when high is set.
void __cdecl Flags_SetMoved(u32 *flags, u32 high);

// 0x435710: copies object's position (+0x30, three dwords, in order) to out.
void __cdecl Object_GetPosition(const u8 *object, u32 *out);

// 0x447830: the cost of car (+0x100 -> +0x18 its type; 0 when car or either is missing)
// reaching x, z (or its own +0x4c, +0x54, when x is null) from fromX, fromZ (or, each when
// null, its piece's (+0x68) +0x4c, +0x54): with distance = 0.1 * |dx, dz| and speed = 0.1 *
// CarType_Lerp(car +0xe8, type), (s+0x60 * distance + base) + s+0x64 * speed, base being the
// type's +0x5c when withBase is set, else 0. s is the type, unless its flags (+0) have 0xc000:
// then ride +4 -> +0xa8 when the next owner's (+0x100 -> +0x2c -> +0x18) flags lack 0xc000,
// and so too when s has 0x4000 and the piece lacks 0x80 (+8).
// sic: dz is squared by its float rounding; z is read only through x.
double __cdecl Car_TravelCost(const u8 *ride, u32 withBase, const u8 *car, const float *x, const float *z,
                              const float *fromX, const float *fromZ);

// 0x443df0 (cdecl (car, &cost, &extra or null, x, z, a, b); name unconfirmed; the edit passes 0x79c698 and 0x79c64c): with a current
// link (0x790fec), a selected car (0x79c640) and 0x785a2c clear, the change in cost of moving the car to (x, z): the new cost is
// Float_RoundUp of its Car_TravelCost (link, without the base, from its piece) to x, z, plus, with extra, that of its follower (+0x64)
// from x, z to its own place; the old one cost __ftol'd plus extra's. 0x79c69c = new - old, or 0 when (a, b) is (0x79c6d8, 0x79c6a0).
void __cdecl Car_SetMoveCostChange(const u8 *car, const float *cost, const float *extra, const float *x, const float *z, u32 a, u32 b);

// 0x4344c0: sets up a coupling {count, two arrays of 0x58-byte seats}: count from spec+0xc,
// both arrays allocated zeroed, and each seat i (in both) given the part with key
// spec+0x10 + i and flag 0x400000 of holder 0x7ae6a8[type+0x14] (+4) and the car (+0x10).
void __cdecl Car_SetupCoupling(u8 *car, u32 *coupling, const u8 *spec, const u8 *type);

// 0x433240: frees a coupling's seat arrays - the first only when the car's part (+0x100)
// lacks flag 1 at +4 - and clears both pointers.
void __cdecl Car_FreeCoupling(const u8 *car, u32 *coupling);
}
