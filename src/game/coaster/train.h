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
}
