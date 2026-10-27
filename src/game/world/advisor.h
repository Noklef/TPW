// The advisor's responses: reimplementation of the lookup 0x5997b0 in the response table at 0x768fb8 (entries of 0x20 bytes, ended by
// the id 0x270f), which Advisor_Say (0x599050, "Advisor says ResponseID %d") also walks.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

extern "C" {

// 0x5997b0 (cdecl; id): the table of 0x20-byte entries at 0x768fb8, ended by an id of 0x270f, searched
// for the id; the dword at +0x1c of the entry found, or 0x17f for none.
u32 __cdecl Table768fb8_Lookup(u32 id);

// 0x5989c0 (cdecl; name unconfirmed): the advisor's 0x168-byte block at 0xf79660 zeroed, its count at 0xf79770 set to 5 and 0xf7978c to -1;
// for model slots 0 and 1 that have a holder, the indices of its mouth parts 1 to 5 (Model_FindMouthPart; as many as the count) from
// 0xf796a0 + 0x14 slot and of its "bug head" (-1) at 0xf79698 + 4 slot. Then with slot 1's holder (or slot 0's), the end times of motion bank
// 5's entries 0-19 (Animator_MotionEndMs) from 0xf796c8, the 14th cleared again, and the particle pools reset (ParticlePools_Reset); none of
// that without a holder.
void __cdecl Advisor_Init();

// 0x599e70 (thiscall): the advisor thing's constructor (thing model 11, whose serializer is 0x599f60): the thing constructor (Thing_Construct,
// cell 1, kind 0xb), +0xc and +0x10 cleared, the bytes at +0x28 + 0x18 k (k < 8) cleared, +0xd4 and +0xdc cleared;
// the 0x15f records of 0x10 bytes from +0xe0 get {0, -1, byte 0 (the other three bytes kept), 0}; the byte at +0x16e4 cleared; the advisor
// added to the message centre's slots 5, 0x11, 0x13, 0x17, 0xe, 8 and 0x19 in that order; then +0x10, +0xd4, +0xdc and +0xd8 cleared. Returns
// it. (The cell the original pushes has this's high address bits above the 1.)
u8 *__fastcall Advisor_Ctor(u8 *advisor);

// 0x59abc0 (thiscall (id, urgent), pops 8; name unconfirmed): whether a message of id may go to the advisor's queue (gAdvisorScoring, which
// the lookups take as this, built first each time as AdvisorScoring_Instance does). Not, as AL 0: unless the byte 0x78d90d is set, id's record
// is active (Record_IsActive); the advisor's record for id (+0xe0 + 0x10 id) has a time (Obj_Get0Shr2) and the game's time since it
// (Game1da70c_MinusAt0Quarters) is below (unsigned) the wait (Table76e300_GetObjectDword); not urgent, the scoring's flag is set
// (Table76e300_ObjectDwordSet) with the record's byte +8, or its limit is reached by the record's +0xc (Table76e300_LimitCheck); the queued
// messages of id (the 8 slots of 0x18 bytes from +0x14, set (+0x14) with +0xc id) at or above the table's byte (Table76e300_GetByte); with
// DefField2c 0, the record's +4 equal to DefField28 - 1. Otherwise 1.
u8 __fastcall Advisor_MayPost(u8 *self, void *, u32 id, u8 urgent);

// 0x59a940 (thiscall (message), pops 4; name unconfirmed): an advisor message (see AdvisorMsg_Init) offered to the queue. With DefField8 of
// its id 1 (gAdvisorScoring built first) it is copied to +0x16d0. Unless Advisor_MayPost (id, the byte +0x15) it is refused (0). Otherwise it
// goes to the first slot not set (+0x14 + 0x18 k, its byte +0x14 0) and 1; with none, the set slot of the lowest score (signed, the first
// such) is replaced when its score (+0x10, -1 when not set) is below the message's (likewise), and 1 either way (0 with no set slot at all).
u8 __fastcall Advisor_Post(u8 *self, void *, const u8 *message);

// 0x59afc0 (thiscall (event), pops 4; name unconfirmed; Advisor_HandleMessage's case for message 0x17): an advisor message of type 0, a 0,
// b -1, flag 0 and no score (AdvisorMsg_Init) for the id 5 n + 0x22, 0x21, 0x24, 0x23 or 0x25 as the event's kind (+8) is 0-4 (n its +0xc;
// another kind: the event's address, sic) posted (Advisor_Post, whose answer is returned). The message is on the stack, its bytes +2, +3
// and +0x17 never written: the queue slot it lands in gets whatever they held.
u8 __fastcall Advisor_PostKindEvent(u8 *self, void *, const u8 *event);

// 0x486b00 (cdecl (id); name unconfirmed): the same message for the id (type 0, a 0, b -1, flag 0, no score; on the stack, its bytes +2, +3
// and +0x17 never written) posted to the advisor of the world [0x7cf83c] (GameTable7cfb90_Get1da71c_5194d0); Advisor_Post's answer.
u8 __cdecl Advisor_PostId(u32 id);

// 0x59ae20 (thiscall (code), pops 4; name invented, unconfirmed; Advisor_HandleMessage's case for message 0x13): code 0 posts the message
// id 0 and, with the mode setting (0xfb3b7c, read under the guard 0xfb2765) 2, id 0x143; 2, 3, 4 post 0x6a, 0x80, 0x81 (each an advisor
// message of type 0, a 0, b -1, flag 0 and no score, AdvisorMsg_Init, on the stack: its bytes +2, +3 and +0x17 never written; Advisor_Post);
// 10 says "Clearing advisor history ..." (OutputDebugStringA, 0x76e190) and clears the 0x15f history records (+0xe0, 0x10 apart: +4 -1,
// the byte +8 0, +0xc 0); any other nothing.
void __fastcall Advisor_HandleCode(u8 *self, void *, u32 code);

// 0x59b060 (thiscall (event), pops 4; the advisor thing's message handler, Thing_HandleMessage's case 11): the event's code (its slot 0)
// 5: with the byte +0x1c, the message at +8 posted (Advisor_Post); 8: id 0x102 of type the word +8, a 0; 14: of type the word +8 (w), a
// the low word of (the byte +7 << 7) + the byte +5 + 1 of the world table entry [0x7cfb90 + 20 w], id 0x7d, 0x7e, 0x7f as +0xc is 0, 1,
// 2 (else nothing); 17: of type 0, a the word +8, id 0x5d, 0x5f, 0x60, 0x61, 0x5e as +0xc is 0-4 (else nothing); 19: Advisor_HandleCode
// (the dword +8); 23: Advisor_PostKindEvent; 25: unless +0xc is 0x160, id +0xc of type 0, a 0; any other code: the code asked again (for
// an empty debug print). Each message has b -1, flag 0 and no score (AdvisorMsg_Init; 8 and 14 inlined), on the stack (bytes +2, +3,
// +0x17 never written), and posted (Advisor_Post).
void __fastcall Advisor_HandleMessage(u8 *self, void *, u8 *event);

// 0x599f60 (thiscall (stream, save, version, unused), pops 0x10): the advisor thing's serializer. 1 when Thing_Serialize fails; 0 for a version
// outside 0..1; then its time stamp at +0xc (TimeSig_Serialize), mSoundId (+0x10), the 8 queued messages from +0x14 (AdvisorMsg_Serialize, 0x18
// bytes apart), mNextAvailableQueuePos (+0xd4), mLastActionStarted (+0xd8), mLastActionDuration (+0xdc), the 0x15f history records of 0x10
// bytes from +0xe0 (mHistory[i]), the message at +0x16d0 (AdvisorMsg_Serialize), mRequestedLastTutorial (the byte +0x16e8) and the dword at
// +0x16ec, each saved or loaded as a serializer's field (the last loaded through Save_ReadU32); 1 at the first that fails, else 0.
u32 __fastcall Advisor_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
