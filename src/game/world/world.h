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

// 0x515540 (thiscall; name unconfirmed): the world's constructor: +0 Class507800_Ctor, the calendar clock at +0x2a0 (FunnyTime_Construct), +0x2c4
// Init_4cf030; the map's 0x4000 cells at +0x2d8 (MapCell_Ctor, 0x44 bytes each), the 0x4000 track cells 0x110000 past them (TrackCell_Ctor, 0x28
// bytes) and the 0x4000 RE cells at map +0x1b1104 (RECell_Ctor, 10 bytes), constructed through CRT_VectorCtorIteratorNoEh, and the memory ranges
// of the first two set (MemRanges_SetCells44 / 28); 0x96 0x20-byte records at +0x1d943c (Empty_Ctor); the words +0x1da73c..+0x1da746 0, the dword
// +0x1da710 1, [0x7cf56c] 0 and [0x801bbc] 0x7cfb90. Returns the world.
u8 *__fastcall World_Ctor(u8 *world);

// 0x51ad30 (thiscall): an empty constructor (the vector constructor's for several types);
// returns this.
void *__fastcall Empty_Ctor(void *self);

// 0x51ae10 (thiscall): a 12-byte record of the 0x21c50-byte thing (five at +0x1f0b8): +0 0,
// +4 -999, byte +8 0; returns this.
u8 *__fastcall Record999_Ctor(u8 *record);

// 0x4c9050 (cdecl on the game object [0x80239c]): whether the dword at +0x1da710 of the game object is 0 (as AL).
u8 __cdecl Game_Field1da710IsZero();

// 0x519450: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da718 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da718_519450(u8 *o);

// 0x519490: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da71a of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da71a_519490(u8 *o);

// 0x5194d0: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da71c of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da71c_5194d0(u8 *o);

// 0x519510: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da720 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da720_519510(u8 *o);

// 0x519550: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da722 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da722_519550(u8 *o);

// 0x519590: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da724 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da724_519590(u8 *o);

// 0x5195d0: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da726 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da726_5195d0(u8 *o);

// 0x519610: the dword of the table at 0x7cfb90 for the 16-bit field at +0x1da736 of the game object (the debug printf in the original is not called).
u32 __fastcall GameTable7cfb90_Get1da736_519610(u8 *o);

// 0x51a9d0 (thiscall on the game object; name unconfirmed; the debug print is not reproduced): the bus stop position (Map_BusStopPos with 0) gives four cells: it, it + 1, it - 0x100 and
// it - 0xff; the handles at +0x2b8 + cell * 0x44 that are not 0 name things (table 0x7cfb90, 20 bytes each) and 1 is returned for the first whose byte +2 is 1 and dword +0x220 is 0x15, else 0.
u32 __fastcall World_BusStopBlocked(u8 *world);
}
