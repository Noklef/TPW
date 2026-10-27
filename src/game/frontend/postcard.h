// The postcard sender's result (the code that writes postcard.jpg, 0x4a8600, gives its sender object, built by 0x5c99f0, this callback at
// +0x90). Reimplementation of 0x4a7bf0. The names are invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4a7bf0 (cdecl (done, result)): when done, the result 0-4 posted as the command 0x34, 0x39, 0x3a, 0x3b or 0x3b (CommandQueue_Post with
// 0); nothing for others.
void __cdecl Postcard_OnSent(u32 done, u32 result);
}
