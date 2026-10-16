// The things' map ids and the people (Sprite, the moving-person base serialized at 0x4f8b10, and
// Visitor, thing model 1, at 0x4fb530): reimplementation of 0x4e2650, 0x4f9460, 0x4fd950 and
// 0x4fdc60, 0x501290 and 0x502430 (thiscall). A thing's mX and mY (+4, +6) are 8.8 fixed point; its map id is the cell's
// row (mY's high byte) * 128 + column (mX's high byte) + 1. Sprite +0xcc mLastRecordedMapId;
// Visitor +0x1d4 mArrivalDate (a world tick), +0x1e8 mPreviousRides (u16[4], newest first),
// +0x1f1 mQueuePos (a byte), +0x220 mState, +0x224 mSavedState.
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

// 0x501290: whether mState is 11 and mQueuePos 0.
u32 __fastcall Visitor_IsState11AtQueuePos0(const u8 *visitor);

// 0x502430: whether mState (mSavedState when it is 8) is 11 to 14.
u32 __fastcall Visitor_IsState11To14(const u8 *visitor);

// 0x50ba80: a person's thoughts (serialized at 0x50bad0) cleared: +0, the 32 u16 pairs of
// the action history (+4), mThoughtScript (+0x84), mActionHistIndex (+0x88) and
// mTimeBubbleShown (+0x8c). Returns them.
u8 *__fastcall Thoughts_Ctor(u8 *thoughts);
}
