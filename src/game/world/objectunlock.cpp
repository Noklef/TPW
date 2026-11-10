#include "objectunlock.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/globalsave.h"
#include "../core/options.h"
#include "../core/stdtree.h"
#include "advisorscore.h"
#include "analyser.h"
#include "mapcell.h"
#include "money.h"
#include "object.h"
#include "objectdefs.h"
#include "research.h"
#include "ridesearch.h"
#include "thingmsg.h"
#include "thingscan.h"
#include "world.h"
#include "../ui/dialog74fb20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// An advisor message (id, type 0, a 0, b, flag 0, scored) posted as message 05.
void post_advice(u32 id, u32 b = 0xffffffffu) {
    u8 advice[0x18];   // (its bytes +2, +3 and +0x17 never written, sic; the message copies none of them)
    AdvisorMsg_Init(advice, nullptr, id, 0, 0, b, 0, -1);
    u8 msg[0x20];   // (its bytes +0xa, +0xb and +0x1f left as the stack had them, as the original's)
    Obj_CtorPart8(msg, nullptr, (u32)(uintptr_t)advice);
    Object_PostMessage(msg);
}

const ObjectUnlockScoresEnv kScoresGameEnv = {GameTable7cfb90_Get1da720_519510, Things_TallestCoaster, Things_BestRatedDef9c1, Rides_BestPiecesOfKind,
                                              Map_CountSpecialCells, GlobalSave_MarkScore, ObjectUnlock_AdviseScore};
const ObjectUnlockScoresEnv *g_scores = &kScoresGameEnv;

void check_score(u8 *self, u32 group, s32 value, u32 limit) {
    if (value <= *reinterpret_cast<const s32 *>((uintptr_t)limit)) return;
    const u32 level = g_scores->mark(GlobalSave_Instance(), nullptr, group, value);
    if (level) g_scores->advise(self, nullptr, group, (s32)level);
}

}  // namespace

void ObjectUnlock_SetScoresEnv(const ObjectUnlockScoresEnv *env) { g_scores = env ? env : &kScoresGameEnv; }

namespace {
const ObjectUnlockStepEnv kStepGameEnv = {&Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &ObjectUnlock_CheckThemes, &ObjectUnlock_CheckScores, &GameTable7cfb90_Get1da720_519510,
                                          &GlobalSave_Byte26, &Map_PercentWithRE, &GlobalSave_MarkExtra, &ObjectUnlock_AdviseExtraLevel};
const ObjectUnlockStepEnv *g_step = &kStepGameEnv;
}  // namespace

void ObjectUnlock_SetStepEnv(const ObjectUnlockStepEnv *env) { g_step = env ? env : &kStepGameEnv; }

namespace {

const ObjectUnlockThemesEnv kThemesGameEnv = {GlobalSave_ThemeByte, GlobalSave_MarkTheme, ObjectUnlock_AdviseLevel, GameTable7cfb90_Get1da720_519510, ParkAnalyser_Total, Things_CountKind1OnPlainCell, Things_AveragePersonStat19c,
                                              Research_AllDone, GameTable7cfb90_Get1da726_5195d0, Money_Total, ParkAnalyser_TotalSince, FunnyTime_Elapsed};
const ObjectUnlockThemesEnv *g_themes = &kThemesGameEnv;

s32 limitAt(u32 address) { return *reinterpret_cast<const s32 *>((uintptr_t)address); }
u8 *world() { return *reinterpret_cast<u8 **>((uintptr_t)0x0080239c); }
bool marked(u32 i) { return g_themes->themeByte(GlobalSave_Instance(), nullptr, i) != 0; }

void markAndAdvise(u8 *self, u32 i) {
    const u32 level = g_themes->mark(GlobalSave_Instance(), nullptr, i);
    if (level) g_themes->advise(self, nullptr, i, (s32)level);
}

}  // namespace

void ObjectUnlock_SetThemesEnv(const ObjectUnlockThemesEnv *env) { g_themes = env ? env : &kThemesGameEnv; }

extern "C" {

void __fastcall ObjectUnlock_CheckThemes(u8 *self) {
    const u8 *const analyser = reinterpret_cast<const u8 *>((uintptr_t)g_themes->table(world()));
    if (!marked(0) && (s32)g_themes->total(analyser) > limitAt(0x00785790)) markAndAdvise(self, 0);
    if (!marked(1) && g_themes->people() > limitAt(0x00785794)) markAndAdvise(self, 1);
    if (!marked(2)) {
        const double average = g_themes->average(const_cast<u8 *>(analyser), nullptr, 0);
        if (!((double)limitAt(0x00785798) >= average) && g_themes->people() > limitAt(0x0078579c)) markAndAdvise(self, 2);
    }
    if (!marked(3) && g_themes->allDone()) markAndAdvise(self, 3);
    if (!marked(4) && (s32)g_themes->money(reinterpret_cast<const u8 *>((uintptr_t)g_themes->table26(world()))) > limitAt(0x007857a0)) markAndAdvise(self, 4);
    if (!marked(5) && (s32)g_themes->since(analyser, nullptr, limitAt(0x007857a8)) > limitAt(0x007857a4)) {
        FileTime out;
        const FileTime *const elapsed = g_themes->elapsed(world() + 0x2a0, nullptr, &out);
        const long long time = (long long)(((unsigned long long)elapsed->high << 32) | elapsed->low);
        if ((s32)(time / 0x1792f8648000LL) > limitAt(0x007857a8)) markAndAdvise(self, 5);
    }
}

void __fastcall ObjectUnlock_CheckScores(u8 *self) {
    g_scores->store(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c));
    u16 out;
    check_score(self, 0, (s32)g_scores->tallest(&out, 0), 0x007857ac);
    check_score(self, 1, (s32)g_scores->bestRated(&out), 0x007857b0);
    check_score(self, 2, (s32)g_scores->bestPieces(&out, 2), 0x007857b4);
    check_score(self, 3, g_scores->specialCells(), 0x007857bc);
}

u8 __stdcall ObjectDef_IsUnlocked(u32 id) {
    const s32 needed = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, (u16)id), 0xc4);
    return needed <= GlobalSave_CountSetLeft(GlobalSave_Instance()) ? 1 : 0;
}

u8 __stdcall ObjectDef_Unlock(u32 id) {
    const s32 needed = at<s32>(ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, (u16)id), 0xc4);
    if (AdvisorScore_ModeSetting(0x00fb34ba) == 1) return 1;
    if (!ObjectDef_IsUnlocked(id)) return 0;
    return Object_AddKeyed(GlobalSave_Instance(), nullptr, id, needed);
}

void __fastcall ObjectUnlock_FirstAdvice(u8 *) {
    if (GlobalSave_CountSet(GlobalSave_Instance()) != 1) return;
    post_advice(0x137);
    post_advice(0x13b);
}

void __fastcall ObjectUnlock_AdviseLevel(u8 *self, void *, u32 group, s32 level) {
    if (level >= 1 && level <= 3) post_advice(0xd6 + (u32)level, group);   // (other levels: a debug print)
    ObjectUnlock_FirstAdvice(self);
}

void __fastcall ObjectUnlock_AdviseScore(u8 *self, void *, u32 group, s32 level) {
    if (level >= 1 && level <= 3) post_advice(0xdb + (u32)level, group);
    else if (level == 4 && group <= 3) post_advice(0xea + group);   // (b -1; other levels and groups: debug prints)
    ObjectUnlock_FirstAdvice(self);
}

void __fastcall ObjectUnlock_AdviseExtraLevel(u8 *self, void *, u32 group, s32 level) {
    if (level >= 1 && level <= 3) post_advice(0xe0 + (u32)level, group);   // (other levels: a debug print)
    ObjectUnlock_FirstAdvice(self);
}

void __fastcall ObjectUnlock_Step(u8 *self) {
    u8 &guard = *reinterpret_cast<u8 *>((uintptr_t)0x00fb34ba);
    if (!(guard & 1)) {
        guard |= 1;
        g_step->readMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_step->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c != 0) return;
    g_step->checkThemes(self);
    g_step->checkScores(self);
    g_step->store(*reinterpret_cast<u8 **>((uintptr_t)0x0080239c));
    if (g_step->byte26(GlobalSave_Instance(), nullptr, 0)) return;
    if (g_step->percent() != 100) return;
    const u32 level = g_step->markExtra(GlobalSave_Instance(), nullptr, 0);
    if (level) g_step->advise(self, nullptr, 0, static_cast<s32>(level));
}

s32 __cdecl ObjectUnlock_ExtraCount() { return GlobalSave_CountSet4(GlobalSave_Instance()); }
}
