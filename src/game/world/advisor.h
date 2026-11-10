// The advisor's responses: reimplementation of the lookup 0x5997b0 in the response table at 0x768fb8 (entries of 0x20 bytes, ended by
// the id 0x270f), which 0x599050 (advisorspeech.h; "Advisor says ResponseID %d") also walks.
#pragma once

#include "../../../re/types.h"
#include "../files/file.h"

struct AddrCall;
void Advisor_SetCalls(const AddrCall *table, u32 count);   // Advisor_Update's and Advisor_PostHeld's callees (the bench's recorders)

extern "C" {

// 0x598ad0 (cdecl; name invented, unconfirmed): 0xf7966c = 0; the speech's text 0xf797a0, when there is one: freed, or 0xf79bb8 = 0 for the static 0xf797d0; then 0xf797a0 = 0.
void __cdecl Advisor_ClearSpeechText();

// 0x5996d0 (cdecl (stop); name invented, unconfirmed): while speaking (0xf7966c), as Advisor_StopSpeech up to the voice, which goes by Sound_Stop when stop is set, else Sound_Forward6; no sound follows.
void __cdecl Advisor_EndSpeech(u32 stop);

// 0x59a550 (thiscall; name invented, unconfirmed): the advisor's update. gAdvisorScoring's next pick (0x59c680 (1)) with a score (0x59bf20) above its floor +0x20 is posted (0x59b590 builds the message, 0x59a940 posts).
// Then, when the real time (0x402d80 of 0x785970) has reached +0xd8 + +0xdc (unsigned, wrapping) and the byte +0x16e8 is not 1 (when 1: cleared, and done): the set slot (eight of 0x18 bytes from +0x14: score +0x10, set +0x14; the first of the highest)
// with a score above the floor is said (line +8, or when that is negative next_line's): 0x59b870 with the byte +0x16 set, else 0x59b620, (&length, line). When they answer: +0xd8 = the time, +0xdc = length + 1000, and the id's (+0xc) history record at +0xe0 + 0x10 id
// stamped (0x41a960), its last line +4 and byte +8 = 1, +0x16ec = id. No such slot: 0x41a990 on +0xc.
void __fastcall Advisor_Update(u8 *self);

// 0x59ab50 (thiscall; AL; name invented, unconfirmed): with the byte +0x16e4 set, the held message +0x16d0: score +0x10 = gAdvisorScoring's +0x32c, byte +0x16 = 1, the byte +0x16e5 = 1, posted (0x59a940); 1. Else 0.
u8 __fastcall Advisor_PostHeld(u8 *self);

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

// 0x5994e0 (cdecl; name invented, unconfirmed): the advisor's speech stopped. Only while it speaks (0xf7966c): its voice (0xf79680, the
// one read first) marked (Sound_SetVoicePending (voice, 0)); with 0xf797c4, the byte 0x78d90c and a voice, Sound_VoiceFunctor3 (voice)
// and 0xf797c4 cleared; 0xf7966c cleared, Sound_Set803acc (0), particle pool 0 emptied (ParticlePool_Resize (0, 0, 0)), the advisor's
// model slot (0xf79674) told (ModelSlot_KillAdvisor); its text (0xf797a0) freed (the CRT's free) unless it is the static 0xf797d0 (then
// 0xf79bb8 cleared), and 0xf797a0 / 0xf79794 / 0xf7979c cleared; the voice (re-read) Sound_Forward6 and cleared. With a voice at first,
// one of the sounds 0x27f-0x281 (rand () % 3) played (Sound_PlayAt, the owner the voice re-read: 0) in the category [0x803a34] at 0, 0, 0.
void __cdecl Advisor_StopSpeech();

// 0x598960 (cdecl; called from 0x4092e1 / 0x4093b8; name invented, unconfirmed): while the advisor speaks (0xf7966c), with the byte
// 0x78d90c and a voice (0xf79680): Sound_SendVoiceFunctor2 of the voice and 0xf797c4 = 1 (the speech held).
void __cdecl Advisor_PauseSpeech();

// 0x598990 (cdecl; called from 0x409332 / 0x40938a; name invented, unconfirmed): with the speech held (0xf797c4), the byte 0x78d90c and a
// voice (0xf79680): Sound_VoiceFunctor3 of the voice and 0xf797c4 = 0 (as Advisor_StopSpeech does first).
void __cdecl Advisor_ResumeSpeech();

// 0x59aa70 (thiscall (id), pops 4; name invented, unconfirmed): the advisor's messages of id withdrawn. With the byte +0x16e4, it is
// cleared when DefField8 of id is 1 (gAdvisorScoring built first) and +0x16dc is id. The queued slots of id (+0x14 + 0x18 k: the byte
// +0x14 set, +0xc id) unset. The message spoken: +0x16ec while the game clock's real time (Clocks_RealNow (0x785970)) is below (unsigned)
// +0xd8 + +0xdc, else 0x160; when it is id, Advisor_StopSpeech, the history record's byte +8 (+0xe0 + 0x10 id) cleared, its +0xc counted
// up, and +0xdc cleared. AL: 1 when a slot or the speech was id's, else 0.
u8 __fastcall Advisor_Cancel(u8 *self, void *, u32 id);

// 0x598dc0 (cdecl; name invented): with a holder for the slot 0xf79674 (ModelSlot_Holder): the 0xf79770 parts of the slot's row at 0xf796a0 (5 a slot) get flag 0x10 (the faces at
// the holder's +8 -> +4 -> +0x70, 0xa0 each); 0xf7978c = -1; ModelSlot_ApplySelection of the kinds 0xf79684 and, if set, 0xf79688; with 0xf79660's bit 0x10000 the slot's part at
// 0xf79698 marked as well. Then its bits 0x3e0000, when 0xf7966c is on, set particle pool 0 by the one set, with the time at 0xf796c8 by 0xf7976c: 0x20000 one of effect 0x2c if one
// started; 0x40000 sound 0xc5 and two of 0x31; 0x80000 three of 0x31; 0x100000 emptied, then two of 0x2c; 0x200000 emptied. With 0xf7966c off, only 0x200000's.
void __cdecl Advisor_UpdateHighlight();

// 0x599800 (cdecl; name invented, unconfirmed): with the pending flag 0xf7968c set and the advisor's model's holder (ModelSlot_Holder of 0xf79674) queueing bank 0xc
// (Layer_GetQueued's first out): the highlight updated and the flags 0xf7968c and 0xf79690 cleared. Then with 0xf79690 set: the highlight updated, that flag cleared.
// Last ParticlePools_Update.
void __cdecl Advisor_FrameTick();
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[24], gText_Advisor_Serialize_76e120, 0x0076e120, 0x18);
GAME_DATA(char[20], gText_Advisor_Serialize_76e158, 0x0076e158, 0x14);
GAME_DATA(char[24], gText_Advisor_Serialize_76e16c, 0x0076e16c, 0x18);
GAME_DATA(char[12], gText_Advisor_Serialize_76e184, 0x0076e184, 0xc);
GAME_DATA(char[68], gText_Advisor_HandleCode_76e190, 0x0076e190, 0x44);
GAME_DATA(u32, gAdvisor_f79bb8, 0x00f79bb8, 0x4);
// The advisor's 0x168-byte block (Advisor_Init zeroes it; Advisor_Load and the cheat module's save read and write it whole).
struct AdvisorState {
    u32 flags;            // +0x00: bit 0x10000 the bug head; bits 0x3e0000 the particle sets
    s32 startTime;        // +0x04
    s32 endTime;          // +0x08
    u32 speaking;         // +0x0c
    s32 mouthOpen;        // +0x10
    s32 slot;             // +0x14: the model slot
    s32 theme;            // +0x18
    u32 sound;            // +0x1c
    u32 voice;            // +0x20
    s32 selection;        // +0x24
    s32 selection2;       // +0x28
    u32 pending;          // +0x2c
    u32 highlight;        // +0x30
    s32 sampleAt;         // +0x34
    u32 bugHead[2];       // +0x38: by slot
    u32 mouthParts[10];   // +0x40: 5 a slot
    s32 animLengths[20];  // +0x68: motion bank 5's end times
    u32 queue[20];        // +0xb8
    u32 queueIndex;       // +0x108
    u32 anim;             // +0x10c
    s32 shapeCount;       // +0x110
    s32 lipAt;            // +0x114
    s32 endAt;            // +0x118
    u32 paused;           // +0x11c
    s32 pauseStart;       // +0x120
    s32 pausedTotal;      // +0x124
    s32 shape;            // +0x128
    s32 shownShape;       // +0x12c
    s32 shapeAt;          // +0x130
    u32 lipBytes;         // +0x134
    u32 lipIndex;         // +0x138
    u32 lipSync;          // +0x13c
    u32 lipTimes;         // +0x140: malloc'd, or gAdvisor_LipBuffer_f797d0
    u32 addOns[4];        // +0x144: the theme costumes' add-on parts (render/state); kind -1 clears them
    u32 subOffs[4];       // +0x154: and their sub-off parts, which kind -1 sets
    u32 held;             // +0x164
};
GAME_DATA(AdvisorState, gAdvisor_State_f79660, 0x00f79660, 0x168);
GAME_DATA(u32, gAdvisor_Stream_f797c8, 0x00f797c8, 0x4);
GAME_DATA(u8[0x3e8], gAdvisor_LipBuffer_f797d0, 0x00f797d0, 0x3e8);
GAME_DATA(char[16], gText_Advisor_Serialize_76e110, 0x0076e110, 0x10);
GAME_DATA(char[12], gText_Advisor_Serialize_76e138, 0x0076e138, 0xc);
GAME_DATA(char[20], gText_Advisor_Serialize_76e144, 0x0076e144, 0x14);
