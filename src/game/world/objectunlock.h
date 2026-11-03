// An object definition's availability against the global save (the count of set bytes left), and its unlocking: reimplementation of
// 0x4d4b90, 0x4d4ad0, 0x4d5b30, 0x4d4fa0, 0x4d5270 and 0x4d5870. The names are invented.
#pragma once

#include "../../../re/types.h"

// ObjectUnlock_CheckScores's calls (the ports by default; the bench's recorders: their own benches have them).
struct ObjectUnlockScoresEnv {
    u32(__fastcall *store)(u8 *game);                              // GameTable7cfb90_Get1da720_519510
    u32(__stdcall *tallest)(u16 *out, u32 def);                    // Things_TallestCoaster
    u32(__stdcall *bestRated)(u16 *out);                           // Things_BestRatedDef9c1
    u32(__stdcall *bestPieces)(u16 *out, u32 kind);                // Rides_BestPiecesOfKind
    s32(__cdecl *specialCells)();                                  // Map_CountSpecialCells
    u32(__fastcall *mark)(u8 *info, void *, u32 i, s32 score);     // GlobalSave_MarkScore
    void(__fastcall *advise)(u8 *self, void *, u32 group, s32 level);   // ObjectUnlock_AdviseScore
};
void ObjectUnlock_SetScoresEnv(const ObjectUnlockScoresEnv *env);

struct FileTime;

// ObjectUnlock_CheckThemes's calls (the ports by default; the bench's recorders: their own benches have them).
struct ObjectUnlockThemesEnv {
    u8(__fastcall *themeByte)(const u8 *info, void *, u32 i);                  // GlobalSave_ThemeByte
    u32(__fastcall *mark)(u8 *info, void *, u32 i);                            // GlobalSave_MarkTheme
    void(__fastcall *advise)(u8 *self, void *, u32 group, s32 level);          // ObjectUnlock_AdviseLevel
    u32(__fastcall *table)(u8 *game);                                          // GameTable7cfb90_Get1da720_519510
    u32(__fastcall *total)(const u8 *analyser);                                // ParkAnalyser_Total
    s32(__cdecl *people)();                                                    // Things_CountKind1OnPlainCell
    double(__fastcall *average)(void *analyser, void *, u32 unused);           // Things_AveragePersonStat19c
    u8(__cdecl *allDone)();                                                    // Research_AllDone
    u32(__fastcall *table26)(u8 *game);                                        // GameTable7cfb90_Get1da726_5195d0
    u32(__fastcall *money)(const u8 *money);                                   // Money_Total
    u32(__fastcall *since)(const u8 *analyser, void *, s32 back);              // ParkAnalyser_TotalSince
    FileTime *(__fastcall *elapsed)(const u8 *clock, void *, FileTime *out);   // FunnyTime_Elapsed
};
void ObjectUnlock_SetThemesEnv(const ObjectUnlockThemesEnv *env);

extern "C" {

// 0x4d4bc0 (thiscall; name invented, unconfirmed): the six themes not yet marked (GlobalSave_ThemeByte, each read anew), each marked (GlobalSave_MarkTheme) when its test holds and a level then advised (ObjectUnlock_AdviseLevel). 0: the analyser's total over [0x785790]. 1: the people over [0x785794].
// 2: the int [0x785798] not at least the average 0x19c (NaN holds too) and the people over [0x78579c]. 3: Research_AllDone. 4: the money's total over [0x7857a0]. 5: the total of the last [0x7857a8] over [0x7857a4] and the time elapsed (the clock [0x80239c] + 0x2a0; signed, in 0x1792f8648000 units) over [0x7857a8].
// All signed. The analyser (the table's entry for [0x80239c]) read first. The debug print after each advice (0x5af4a0 of the instance into the empty 0x5da3c0) is left out.
void __fastcall ObjectUnlock_CheckThemes(u8 *self);

// 0x4d4e50 (thiscall; name invented): the park's four best figures against their limits: the tallest coaster (Things_TallestCoaster with the id 0; group 0, [0x7857ac]), the best rated (Things_BestRatedDef9c1;
// 1, [0x7857b0]), the most pieces of the kind 2 (Rides_BestPiecesOfKind; 2, [0x7857b4]) and the special cells (Map_CountSpecialCells; 3, [0x7857bc]). A figure over its limit (signed) is marked in the global save
// (GlobalSave_MarkScore) and when that answers a level advised (ObjectUnlock_AdviseScore). The store's getter is called first, its answer given only as an unused ECX (sic); the debug print after each advice
// (0x5af4a0 of the instance into the empty 0x5da3c0) is left out.
void __fastcall ObjectUnlock_CheckScores(u8 *self);

// 0x4d4b90 (stdcall (id), pops 4; name invented): whether the definition of the id (ObjectDefs_Find on 0x7890a0, not checked) asks for
// no more (its s32 +0xc4) than the global save's count left (GlobalSave_CountSetLeft of GlobalSave_Instance; signed). Only AL is set.
u8 __stdcall ObjectDef_IsUnlocked(u32 id);

// 0x4d4ad0 (stdcall (id), pops 4; name invented): the definition's s32 +0xc4 read first (ObjectDefs_Find, not checked); with the mode
// setting (0xfb3b7c, read the first time under the guard 0xfb34ba & 1) 1, true. Otherwise false unless ObjectDef_IsUnlocked (inlined: the
// definition looked up again); then the id goes into the global save's set (Object_AddKeyed on GlobalSave_Instance, the +0xc4 its
// amount): whether it was new. Only AL is set.
u8 __stdcall ObjectDef_Unlock(u32 id);

// 0x4d5b30 (thiscall, this unused; called from 0x4d5235, 0x4d5824 and 0x4d5b05): when the global save's count (GlobalSave_CountSet of
// GlobalSave_Instance, inlined there) is 1, the advisor messages 0x137 and 0x13b (AdvisorMsg_Init: type 0, a 0, b -1, flag 0, scored)
// each posted as message 05 (Obj_CtorPart8, Object_PostMessage).
void __fastcall ObjectUnlock_FirstAdvice(u8 *self);

// 0x4d4fa0 (thiscall (group, level), pops 8; called from 0x4d4c0c and five more with the group 0-3 and the level an unlock count
// gave; its chunk from 0x4d5020 listed as a function of its own): the advisor message 0xd7, 0xd8 or 0xd9 for the level 1, 2 or 3
// (AdvisorMsg_Init: type 0, a 0, b the group, flag 0, scored) posted as message 05 (Obj_CtorPart8, Object_PostMessage); another level
// none (a debug print there). Then ObjectUnlock_FirstAdvice either way.
void __fastcall ObjectUnlock_AdviseLevel(u8 *self, void *, u32 group, s32 level);

// 0x4d5270 (thiscall (group, level), pops 8; called from 0x4d4e96 and three more with the group 0-3 and GlobalSave_MarkScore's
// answer): as ObjectUnlock_AdviseLevel with the advisor messages 0xdc, 0xdd and 0xde for the levels 1-3; for 4 (a score beaten) the
// message 0xea + the group with b -1 for the groups 0-3 (unsigned; another none). Two jump tables (0x4d5844, 0x4d5854) there.
void __fastcall ObjectUnlock_AdviseScore(u8 *self, void *, u32 group, s32 level);

// 0x4d5870 (thiscall (group, level), pops 8; called from 0x4d4a92 with the group 0 and GlobalSave_MarkExtra's level): as
// ObjectUnlock_AdviseLevel with the advisor messages 0xe1, 0xe2 and 0xe3.
void __fastcall ObjectUnlock_AdviseExtraLevel(u8 *self, void *, u32 group, s32 level);
}
