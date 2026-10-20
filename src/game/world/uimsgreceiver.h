// The UI message receiver (thing model 17, whose messages Thing_HandleMessage hands to 0x50ca50): reimplementation of its constructor.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x50c9b0 (thiscall): the thing constructor (Thing_Construct, cell 1, kind 0x11), then the receiver added to the message centre's slots
// 0x12, 0xa, 7, 8, 0x1b, 0xf, 0x13 and 0x1c in that order. Returns it. (The cell the original pushes has this's high address bits above the 1.)
u8 *__fastcall UIMsgReceiver_Ctor(u8 *receiver);
}
