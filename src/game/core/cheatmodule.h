// The cheat system's and the advisor scoring's saved modules (CHTS, ADSC): reimplementation of 0x405570,
// 0x4055d0, 0x59c860, 0x59c890 and 0x599c30.
//
// Two bytes of the cheat system, at +0xa and +0xb, are read from / written to a chunk's stream (the
// chunk's first dword).
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x405570 (thiscall; chunk, pops 4): reads the byte at +0xa then the one at +0xb from the chunk's
// stream. 0 when the first read fails (a negative result), else 1 unless the second does.
u32 __fastcall CheatSystem_Load(u8 *cheats, void *, const u32 *chunk);

// 0x4055d0 (thiscall; chunk, pops 4): the same, writing.
u32 __fastcall CheatSystem_Save(u8 *cheats, void *, const u32 *chunk);

// 0x599c30 (cdecl; stream): the advisor module (ADVS) written: the tag 'ADV_' (0x5f564441) as a dword, the
// size 0x168 as a dword, the 0x168 bytes at 0xf79660, then, when the pointer at 0xf797a0 is set, the
// dword count at 0xf79794 and that many bytes from the pointer; otherwise a zero dword.
void __cdecl Advisor_Save(Stream *stream);

// 0x59c860 / 0x59c890 (thiscall; chunk, pops 4): the advisor scoring's first dword read from / written to
// the chunk's stream (the chunk's first dword). 1 unless the result is negative, then 0.
u32 __fastcall AdvisorScoring_Load(u8 *scoring, void *, const u32 *chunk);
u32 __fastcall AdvisorScoring_Save(u8 *scoring, void *, const u32 *chunk);

// 0x4053b0: sets +0xa, +0xb, +0x9; returns the object.
u8 * __fastcall CheatSystem_Construct(u8 *o);
}
