// Five channel volumes and a master volume: reimplementation of 0x503870 and 0x503900.
//
// The object holds the channels' volumes (0-100) at +0x140c + 0x10 * i, each with its share at the
// dword after it (+0x1410 + 0x10 * i), and the master volume at +0x1460. A share is the channel's
// volume as a percentage of the sum of all five, times the master volume, over 100 (0 when the
// sum is 0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x503900 (thiscall; volume, pops 4): the master volume = volume, at most 100; then every share
// worked out again.
void __fastcall Volumes_SetMaster(u8 *object, void *, u32 volume);

// 0x503870 (thiscall; channel, volume, pops 8): channel's volume = volume (at most 100, unsigned;
// the channel number is not checked), then every share worked out again. Returns the volume held.
u32 __fastcall Volumes_SetChannel(u8 *object, void *, u32 channel, u32 volume);
}
