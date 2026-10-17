// The things' map ids and the people (Sprite, the moving-person base serialized at 0x4f8b10, and
// Visitor, thing model 1, at 0x4fb530): reimplementation of 0x4e2650, 0x4f9460, 0x4fd950 and
// 0x4fdc60, 0x501290 and 0x502430 (thiscall). A thing's mX and mY (+4, +6) are 8.8 fixed point; its map id is the cell's
// row (mY's high byte) * 128 + column (mX's high byte) + 1. Sprite +0xcc mLastRecordedMapId;
// Visitor +0x1d4 mArrivalDate (a world tick), +0x1e8 mPreviousRides (u16[4], newest first),
// +0x1f1 mQueuePos (a byte), +0x220 mState, +0x224 mSavedState.
#pragma once

#include "../../../re/types.h"

struct Stream;

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
// 0x50bad0 (thiscall; stream, save, version, unused): mActionHistIndex (+0x88), the 32 action history pairs (dwords +4..), +0, mThoughtScript
// (+0x84) and mTimeBubbleShown (+0x8c) saved or loaded as a serializer does (Save_Field; the load reads the last through Save_ReadU32, the same).
u32 __fastcall Thoughts_Serialize(u8 *thoughts, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x5065d0 (thiscall; word, pops 4): Person_HasWord (world/person); when it holds: 1 for kind 5 with the dword at +0x19c equal to 0xb and for kind 4 with
// it equal to 0xd, otherwise 0.
u32 __fastcall Person_HasWordInState(const u8 *object, void *, u32 word);

// 0x5012b0 (thiscall): the dword at +0x1f8 = 1 (the original also reads the handle at +0x1dc into its debug print).
void __fastcall Visitor_Set1f8One(u8 *object);

// 0x506580 (thiscall; word, pops 4): for the kind byte at +2 being 4 the word at +0x218, for 5 the word at +0x21a, is equal to the low 16 bits of
// word (1 or 0); any other kind gives 0 (the original reports it to its empty debug printf).
u32 __fastcall Person_HasWord(const u8 *object, void *, u32 word);

// 0x518e20 (thiscall; one unused stack word, pops 4): the thoughts at +0x30 (Thoughts_Ctor), the navigation block at +0xd4 (Nav_Ctor) and the dwords at +0x1f0 .. +0x1fc cleared.
// Returns the object.
u8 *__fastcall Person_BaseCtor(u8 *object, void *, u32 unused);

// 0x518e00 (thiscall; one stack argument, unused; name unconfirmed): Person_BaseCtor without its four cleared dwords: the thoughts (+0x30) and the navigation block (+0xd4)
// constructed; returns the object. Its one caller builds a 0x22c-byte object with it.
u8 *__fastcall Person_PartsCtor(u8 *object, void *, u32);
}
