#include "objectunlock.h"

#include <cstdint>

#include "../core/globalsave.h"
#include "../core/stdtree.h"
#include "advisorscore.h"
#include "mapcell.h"
#include "object.h"
#include "objectdefs.h"
#include "ridesearch.h"
#include "thingmsg.h"
#include "thingscan.h"
#include "world.h"

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

extern "C" {

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
}
