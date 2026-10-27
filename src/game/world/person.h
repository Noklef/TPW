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

// 0x4fa670 (thiscall (flag), pops 4; name invented): the dword +0x198 (the timer Sprite_SetDestination and Person_PlaceSprite clear) the
// next count of 0x7cdb98 (Counter7cdb98_Inc) for a flag, else 0. (The debug print of the handle, Thing_GetHandle's, is left out.)
void __fastcall Sprite_SetStranded(u8 *sprite, void *, u32 flag);

// 0x4fa530 (thiscall; a cell id; name unconfirmed): Sprite_SetDestination to the cell's centre: x (id - 1) & 0x7f and y its bits 7-14, each the
// high byte of a word whose low byte is 0x80.
u32 __fastcall Sprite_SetDestinationCell(u8 *sprite, void *, u32 cell);

// 0x4f7de0 (thiscall; x, y; name unconfirmed): Sprite_SetDestination; when it answers nonzero the dword +0x39c and the dword +0x10 (+0xc's
// second, Set4_4217f0) set to 1, and 1; else 0.
u32 __fastcall Sprite_SetDestinationMarked(u8 *sprite, void *, s32 x, s32 y);
// 0x4f7da0 (thiscall; a cell id; name unconfirmed): the same with Sprite_SetDestinationCell.
u32 __fastcall Sprite_SetDestinationCellMarked(u8 *sprite, void *, u32 cell);

// 0x4f9f00 (thiscall; t, unit, &x, &height, &z, &frame, pops 0x18; x87; name unconfirmed): a moving person's position t of the way from the last one
// (the dwords +0x190, +0x194) to its nav's (+0xd4, Nav_GetPos), t held to 0..1 for a nonzero unit, else to -1..2 (NaN to the low end): each
// __ftol(the difference (wrapping) * t + the last) in the x87's 24-bit precision, its low dword. x and z are those times 10 over 65536
// (16.16 cells to the scene's units) stored to floats, the height 0, the frame the dword +0x1c.
void __fastcall Sprite_InterpolatedPos(const u8 *sprite, void *, float t, u32 unit, float *x, u32 *height, float *z, u32 *frame);

// 0x4faa00 (thiscall; category, sound, pops 8; name unconfirmed): Sound_PlayAt(0, category, sound, x, y, z) at the sprite's script's position
// (+0xc, SpriteScript_GetPosition: its float registers, each __ftol'd to its low dword); its result. Without the script (or the list not ready)
// the original passes its uninitialised locals; the port passes 0s.
u32 __fastcall Sprite_PlaySound(const u8 *sprite, void *, u32 category, u32 sound);

// 0x4f8940 (thiscall; cell, kind, pops 8; name unconfirmed): a moving person's constructor: the thing (Thing_Construct with the cell and kind),
// the script reference at +0xc (Init_4d3ed0), the dwords +0x1c..+0x28 and the byte +0x2c 0, the thoughts at +0x30 (Thoughts_Ctor), the words +0xc0,
// +0xc2, +0xc4, +0xcc and the dwords +0xc8, +0xd0 0, the nav at +0xd4 (Nav_Ctor), the dwords +0x190..+0x198 0; then the word +0xc0 one of the five
// at 0x75c7f8 by World_Rand (of [0x80239c]) % 5, +0xc2 the word 0x75c7f0, the person added to the message centre's slot 0xa (MessageCentre_Instance,
// inlined in the original), +0xcc the cell, the nav placed at the middle of the cell (column (id - 1) & 0x7f and row (id - 1) >> 7 of the cell's
// low word, in 1/65536 cells; no speed: Nav_SetPosVel) and the position read back to +0x190 / +0x194 (Nav_GetPos); +0x28, +0xd0 and +0x24 0 again.
// Returns the person.
u8 *__fastcall MovingPerson_Ctor(u8 *sprite, void *, u32 cell, u32 kind);

// 0x4f8b10 (thiscall; stream, save, version, unused): a moving person: 1 when the thing (Thing_Serialize) or the script reference (+0xc,
// SpriteAnim_Serialize) fails. Then, for a version of 0..1, saved (save nonzero) or loaded: the words +0x18, +0x1a, +0xc4, +0xc0, the byte +0x2c,
// the dword +0x24, the word +0xcc, the nav (+0xd4: Nav_Serialize; loading Nav_SerializeFwd with save 0), the dwords +0xc8, +0x190, +0x194, +0x198,
// the word +0xc2, the dwords +0xd0, +0x1c, +0x20 (loaded: then taken modulo the folder +0x24's count, SpriteFolder_Count, an unsigned divide that
// faults on 0) and +0x28, and the thoughts (+0x30, Thoughts_Serialize); 1 at the first that fails (the load's first four fields through Save_Field,
// the rest through the readers, not quiet). Else (or a version outside 0..1) Sprite_SetDestination to the words +0x18 / +0x1a, and 0.
u32 __fastcall Sprite_Serialize(u8 *sprite, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4fb530 (thiscall; the same arguments): a visitor: Sprite_Serialize first (1 if it fails); for a version of 0..1 then, saved or loaded as a
// serializer does (Save_Field; the load from mBeenAdmitted on through the readers): mArrivalDate, mArrivalIndex, mBalloonScript, mBeenAdmitted,
// mCash, mExitLevel, mHappiness, mHunger, mLastPosX, mLastPosY, mLitter, mMajorDest, mNumRides, mNumShops, mNumSideshows, mNumSideshowsWon,
// mPaidAdmission, mParkOpeningWaitingTime, mPersonType, mPrankeryIndex, mPreviousRides[i] and mPreviousTemporaryRides[i] for each i, mQNext,
// mQPrev, mQueueMoveDelay, mQueuePos, mRemainingBalloonLife, mSavedMajorDest, mSavedState, mState, mThirst, mTimeOfLastSpotAnim,
// mTimeStartedIdling, mTiredness, mToilet and mVomit (re/types.h's Visitor). The stats (floats) are saved as the float of their __ftol's low byte
// and loaded through a temporary, stored only when read whole. 1 at the first that fails, else 0.
u32 __fastcall Visitor_Serialize(u8 *visitor, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4fde50 (thiscall; an object; name unconfirmed): 0 for a free object (Obj_Get194 0). Else what the visitor would pay: the definition's
// effects (+0x150 happiness, against 100 less the __ftol low byte of mHappiness; +0x144 thirst, +0x148 hunger, +0x14c vomit, less, against the
// stats') each times over 100 (signed), plus 100, times 115% of the first cost (+0x140), over 100; plus the object's +0x188 times its byte +0x190
// over 100 for a definition of kind (+0x4ac) 2, times 100 + the controls' +0xc, over 100; times 100 + the happiness byte, over 100 (unsigned from
// the first product on). The margin ((that + 5) - price) * 10, at most 100 (unsigned), goes to the park analyser's ring (ParkAnalyser_Record on
// the game table entry of [0x7cf83c]'s +0x1da720): for kind 1 the ingredient's (+0x158, 1-4) and the appearance's (+0x15c: 1 -> 6, 2 -> 7),
// for kind 2 ring 9. 1 when the price is above that or above mCash (+0x1a0), else 0.
u32 __fastcall Visitor_WontPay(const u8 *visitor, void *, u8 *object);

// 0x4fd4e0 (thiscall (object), pops 4; name unconfirmed): 100 for an object in poor condition (Object_InPoorCondition); else the gap
// between the visitor's wanted rating (the byte of 0x7850e4's 12-byte row +0x1f0) and the object's (Object_Rating(0)), its size held to
// 50, negative when the wanted one is at least the object's.
s32 __fastcall Visitor_RatingGap(const u8 *visitor, void *, u8 *object);

// 0x4fa030 (thiscall; t, pops 4; x87; name unconfirmed): a moving person's sprite placed for the moment t: its position at t - 0.35 (unit 0) and at t
// (unit 1) by Sprite_InterpolatedPos. Unless +0x28 is set, the script reference +0xc's slot is moved there (SpriteScript_SetPosition; r15 the
// frame less 0x380, & 0x7ff, >> 8). The timer +0x198 is zeroed when the counter 0x7cdb98 (Counter7cdb98_Inc) is below it; while it is set, a square
// face (DynFaces_AddSquare, tag 1) on the person's cell (column and row times 10.0f). The slot +0xb4 (when set) goes to the position 2.5 higher (r15
// 0). For a person of kind 1 (byte +2) with the script +0x210, that is moved (Person_MoveScript) to the position at t - 0.35 plus a row of the
// bob table 0x75c810 (three floats times 1.5; the row the handle plus 0x7cedd4, modulo the dword 0x75c80c), its height plus
// 1 - (|dx| + |dz|) / 0.5 between the two positions; then the bob's time 0x7cedd8 gains 0.1 until it is 1.0 or more, when it is 0 and the
// row 0x7cedd4 moves on (modulo the count). All x87 at 24 bits.
void __fastcall Person_PlaceSprite(u8 *person, void *, float t);

// 0x518f90 (thiscall on the world [0x7cf83c], unused; t, pops 4; name unconfirmed): over the thing table's chain (the first entry [0x7cf56c]'s
// handle +4, each thing's next by its own handle: entry +8, 0 ends it; a null thing faults at its handle as the original does) every person
// (kind 1, 4, 5, 6, 7, 8 or 0x12) whose byte +3 is clear has its sprite placed for t (Person_PlaceSprite).
void __fastcall World_PlaceSprites(u8 *world, void *, float t);

// 0x4faa60 (thiscall; name unconfirmed; called from 0x4f8af9 before the history at +0x30 goes, 0x50bab0): a debug dump of the action history (the
// thoughts at +0x30): from the 32nd entry from the end to the last (Ring_FromEnd), the thing its handle (+2) names (its definition id +0xe when an
// object, its cell +5 / +7) and the action (+0, by its kind 0..0x19) to the empty Debug_Printf: only the reads remain. Returns -1 (the loop's
// counter, left in EAX; the caller ignores it).
u32 __fastcall Person_DumpActionHistory(u8 *person);

// 0x4fa870 (thiscall; name unconfirmed; called from 0x4f7c63, 0x501658, 0x505495): the speed +0xc8 = ((+0xc0 + +0xc2 + +0xc4, words) / the word
// at 0x75c7fc - +0xc8 * -3.0) * 0.25 (x87), passed to the nav (+0xd4: Nav_SetSpeed); the word +0xc4 = its product with 99 (16 bits) / 100; the
// position +0x190 / +0x194 read back from the nav (Nav_GetPos), and the script reference +0xc applied (ScriptRef_Apply).
void __fastcall Person_UpdateSpeed(u8 *person);
}
