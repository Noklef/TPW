// A ride's box in the world: reimplementation of 0x466b70.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x466b70: ride index's (0x7a4610) box when it has its +0xcc block, as min x, y, z then
// max x, y, z: x and z from the tile corners packed in +0xc0 (x high half, z low) and +0xc4
// (the last tile, + 1), 10 units a tile; y from the block's +0x1c and +0x28 above the ride
// model's root (+8 -> +4 -> +0x78) +0x44. Returns 1, or 0 (out left) for no ride or block.
u32 __cdecl Ride_Bounds(s32 index, float *out);

// 0x556af0 (name unconfirmed): the centre of the box of the ride whose index is at handle: x and z the middles of its x and z
// extents and y the ride model's root's y (not the box's), stored y first, then x, then z; returns 0. With the handle 0 or no
// box, x, y and z are 0 (z stored first) and it returns 1.
u32 __cdecl Ride_Centre(const s32 *handle, float *x, float *y, float *z);

// 0x556b90 (name unconfirmed; called by the ride script's walk-node and instruction code): the place of a part of the ride whose index
// is at obj +0xc8. For key at or above 0 and a ride, the part Model_FindPartByKey4(ride, mask, key) (none: 0 returned): its node (the
// ride's +8 -> +0x28 -> +4 entries of 0x14 bytes, +4) gives pos (+0x30..+0x38, else 0s) and, when dir is not null, dir: its +0x20..
// +0x28, negated when the entry's flag 0x10 is set, divided by its length (made a float) and times 1024 (no node: 0, 0, 1024).
// Otherwise pos is the ride's box's centre with its root's y, as Ride_Centre has them (0s for no ride or box) and dir is left. Returns 1.
u32 __cdecl Ride_PartPlace(const u8 *obj, float *pos, float *dir, s32 key, u32 mask);

// 0x551b30 (name unconfirmed; called by the ride script's scheduler code): for part *part of ride index's model (its node as
// Ride_PartPlace finds it), dir is the node's +0x20..+0x28 (negated when the entry's flag 0x10 is set) and up, when not null, its
// +0x10..+0x18 (as they are), each divided by its length made a float; with no node, dir is (0, 0, 1) and up (0, 1, 0).
void __cdecl Ride_PartAxes(s32 index, const u32 *part, float *dir, float *up);
}
