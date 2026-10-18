// The surface pool at 0x8c47d8 (name unconfirmed): two DirectDraw surfaces for each side 0x100 down to 1 (the dwords at +0, two per
// side by SurfacePool_SideIndex), handed out in turn (+0x80 + 4 * index: which of the two is next). 0x58b0b0 makes one (a 0x7c-byte
// surface description, caps 0x1800), 0x58b280 releases them all (its atexit 0x58b0a0). Reimplementation of 0x5848b0 and 0x58b1f0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5848b0 (cdecl; name unconfirmed): a power-of-two side's index, 0x100 -> 0 down to 1 -> 8; any other side -1. The texture cache
// (0x58d1fd, 0x58d6e4) uses it too.
s32 __cdecl SurfacePool_SideIndex(s32 side);

// 0x58b1f0 (thiscall; side, an unused dword; pops 8; name unconfirmed): the side's index (SurfacePool_SideIndex); the turn at +0x80 +
// 4 * index becomes (turn - 1) & 1, then the dword at +8 * index + 4 * the old turn (read after that write) is returned.
u32 __fastcall SurfacePool_Next(u8 *pool, void *, s32 side, u32 unused);
}
