// A ride's box in the world: reimplementation of 0x466b70.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x466b70: ride index's (0x7a4610) box when it has its +0xcc block, as min x, y, z then
// max x, y, z: x and z from the tile corners packed in +0xc0 (x high half, z low) and +0xc4
// (the last tile, + 1), 10 units a tile; y from the block's +0x1c and +0x28 above the ride
// model's root (+8 -> +4 -> +0x78) +0x44. Returns 1, or 0 (out left) for no ride or block.
u32 __cdecl Ride_Bounds(s32 index, float *out);
}
