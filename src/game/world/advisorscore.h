// The advisor's scoring: the function-static gAdvisorScoring (0xfb3540, advisor.sam's values, built on first use) scores a message id
// through a pointer to a member function in the id's record of the table at 0x76e300 (0x38 bytes: +4 id, +0xc when it applies (0 offline,
// 1 online, 2 always, against the mode setting 0xfb3b7c), +0x10 the method, +0x14 the offset of its this in gAdvisorScoring), and the
// advisor's messages (0x18 bytes) take their score from it. Reimplementation of 0x59bf20, 0x59c0a0 and 0x59b590.
#pragma once

#include "../../../re/types.h"

struct Stream;

struct AdvisorScoreEnv {
    s32(__cdecl *atExit)(void(__cdecl *fn)());   // crt_atexit (0x67a7d0)
};
// The atexits of the statics built on first use go to env (nullptr: the game's).
void AdvisorScore_SetEnv(const AdvisorScoreEnv *env);

// gAdvisorScoring (0xfb3540; not a game function, the game inlines it): built the first time under the guard 0xfb2766 & 1
// (AdvisorScoring_Construct), AdvisorScoring_StaticDtor (0x415e70) then given to atexit.
u8 *AdvisorScoring_Instance();

// The mode setting (0xfb3b7c; not a game function, the game inlines it under each user's own guard byte): read the first time under bit 0
// of the byte at guard_address (Setting_ReadMode), its (empty) destructor 0x5d1750 then given to atexit.
u32 AdvisorScore_ModeSetting(u32 guard_address);

extern "C" {

// 0x59bf20 (thiscall (id), pops 4; name unconfirmed): id's record of the table at 0x76e300 (Table76e300_FindRecord, looked up twice; its
// failed search's message goes to the empty Debug_Printf): none, -1. Its +0xc 2, or 0 with the mode setting (0xfb3b7c, read the first time
// under the guard 0xfb34bc & 1 (Setting_ReadMode, atexit 0x5d1750)) 0, or 1 with the setting 2: the record's method (+0x10, thiscall, no
// arguments) on this + its +0x14, whose answer is returned; otherwise -1.
s32 __fastcall AdvisorScoring_Score(u8 *self, void *, u32 id);

// 0x59c0a0 (thiscall (id, &word, &dword), pops 12; name unconfirmed): the same, but with a record the word at +4 and the dword at +8
// cleared first and, called or not, copied out to word and dword after (no record: -1 and neither written).
s32 __fastcall AdvisorScoring_ScoreDetail(u8 *self, void *, u32 id, u16 *word, u32 *dword);

// 0x59b590 (thiscall (id, type, a, b, flag, score), pops 24; name unconfirmed): an advisor message: +0 the word type, +0xc id, +4 a, +8 b,
// the byte +0x15 flag; score at or above 0 is kept (+0x10, the byte +0x16 1), else gAdvisorScoring (0xfb3540, built the first time under the
// guard 0xfb2766 & 1 by AdvisorScoring_Construct with AdvisorScoring_StaticDtor given to atexit) scores id (AdvisorScoring_Score) into +0x10,
// +0x16 0. The byte +0x14 set to 1. Returns the message.
// (The message, 0x18 bytes: +0 type, +4 a, +8 b, +0xc id, +0x10 score, the bytes +0x14 set, +0x15 flag, +0x16 the score given.)
u8 *__fastcall AdvisorMsg_Init(u8 *self, void *, u32 id, u16 type, u32 a, u32 b, u8 flag, s32 score);

// 0x59ba30 (thiscall (stream, save, version, unused), pops 16): the message saved or loaded as the serializers do (core/savegame's
// Save_Field): mThingId (the word +0), mSecondary (+4), mMessage (+0xc), mScore (+0x10), mValid (the byte +0x14), mResponseNo (+8),
// mOverrideOnlyOnce (+0x15), mScoreOverridden (+0x16); a load reads all but the first through Save_ReadU32 / Save_ReadU8.
u32 __fastcall AdvisorMsg_Serialize(u8 *self, void *, Stream *stream, u32 save, s32 version, u32 unused);
}
