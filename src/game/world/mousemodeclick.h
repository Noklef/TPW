// What two mouse modes do with the tile under the pointer (the tile's id at [0x7b05cc], its record at the level + 0x294 + id * 0x44): reimplementation of 0x46cc40, 0x46cdc0, 0x46d000 and 0x46d0d0.
// All thiscall, this unused but for Cancel's. Names invented, unconfirmed: the class of each is told only by its place after that class's constructor.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Their calls by address (the bench's recorders).
void MouseModeClick_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x46cc40 (two arguments unread, pops 8): unless the tile is flagged 0x40 or answers 0x536450 or 0x5363f0, and when it answers any of 0x536390, 0x536310, 0x536320: Holder_PlaceStaff (the level's 0x519450; the id),
// then the plain mode (vtable 0x6fea10, +4 1) set on the holder [0x7b05e8], made when missing as MouseModeHolder_Ctor makes it.
void __fastcall MMStaffDrop_Click(void *, void *, u32, u32);

// 0x46cdc0: the sprite +8 killed if any and cleared; while the level's 0x519450 has a thing at +0x60 (a word), Holder_PlaceStaff with the word id of that thing's record ([0x7cfb90 + id * 0x14]: its +5 + +7 * 0x80 + 1).
void __fastcall MMStaffDrop_Cancel(u8 *self);

// 0x46d000 (four arguments unread, pops 0x10): nothing without a tile. GameMode_MapPreview (id - 1 split at 7 bits, 0); the cursor 0x13 over a tile that passes (see the .cpp), else 8.
void __fastcall MouseMode6fead0_Hover(void *, void *, u32, u32, u32, u32);

// 0x46d0d0 (two arguments unread, pops 8): the cursor 1. A tile whose Map_ObjectOrEntranceAt finds a thing with no window (+0x4ac of its 0x4dd4e0): 0x4e15b0 (0) on it and the plain mode (when [0xfb3b7c] is not 1; else the online
// one, the cursor 0) set on the holder, made by MouseModeHolder_Ctor; that ends it. Else a tile that passes: Camera_ToggleWalkMode, the same mode set, the holder made inline, the cursor 0; one that fails: the cursor 8.
void __fastcall MouseMode6fead0_Click(void *, void *, u32, u32);
}
