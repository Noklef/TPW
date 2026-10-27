// The font files the game adds to Windows for its run (AddFontResource; the global vector of their names at 0x789c1c, begin / end /
// capacity, of 0x14-byte Strings): reimplementation of its exit-time destructor (0x412bc0).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x412bc0 (cdecl; the atexit handler the static initialiser 0x412b90 registers; name unconfirmed): each String from [0x789c1c] to
// [0x789c20] (0x14 bytes apart; the end read once) is told slot 0xc with 0 (its destructor), the block (the begin) freed (Mem_Free, also
// when null) and the begin, end and capacity (0x789c24) cleared.
void __cdecl FontFiles_Destroy();
}
