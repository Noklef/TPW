// The world's own small methods: reimplementation of 0x51aad0, 0x51aaf0, the built-rides
// bitsets 0x51ab20 / 0x51ab60, and the empty and 12-byte record constructors 0x51ad30 /
// 0x51ae10. World fields as World_Save (0x516c80) names them: +0x1da714
// mNumberOfVisitorsToDate, +0x1da72a mCurrentArrivalVehicle, +0x1da72c mArrivalVehicle_Size1.
// A ride's number in the ride database is its definition's +0x94 (1 on; 0 none), bit n - 1 of
// a 32-byte bitset; its definition's +0x98 marks it to leave out of 0x51ab60's count.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x51aad0 (thiscall): 1 when the current arrival vehicle is set and is not the size-1 one.
u32 __fastcall World_ArrivalVehicleNotSize1(const u8 *world);

// 0x51aaf0 (thiscall): one more visitor to date (the original reports the count to the empty
// Debug_Printf); returns the count.
u32 __fastcall World_AddVisitorToDate(u8 *world);

// 0x51ab20 (thiscall, the world unused; u16 id): the ride with that definition id (on
// gObjectDefs 0x7890a0, which has to have it) marked built in the bitset at 0x802080.
void __fastcall World_MarkRideBuilt(u8 *world, void *, u32 id);

// 0x51ab60 (thiscall, the world unused; wanted, leave_out): the database's rides as a bitset at
// 0x7cfb70 (those with +0x98 set left out when leave_out); 0 when a wanted ride is not among
// them, else 1. Only bit 0 of each byte is ever compared (sic: (byte & bit) == 1).
u32 __fastcall World_HasRides(u8 *world, void *, const u8 *wanted, u32 leave_out);

// 0x51ad30 (thiscall): an empty constructor (the vector constructor's for several types);
// returns this.
void *__fastcall Empty_Ctor(void *self);

// 0x51ae10 (thiscall): a 12-byte record of the 0x21c50-byte thing (five at +0x1f0b8): +0 0,
// +4 -999, byte +8 0; returns this.
u8 *__fastcall Record999_Ctor(u8 *record);
}
