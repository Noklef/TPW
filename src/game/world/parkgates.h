// The park's gates (the arrival rate, the bus and whether the gates are open): reimplementation of their serializer 0x4cf050. The field
// names are the save's.
#pragma once

#include "../../../re/types.h"

struct Stream;

extern "C" {

// 0x4cf050 (thiscall; stream, save, version, unused; pops 16): 0 for versions other than 0 and 1. Otherwise as a serializer does
// (Save_Field; the last field's load through Save_ReadU8): mArrivalRate (+0, 4 bytes), then the time stamp at +4 (TimeSig_Serialize, given
// the same arguments), mTargetVehicleCapacity (+8, 4), mPeopleOnBus (+0xc, 4), mOffloading (+0x10, 1) and mGatesOpen (+0x11, 1). 1 at the
// first that fails (the debug print left out), else 0.
u32 __fastcall ParkGates_Serialize(u8 *gates, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
