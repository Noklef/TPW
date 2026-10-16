// The things' map ids and the people (Sprite, the moving-person base serialized at 0x4f8b10, and
// Visitor, thing model 1, at 0x4fb530): reimplementation of 0x4e2650, 0x4f9460, 0x4fd950 and
// 0x4fdc60 (thiscall). A thing's mX and mY (+4, +6) are 8.8 fixed point; its map id is the cell's
// row (mY's high byte) * 128 + column (mX's high byte) + 1. Sprite +0xcc mLastRecordedMapId;
// Visitor +0x1d4 mArrivalDate (a world tick), +0x1e8 mPreviousRides (u16[4], newest first).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e2650 (&out): the thing's map id to *out. Returns out.
u16 *__fastcall Thing_MapId(const u8 *thing, void *, u16 *out);

// 0x4f9460 (&out): the last recorded map id to *out, and the current one recorded. Returns out.
u16 *__fastcall Sprite_RecordMapId(u8 *sprite, void *, u16 *out);

// 0x4fd950: the world's tick (gWorld +0x1da70c) less the arrival's, over 4 (unsigned).
u32 __fastcall Visitor_TimeInPark(const u8 *visitor);

// 0x4fdc60 (ride): the previous rides moved down one (the oldest dropped), the ride first.
void __fastcall Visitor_PushPreviousRide(u8 *visitor, void *, u32 ride);
}
