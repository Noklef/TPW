// The flying rides (module FLYR, saved as 'FLY_' by 0x55de70): a table of 100 records at 0x8791f8,
// numbered from 1 (0x55a620 allocates them, 0x122c bytes each). Reimplementation of their
// accessors 0x55a570 and 0x55d610-0x55de30, and of their set-up 0x55a5a0.
//
// A record's fields as 0x55a620 sets them up: +0 and +4 (1), +0xc (0), +0x10 and +0x28 (0x12c),
// +0x14/+0x18/+0x1c (its arguments), +0x20/+0x24 (0x400), +0x30 (0x9c4 or 0x1388), +0x38 (1),
// +0x3c/+0x44/+0x48 (0), +0x50 (5); then 20 slots of 0xe4 bytes from +0x5c. A slot's fields:
// +0x80 its state, +0x84 a value capped by 0x55d950, +0x8c a signed step (0 at rest), +0x90 a
// u32 for each step, +0xb8, +0xc0 a ride (an index in 0x7a4610) and +0xd8/+0xdc the base and
// count of that ride's flag-0x80 parts the steps go through.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x55a570 (index): the first slot's ride, 0 for an index outside 1-99 or an empty entry (as
// for all of these).
u32 __cdecl FlyingRide_FirstSlotRide(s32 index);

// 0x55aa80 (index, slot, bank, entry, loop; name unconfirmed; the index and slot not checked): when the slot has a ride and its last motion
// (+0xcc bank, +0xd0 entry, +0xd4 loop) differs, the ride's animator (0x7a4610) plays it (Animator_Play, mode 1 when looping else 0, speed 1,
// layer 0) and the three are kept.
void __cdecl FlyingRide_PlaySlotMotion(s32 index, u32 slotIndex, u32 bank, u32 entry, u32 loop);

// 0x55d610 (index): the first slot in state 1, if its step is negative: the ride part the step
// names released (key (base + -step) % count + 1, when count and the ride are set and the ride
// has that part), the step brought one nearer 0, +0x48 decremented, and the u32 for the new
// step returned; else 0.
u32 __cdecl FlyingRide_StepInSlot(s32 index);

// 0x55d720 (index): the same for the first slot with a positive step (in any state).
u32 __cdecl FlyingRide_StepOutSlot(s32 index);

// 0x55d830 (index, value; name unconfirmed): the first slot in state 1, if its step is not negative: value stored as the u32 for that
// step, the step counted up (through the table entry again), the ride part the new step names activated for value (key (base + step) %
// count + 1, when count and the ride are set and the ride has that part; PartStates_Activate), +0x48 incremented and 1 returned; else 0.
u32 __cdecl FlyingRide_AddToSlot(s32 index, u32 value);

// 0x55d950 (index, value): +0x10 = value / 64, and each slot's +0x84 capped at it.
void __cdecl FlyingRide_SetCap(s32 index, s32 value);

// 0x55d9b0 (index, value): +0x50 = value / 10.
void __cdecl FlyingRide_SetTenth(s32 index, s32 value);

// 0x55d9f0 (index, on): on: +0x38 = 1 and +0 = +4. Off, unless +0x34 is 3: +0 = 1, each slot
// not in state 0 has +0x84 cleared and one in state 1 its positive step negated, +0x38 = 0.
void __cdecl FlyingRide_SetOn(s32 index, u32 on);

// 0x55dcf0 (index, value): +4 = value, and +0 too while +0x38 is set.
void __cdecl FlyingRide_Set4(s32 index, u32 value);
void __cdecl FlyingRide_SetBounds();
void __cdecl FlyingRide_ReleaseSlotPart(s32 index, s32 slotIndex, s32 step);

// 0x55cda0 (cdecl (index, slot); called by 0x55d3d0; name invented): the record's (table 0x8791f8, unchecked) slot emptied: its particle
// system (+0xc4) ended (ParticleSystem_End) and cleared, its ride (+0xc0) destroyed (Ride_Destroy 0x463c80, through the queue pieces'
// env) and cleared, the record's +0xc counted down, its sound (+0xe0) given to Sound_Forward6 and cleared (the record re-read after
// each call, as the original).
void __cdecl FlyingRide_ClearSlot(s32 index, u32 slotIndex);

// 0x55c270 (cdecl; camera dwords: [0] the ride index, [0x13..0x15] the target, [0x16..0x18] the old one, [0x22] the corner): the target moves to the old one, the target
// height (+0x14) becomes the record's +0x18 plus 5000 (at least 14000) and the corner goes round (0 to 3): the target x / z become the record's +0x14 / +0x1c plus or minus
// 25000 for the corners (+, +), (+, -), (-, -), (-, +).
void __cdecl FlyingRide_NextCameraCorner(s32 *camera);

// 0x55dd20 (index): +0x3c (else 0).
u32 __cdecl FlyingRide_Get3c(s32 index);

// 0x55dd40 (index): whether the first slot has +0xb8 set and is in state 1.
u32 __cdecl FlyingRide_FirstSlotReady(s32 index);

// 0x55dd80 (index): when +0x38 or +0x48 is set or +0xc is above 1 (signed), and +0x3c is set:
// +0x3c cleared and 1; else 0.
u32 __cdecl FlyingRide_Take3c(s32 index);

// 0x55ddd0 / 0x55ddf0 / 0x55de10 (index, value): +0x44 / +0x20 / +0x24 = value.
void __cdecl FlyingRide_Set44(s32 index, u32 value);
void __cdecl FlyingRide_Set20(s32 index, u32 value);
void __cdecl FlyingRide_Set24(s32 index, u32 value);

// 0x55de30 (index): 0 when +0x48 is 0, +0x3c set and +0xc 1 (or for no record); else 1.
u32 __cdecl FlyingRide_Busy(s32 index);

// 0x55e3b0 (cdecl; name unconfirmed; called by 0x415100 after a load): for each record of the table (indices 1-99) each slot in use (+0x80
// set) claims its ride (+0xc0) again: Ride_Claim(ride, the +0x4bc of the object definition (ObjectDefs_Find on 0x7890a0) for the id at +0xe
// of the record's thing (the u16 slot at +0x58 in gThingSlots), key 0x101, no description, the rest 0), the result kept as the slot's ride.
// (A Debug_Printf is left out.)
void __cdecl FlyingRides_ReclaimRides();

// 0x55cc00 (cdecl; x, y; name unconfirmed; called at 0x55c417, 0x55c7b0 and 0x55c7c8): the floor at a world position. 100000 at or past
// 20000 beyond the bounds (x below 0x7660b8 and from 0x7660bc, y below 0x7660c0 and from 0x7660c4); 30000 when a record of the table
// (indices 1-99) is near: (x - its +0x14) / 32 and (y - its +0x1c) / 32 (truncated) squared and summed (wrapping), __ftol of its x87
// square root << 5 under 10000 (signed); 0 outside the bounds themselves; else Terrain_SurfaceHeight (no slot) of the cell x / 3000,
// y / 3000 (truncated, then clamped to 0 and to below the grid sizes at 0x7a0314 and 0x7a0374) by __ftol, times 300.
s32 __cdecl FlyingRides_FloorAt(s32 x, s32 y);

// 0x55c350 (cdecl; a record; name invented, unconfirmed; called at 0x55b85c and 0x55b874): the next waypoint (+0x4c, +0x50, +0x54) on
// the 20000 grid. A step of 20000 along the axis the last leg (from +0x58 / +0x60) moved most (x when |dx| > |dz|, signed; its sign,
// 0 for none); the waypoint kept as the old one; the floor limit +8 + 5000. With World_Rand (the world 0x7cf83c) % 4 not 0 and the
// floor (FlyingRides_FloorAt) a step ahead under the limit (signed): the waypoint the centre of its square (v / 20000 * 20000, less
// 20000 below 0, plus 10000) and a step ahead. Else the first under the limit of a step to one side, the other, ahead and back
// (the limit rising 5000 each round), the waypoint moved there and centred. Its y: the surface (Terrain_SurfaceHeight, no slot) by
// __ftol, times 300, at whichever of the waypoint and +4 / +0xc is higher (the waypoint only when strictly; each 0 outside the bounds
// 0x7660b8-0x7660c4, cells x / 3000 clamped to the grid 0x7a0314 / 0x7a0374), the chosen one by Camera_ClampCell, plus 5000, at least
// 12000.
void __cdecl FlyingRide_PickWaypoint(u8 *rec);

// 0x55ce60 (cdecl; t, a float capped at 2.0; name unconfirmed): each record's (0x8791fc-0x879388) slots in use (+0x80 set) placed at t
// between their old and new positions. A slot's position (+4, +8, +0xc; old +0x1c, +0x20, +0x24) and three 12-bit angles (+0x10, +0x18,
// +0x14; old +0x28, +0x30, +0x2c) go to the globals 0x8791e0, 0x8791f4, 0x8791d8 and 0x8791e8, 0x8791c4, 0x8791c0: when +0xc8 is set
// (cleared then) the new ones (the position / 300), else (old + (new - old) * t) / 300 and old + the angle's step (12-bit, from 0x800 less
// 0xfff, sic) * t, the position then eased from the last (+0x34, +0x38, +0x3c) as (p + 3 * last) / 4, and the angles from theirs (+0x48,
// +0x40, +0x44; __ftol, 12-bit, brought within 0x800) as (a + 3 * last) / 4, the third (a + 7 * last) / 8 (signed). The six stored as the
// slot's last. A slot with a ride (+0xc0, an index in 0x7a4610) gets its matrix (+8 -> +4 -> +0x78, +0x10) rotation reset, the position as
// its translation, and Matrix_RotateEulerFast(order 0xe) by (-pi * b / 2048, -pi * c / 512, pi * (1 + (3072 - a) / 2048)); then with a
// particle system (+0xc4) and a part keyed 1 among its flag-0x100 parts (Model_FindPartByKey4; none: skipped) the system's vector A set to
// the part's +0x30, +0x34 and +0x38 (+8 -> +0x28 -> +4, 0x14 each, +4; a null part: 0, 0, 0), each __ftol'd and << 10.
void __cdecl FlyingRides_LerpSlots(float t);

// 0x55a5a0 (cdecl; called from 0x54eab4; name unconfirmed): once (the flag 0x8791ec, then set, as 0x8791f0): the table of 100 entries
// at 0x8791f8 cleared and a sine table of 0x1000 dwords made at [0x8791c8] (Mem_GlobalAlloc 0x4000, unchecked): entry j = __ftol(1/2 -
// fsin(2j * [0x701090] (2 pi / 8192)) * [0x701098] (-1024)) (low half; x87 at the game's precision), the table pointer re-read each time.
void __cdecl FlyingRides_Init();

// 0x55da70 (cdecl (index, mode); name unconfirmed): for a record (index 1-99, its entry set) whose mode +0x34 is not mode already, +0x34
// = mode, then by mode: 1 lands it: +0 = +4; with +0x114 set its slot 0 played motion (0xa, 0, not looping: FlyingRide_PlaySlotMotion),
// +0x114 cleared and the sound 0xbb played in the category [0x803a28] at its position +0x60 / +0x64 / +0x68 over 300 (signed:
// Sound_PlayAt, owner 0); with +0xd8 set, +0xd8 = |World_Rand([0x7cf83c])| % 100 + +0x28; its particle system +0x120 ended
// (ParticleSystem_End) and cleared when set. 2: +0 = +4, +0x120 ended when set and replaced by Particle_Spawn(0x27) at the position.
// 3 and 4: +0x114 = 1, +0xe0 = 0, +0x120 ended when set and replaced by Particle_Spawn(0x28) at the position times 1024 over 300
// (signed, the shift wrapping); 4 then steps every slot out (FlyingRide_StepOutSlot until it answers 0) and sets +0 = 1. Other modes
// are only stored.
void __cdecl FlyingRide_SetMode(s32 index, s32 mode);
// 0x55d3d0 (cdecl (index); name invented, unconfirmed): flying ride index's record (table 0x8791f8, 1-99; none: nothing) freed: each of its
// 20 slots (0xe4 bytes from +0x5c) with +0x80 set first has its parts released for the steps |+0x8c| down to 1 (as
// FlyingRide_ReleaseSlotPart: the key (+0xd8 + step) % +0xdc + 1 among the flag-0x80 parts of the ride +0xc0, PartStates_Release; INT_MIN
// stays as it is) and is cleared (FlyingRide_ClearSlot); then the record goes (Mem_GlobalFree) and the table entry is cleared.
void __cdecl FlyingRide_Free(s32 index);
}
