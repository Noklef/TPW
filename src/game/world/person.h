// The things' map ids and the people (Sprite, the moving-person base serialized at 0x4f8b10, and
// Visitor, thing model 1, at 0x4fb530): reimplementation of 0x4e2650, 0x4f9460, 0x4fd950 and
// 0x4fdc60, 0x501290, 0x502430, 0x4fec10, 0x4fe9c0, 0x4fea70 and 0x4fb4f0 (thiscall). A thing's mX and mY (+4, +6) are 8.8 fixed point; its map id is the cell's
// row (mY's high byte) * 128 + column (mX's high byte) + 1. Sprite +0xcc mLastRecordedMapId;
// Visitor +0x1d4 mArrivalDate (a world tick), +0x1e8 mPreviousRides (u16[4], newest first),
// +0x19c a float kept to 0..100, +0x1f1 mQueuePos (a byte), +0x220 mState, +0x224 mSavedState.
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

// 0x4fec10 (thiscall; name unconfirmed): whether mState is 6 to 10 (others are reported to the empty Debug_Printf).
u32 __fastcall Visitor_IsState6To10(const u8 *visitor);

// 0x4fe9c0 / 0x4fea70 (thiscall; index, pops 4; names unconfirmed): the float at +0x19c raised / lowered by the global byte for index
// (Bytes785058_Get's, inlined: 0 past 2) in x87 single precision, then kept to 0..100: above 100 it is 100, below 0 (or NaN) 0.
void __fastcall Visitor_Raise19c(u8 *visitor, void *, u32 index);
void __fastcall Visitor_Lower19c(u8 *visitor, void *, u32 index);

// 0x4fb4f0 (thiscall on a float; delta, pops 4; name unconfirmed): the float plus delta in x87 single precision, kept to 0..100 as
// Visitor_Raise19c's (the visitor's 0..100 values: +0x1b8 and others).
void __fastcall Stat100_Add(float *stat, void *, float delta);

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

// 0x4fa5f0 (thiscall; x, y in 1/256 cells, their low words; name unconfirmed): a moving person sent to the point. The dword +0x198 zeroed when
// above the next count of 0x7cdb98; when still set and Obj_BlockValuesBelowLimit, 0. Else the words +0x18 and +0x1a the point, +0x198 zeroed,
// and Nav_PathToPoint of the nav at +0xd4 to it (16.16, each << 8); its result.
u32 __fastcall Sprite_SetDestination(u8 *sprite, void *, s32 x, s32 y);

// 0x4fa530 (thiscall; a cell id; name unconfirmed): Sprite_SetDestination to the cell's centre: x (id - 1) & 0x7f and y its bits 7-14, each the
// high byte of a word whose low byte is 0x80.
u32 __fastcall Sprite_SetDestinationCell(u8 *sprite, void *, u32 cell);

// 0x4f7de0 (thiscall; x, y; name unconfirmed): Sprite_SetDestination; when it answers nonzero the dword +0x39c and the dword +0x10 (+0xc's
// second, Set4_4217f0) set to 1, and 1; else 0.
u32 __fastcall Sprite_SetDestinationMarked(u8 *sprite, void *, s32 x, s32 y);
// 0x4f7da0 (thiscall; a cell id; name unconfirmed): the same with Sprite_SetDestinationCell.
u32 __fastcall Sprite_SetDestinationCellMarked(u8 *sprite, void *, u32 cell);

// 0x4f8b10 (thiscall; stream, save, version, unused): a moving person: 1 when the thing (Thing_Serialize) or the script reference (+0xc,
// SpriteAnim_Serialize) fails. Then, for a version of 0..1, saved (save nonzero) or loaded: the words +0x18, +0x1a, +0xc4, +0xc0, the byte +0x2c,
// the dword +0x24, the word +0xcc, the nav (+0xd4: Nav_Serialize; loading Nav_SerializeFwd with save 0), the dwords +0xc8, +0x190, +0x194, +0x198,
// the word +0xc2, the dwords +0xd0, +0x1c, +0x20 (loaded: then taken modulo the folder +0x24's count, SpriteFolder_Count, an unsigned divide that
// faults on 0) and +0x28, and the thoughts (+0x30, Thoughts_Serialize); 1 at the first that fails (the load's first four fields through Save_Field,
// the rest through the readers, not quiet). Else (or a version outside 0..1) Sprite_SetDestination to the words +0x18 / +0x1a, and 0.
u32 __fastcall Sprite_Serialize(u8 *sprite, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
