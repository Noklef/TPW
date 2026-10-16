// The cheat system's saved module (CHTS): reimplementation of 0x405570 and 0x4055d0.
//
// Two bytes of the cheat system, at +0xa and +0xb, are read from / written to a chunk's stream (the
// chunk's first dword).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x405570 (thiscall; chunk, pops 4): reads the byte at +0xa then the one at +0xb from the chunk's
// stream. 0 when the first read fails (a negative result), else 1 unless the second does.
u32 __fastcall CheatSystem_Load(u8 *cheats, void *, const u32 *chunk);

// 0x4055d0 (thiscall; chunk, pops 4): the same, writing.
u32 __fastcall CheatSystem_Save(u8 *cheats, void *, const u32 *chunk);
}
