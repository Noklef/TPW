// The challenges (thing model 19: 0x14 challenges of 0x30 bytes from +0xc, the current one's
// index at +0x3cc) and the BalanceChallenges file they load from (vtable 0x700398):
// reimplementation of 0x4d1c80, 0x4d1e60, 0x4d2840 and the offer's answers 0x4d2df0 / 0x4d2ec0 (thiscall).
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x4d1c80: each challenge in use (+0 set) that names another (+0x18 set) is dropped (+0 = 0)
// unless some other challenge in use has that +0, as the list stands at that point.
void __fastcall Challenges_DropOrphans(u8 *challenges);

// 0x4d2840: the current challenge (index +0x3cc, 0-19 signed), or null.
u8 *__fastcall Challenges_Current(u8 *challenges);

// 0x4d1a00 (name unconfirmed): a challenge (0x30 bytes) finished: its byte +0x2c = 1, and when it names no other (+0x18 clear) message
// 0x19 posted (a = Kind_ToIdC of its +0, b = 0x110, d = its +0x14, the rest 0) through the objects' post (Object_PostMessage).
void __fastcall Challenge_Finish(u8 *challenge);

// 0x4d2df0: an offer accepted: when waiting for the reply (+0x3da) and there is a current challenge, the wait ends, the challenge is on
// (+0x3d9 = 1) and mNextChallengeEventTime (+0x3d0, 8 bytes) = the world's funny time now (FunnyTime_Now) + its mTargetTime (+4) days.
void __fastcall Challenges_AcceptOffer(u8 *challenges);

// 0x4d2ec0: an offer declined: when waiting for the reply, the wait ends and the challenge is off; unless the current challenge is
// independent (+0x1d; an index out of range reads it from 0, a fault), the money record (the world's handle +0x1da726) is credited
// half its mPrize (+0x14, signed, toward 0; Money_Credit) and mActualThing (+0x3e0, a word) = 0; then the next event time = now + the
// days of gBalanceLevel.mDaysAfterDeclinedChallenge (0x7857c4).
void __fastcall Challenges_DeclineOffer(u8 *challenges);

// 0x4d1e60 (the vtable's first slot; index): the schema of a field, 0x739b98 + index * 0x3c;
// this is unused.
const u8 *__fastcall BalanceChallenges_GetSchema(const u8 *file, void *, u32 index);

// 0x4d1e80 (thiscall): a BalanceChallenges destroyed: its vtable (0x700398) back, then BalanceFile_Release.
void __fastcall BalanceChallenges_Dtor(u8 *file);

// 0x4d1ce0 (thiscall): the challenges loaded from data/challenges.sam. Every slot of the list is freed (+0 = 0); a BalanceChallenges on the
// stack (0x970 bytes: the schema's one group of 10 fields by 60 rows, 0x28 bytes a row from +0xc) is set up (BalanceFile_Init) and read
// (BalanceFile_Load, keep 0; a failed read only asserted). Then for each of gBalanceLevel's challenge indices (0x7857d4, 20 dwords)
// that is positive: row index of the file goes into the first free slot (+0 = 0) as mType (+0) = Type, mDoubleOrNothingFollowup
// (+0x18) = FollowupType, mTargetTime (+4) = TargetTime, mTargetVal (+8) = TargetVal, mTargetObj / mTargetObj2 (+0xc, +0xe, words) =
// the low words of TargetObj / TargetObj2, mTargetStaffType (+0x10) = TargetStaffType, mPrize (+0x14) = Prize, mCheckAtEndOnly (+0x1c)
// = CheckAtEndOnly != 0, mIndependent (+0x1d) = Independent != 0, +0x20..+0x2b, +0x2c and +0x2e (a word) 0; with no free slot the
// index is skipped. (The bytes +0x1e, +0x1f and +0x2d are uninitialised stack in the original: 0 here, sic.) Then the orphans are dropped
// (Challenges_DropOrphans) and the file destroyed (BalanceChallenges_Dtor).
void __fastcall Challenges_LoadSam(u8 *challenges);

// 0x4d0f90 (thiscall; stream, save, version, unused): a challenge's 0x30 bytes saved or loaded as a serializer does (Save_Field; the load
// path through the readers): mType, mTargetTime, mTargetVal, mTargetObj, mTargetObj2, mTargetStaffType, mPrize, mDoubleOrNothingFollowup,
// mCheckAtEndOnly (+0x1c), mChallengeOffered, mChallengeLost, mChallengeDeclined, mHaveWon (+0x2c), mIndependent (+0x1d),
// mThingIdForFollowup (+0x2e).
u32 __fastcall Challenge_Serialize(u8 *challenge, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4d2320 (thiscall; the same arguments): the thing (Thing_Serialize), its 0x14 challenges (Challenge_Serialize), both results ignored
// (sic), then mCurrentChallenge (+0x3cc), mNextChallengeEventTime (8 bytes), mSystemSwitchedOn, mChallengeOn, mWaitingForReply, mValue
// (+0x3dc) and mActualThing (+0x3e0).
u32 __fastcall Challenges_Serialize(u8 *challenges, void *, Stream *stream, u32 save, s32 version, u32 unused);

// 0x4d1bd0 (thiscall; name unconfirmed): the thing constructor (Thing_Construct, cell 1, kind 0x13; the original pushes this's high address
// bits above the 1), each of the 0x14 challenges' +0, +0x20, +0x24, +0x28 and the byte +0x2c zeroed, mCurrentChallenge (+0x3cc) = -1,
// +0x3d0, +0x3d4, the bytes +0x3d8..+0x3da, +0x3dc and the word +0x3e0 zeroed; the thing added to the message centre's slots 0x1a, 0xb and
// 0x11; then the challenges loaded (Challenges_LoadSam). Returns challenges.
u8 *__fastcall Challenges_Ctor(u8 *challenges);
}
