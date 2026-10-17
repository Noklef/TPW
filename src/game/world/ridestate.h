// Predicates on a ride's state that ask each of the ride systems in turn (track rides, the ride list with the flying rides, the coaster registry,
// the ride's model): reimplementation of 0x42a440.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x42a440 (cdecl; object, index): 1 when the ride object (may be 0) has a track ride handle (+0x28) whose link gives a value
// (TrakRide_LinkedValue), or its ride record (id +0x24) names (+0x9c) a flying ride whose first slot holds a ride; else when ride index has
// bit 18 set and its coaster link a value; else whether the ride's holder (0x7a4610[index]) has a part with key 1 among the 0x1000 parts.
u32 __cdecl Ride_IsBusy(const u8 *object, u32 index);
}
