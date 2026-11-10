// The Direct3D driver's texture coordinate offset found: known cards by their ids, others by drawing a striped texture and reading it back. Reimplementation of 0x583cc0, 0x583e90 and 0x584310.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// The callees, by the game's addresses (the driver's: recorders). nullptr restores the game's.
void UvProbe_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x583cc0 (thiscall on the settings at 0x8bd550; names invented): +0x40e0..+0x40ec cleared. Only with driver kind 1, by the ids at +0x428 (vendor), +0x42c (device) of [0x879a24]: ATI's 0x4c42 and 0x524c set [0x8bcbc8] bit 2; a Permedia 2 sets it too and the
// offsets +0x40e8, +0x40ec to 0.45; a TNT (0x10de, 0x20) has its driver's caps read through the surface at [0x879da8] + 8 (slot 3, failure logged, sic: its out structure is read all the same) and bit 4 clear at +0xf4 sets the bit too and the offsets to 0.25, as a
// [0x8bcbc8] & 0x8000002 does. Else the two probes, across and then down.
void __fastcall UvProbe_Detect(u8 *settings);

// 0x583e90 (thiscall; pops none): the offset down the screen, +0x40ec; a texture striped in rows drawn and read back at 60 lines in 12 halvings from 0, twice: first from the lowest place TexFormat_Classify answers 0 (whose last and first are kept), then going up from there
// (its last kept). Half the sum of the last found and the first, or of the two searches' lasts; 0 if none answered 0. No texture memory: nothing.
void __fastcall UvProbe_Down(u8 *settings);

// 0x584310 (thiscall): the same with the stripes in columns and the lines read across, +0x40e8.
void __fastcall UvProbe_Across(u8 *settings);
}
