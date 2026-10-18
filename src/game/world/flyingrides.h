// The flying rides (module FLYR, saved as 'FLY_' by 0x55de70): a table of 100 records at 0x8791f8,
// numbered from 1 (0x55a620 allocates them, 0x122c bytes each). Reimplementation of their
// accessors 0x55a570 and 0x55d610-0x55de30.
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
}
