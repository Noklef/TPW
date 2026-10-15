// A ride's model as it is loaded (the loader 0x461f10 follows): reimplementation of 0x461ed0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x461ed0: for the instance at 0x7a4610[*index] (+4 flags, +8 holder, whose +4 model): flag
// 0x20 on the model (+0x30), 0x80800 on its root (+0x78, flags at +0), then 0x1000000 on the
// instance, each read through the pointers again.
void __cdecl Ride_MarkModel(const u32 *index);
}
