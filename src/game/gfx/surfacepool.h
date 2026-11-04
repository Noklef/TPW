// The surface pool at 0x8c47d8 (name unconfirmed): two DirectDraw surfaces for each side 0x100 down to 1 (the dwords at +0, two per
// side by SurfacePool_SideIndex), handed out in turn (+0x80 + 4 * index: which of the two is next). 0x58b0b0 makes one (a 0x7c-byte
// surface description, caps 0x1800), 0x58b280 releases them all (its atexit 0x58b0a0). Reimplementation of 0x5848b0 and 0x58b1f0.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

extern "C" {

// 0x5848b0 (cdecl; name unconfirmed): a power-of-two side's index, 0x100 -> 0 down to 1 -> 8; any other side -1. The texture cache
// (0x58d1fd, 0x58d6e4) uses it too.
s32 __cdecl SurfacePool_SideIndex(s32 side);

// 0x58b1f0 (thiscall; side, an unused dword; pops 8; name unconfirmed): the side's index (SurfacePool_SideIndex); the turn at +0x80 +
// 4 * index becomes (turn - 1) & 1, then the dword at +8 * index + 4 * the old turn (read after that write) is returned.
u32 __fastcall SurfacePool_Next(u8 *pool, void *, s32 side, u32 unused);
}

// (Test seam: the fncmp driver's table replaces the calls by address.)
void SurfacePool_SetCalls(const AddrCall *table, u32 count);

// 0x58b0b0 (thiscall (side, format), pops 8; name invented, unconfirmed): the side's two surfaces at +8 * index made. The format's record (TexFormat_Record on [0x879da8]): none gives 0. Each: a description (0x7c bytes, flags 0x1007, height and width the side, the record's 0x20 bytes at +0x48, caps 0x1800),
// the device's (+8 of [0x879a24]) slot 0x18, the surface's slot 0x94 with 0. Each result kept at [0x879cd4], a negative one logged; not 0 pushed on the engine's list (kind 3, 0x879ad0) and 0. Returns 1.
extern "C" {
u32 __fastcall SurfacePool_MakeSurfaces(u8 *pool, void *, s32 side, u32 format);
// 0x58b280 (thiscall; name invented, unconfirmed): each of the 32 surfaces set: its slot 0x98 (0), its Release, a Release's failure logged, the pointer 0. Returns 1.
u32 __fastcall SurfacePool_ReleaseAll(u8 *pool);
}
