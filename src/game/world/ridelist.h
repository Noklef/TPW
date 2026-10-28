// The list of rides' records and its accessors: reimplementation of 0x55a070, 0x55a0b0, 0x55a300, 0x55a330, 0x55a360, 0x55a390 and 0x55a3e0.
//
// The dword at 0x8791a0 says the ride system exists (the accessors report to the empty debug printf and give 0 otherwise); the dword at 0x8791b0
// heads a list of records (the next at +0, an id at +8, a link id at +0x14, an array of dwords at +0x1c of the count at +0x8c, a word at +0xc0, a
// dword at +0x9c). The debug printf calls of the original are left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x55a070 (cdecl; id): the record with the id, 0 when there is none or no system.
u8 *__cdecl Rides_FindById(s32 id);

// 0x55a0b0 / 0x55a390 (cdecl; record, index[, value]): the dword at that index of the record's array, set / read, when the record is not 0 and
// the index is within its count (0 if not).
void __cdecl Rides_SetElement(u8 *record, s32 index, u32 value);
u32 __cdecl Rides_GetElement(const u8 *record, s32 index);

// 0x55a300 / 0x55a330 / 0x55a360 (cdecl; record[, value]): the word at +0xc0 set from the low 16 bits of the value / read, sign extended / the
// dword at +0x9c; 0 for no record or no system.
void __cdecl Rides_SetWordC0(u8 *record, u32 value);
s32 __cdecl Rides_GetWordC0(const u8 *record);
u32 __cdecl Rides_Get9c(const u8 *record);

// 0x55a4e0 (cdecl (id); name unconfirmed): the dword +0xe0 of the ride record with the id; 0 without the system or such a record.
u32 __cdecl Rides_GetE0ById(s32 id);

// 0x55a3e0 (cdecl; id, index): the element of the array of the record that the link id (+0x14) of the record with the id names; 0 if either
// record is missing or the index is outside.
u32 __cdecl Rides_GetLinkedElement(s32 id, s32 index);

// Objects that keep the id of their ride record at +0x24: 0x4e03f0 (thiscall) sets its element 4 to 0; 0x4e0410 whether its element 7 is above 0
// (signed); 0x4e0a70 whether its element 0 differs from the word at +0x3c.
void __fastcall Obj_ClearRideElement4(const u8 *object);
u32 __fastcall Obj_RideElement7Positive(const u8 *object);
u32 __fastcall Obj_RideElement0Differs(const u8 *object);

// 0x4dd6e0 (thiscall; speed, pops 4): the word at +0xc0 of the object's ride record set from the speed, and the dword at +0x58 = the speed.
void __fastcall Obj_SetSpeed(u8 *object, void *, u32 speed);

// 0x4dd720 (thiscall; duration, pops 4): the low byte of the duration limited to the bounds of the object's definition (the dwords at
// +0x12c and +0x130; ObjectDefs_Find of the id at +0xe, signed compares) is stored as element 3 of the ride record and at +0x5c (a byte).
void __fastcall Obj_SetDuration(u8 *object, void *, u32 duration);

// 0x51a690 (thiscall on the game object): -1 when the word at +0x1da72a is 0. Otherwise element 1 of the ride record of the thing with that handle:
// when it is not 6 that is returned; when it is 6 the element is set to 0, the word cleared and -1 returned.
s32 __fastcall Game_TakeRideElement1(u8 *game);

// 0x51a760 (thiscall on the game object; name unconfirmed): 1 when the word at +0x1da72a is 0 or differs from the one at +0x1da72c. Otherwise 0 when
// Game_TakeRideElement1 gives 3; else 1 when the dword at +0x2d0 is at least 10 (signed); else 0 when a second take gives 1, or a third (the same, the
// thing found by Thing_ByHandle to clear the element) gives 2; 1 otherwise.
s32 __fastcall Game_PollRideElement1(u8 *game);

// 0x4396e0 (cdecl; object, may be 0): for each of the dword at +0xf4 slots of 0x80 bytes from the block at +0x128 whose dwords at +0x54, +0x58,
// +0x5c and +0x60 are 0: Rides_GetLinkedElement of the object's ride id (+0x24) and element 5, 6, 7 and 8.
void __cdecl Obj_FillLinkedElements(u8 *object);

// 0x436f60 (cdecl; id, value): the object with the dword id at +0x10 in the list from [0x790fe0] (linked through +0x18) gets the dword at +0x24 = value
// (a ride record id) and Obj_FillLinkedElements; nothing when no object has the id.
void __cdecl Objects_SetRideId(u32 id, u32 value);

// 0x4e2670 (thiscall; one argument that is not used, pops 4): the byte at +0x190 compared with the world's next random number modulo 100
// (World_Rand of [0x7cf83c], after finding the definition by the id at +0xe, which becomes the definitions' cache); element 0xb of the object's ride record = (the byte >= that number). Returns that comparison too.
u32 __fastcall Obj_RollChance(const u8 *object, void *, u32 unused);

// 0x4e0db0 (thiscall): element 4 of the object's ride record (Rides_FindById of the dword at +0x24) = 1 (the debug print is not reproduced).
void __fastcall Obj_SetRideElement4One(const u8 *object);

// 0x4e0ac0 (thiscall; handle, pops 4; name unconfirmed): the word at +0x6c = 0; element 0 of the object's ride record (Rides_FindById of the
// dword at +0x24) = 0 when it equals the handle's low word (the debug prints are not reproduced).
void __fastcall Obj_ReleaseRideElement0(u8 *object, void *, u32 handle);

// 0x4e0900 (thiscall; handle, pops 4; name unconfirmed): 0 when the state at +0x19c is 1 or 4 or the dword at +0x68 is 0. Otherwise the word at +0x6c = 0
// (sic: not the handle) and, when element 0 of the object's ride record (Rides_FindById of the dword at +0x24) is 0, it becomes the handle's low word and
// the result is 1; 0 when it was taken (the debug prints are not reproduced).
u32 __fastcall Obj_ClaimRideElement0(u8 *object, void *, u32 handle);

// 0x51a290 (thiscall on the game object; the debug print is not reproduced): -1 when the word at +0x1da732 is 0, otherwise element 1 of the
// ride record of the thing with that handle (Rides_GetElement of Rides_FindById of the dword at +0x24 of the thing in its slot).
s32 __fastcall Game_GetRideElement1(const u8 *game);

// 0x4685c0 (cdecl; index, value; name unconfirmed): for a non-zero index, the ride in table 0x7a4610 gets the dword at +0xd4 = value and, when bit 0x40000 of its flags (+4) is set,
// Objects_SetRideId(index, value) hands the value to the objects.
void __cdecl Ride_SetRecordId(u32 index, u32 value);

// 0x55a100 (cdecl (id); name unconfirmed): the ride record with the id (Rides_FindById; none: its +0xac read through null, sic) and
// its thing (the u16 +0xac, 0 for none: nothing; 0x7cfb90, 20 bytes each): the thing's definition (ObjectDefs_Find of 0x7890a0 by
// its u16 +0xe) names an effect at +0x6c (0: nothing). With the ride index +0xc8 set and its box (Ride_Bounds; none: nothing) the
// effect starts (Particle_Spawn) at 1024 ([0x700fdc]) times the box's middle x and z (sums times 0.5, z unrounded, x as a float)
// and the ride model's root's y (+8 -> +4 -> +0x78, +0x44); the box read again (when the index is still set and it has one) gives
// the system's vector B (ParticleSystem_SetVectorB) the box's extents x and z times [0x701050] and y times [0x701054] (z and y
// unrounded, x a float); then sound 0xb8 plays in the category [0x803a28] (Sound_PlayAt, owner 0) at the float x, y, z, __ftol'd.
// Each __ftol'd z first. The original's Debug_Printf calls (no system; no things) are left out.
void __cdecl Ride_PlayEffect(s32 id);

// 0x55a470 (cdecl; name invented, unconfirmed): the EAX record (Sound_SetEaxRecord) of the first ride record (the list 0x8791b0, the system's
// flag not looked at) of the thing the camera follows (Camera_Followed, +0xc8) with no parent (+0x10) and a reverb (+0xd8): its reverb,
// nothing when that is negative; none followed or no such record: gBalanceLevel's reverb ([0x78578c]).
void __cdecl RideScripts_SetReverb();
}  // extern "C"
