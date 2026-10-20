#include "advisorscore.h"

#include <cstdint>

#include "../core/balancefile.h"
#include "../core/options.h"
#include "objectdefs.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const AdvisorScoreEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0)};
const AdvisorScoreEnv *g_env = &kGameEnv;

// The mode setting (0xfb3b7c), read the first time under the function's own guard bit, its (empty) destructor 0x5d1750 given to atexit.
u32 mode_setting(u32 guard_address) {
    u8 &guard = global<u8>(guard_address);
    const u8 bits = guard;
    if (!(bits & 1)) {
        guard = bits | 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>(0x005d1750));
    }
    return global<u32>(0x00fb3b7c);
}

// Whether the record applies: +0xc 2; 0 with the setting 0; (re-read) 1 with the setting 2.
bool applies(u32 record) {
    const u32 when = global<u32>(record + 0xc);
    if (when == 2)
        return true;
    if (when == 0 && mode_setting(0x00fb34bc) == 0)
        return true;
    return global<u32>(record + 0xc) == 1 && mode_setting(0x00fb34bc) == 2;
}

s32 call(u8 *self, u32 record) {
    using Method = s32(__fastcall *)(u8 *, void *);
    return reinterpret_cast<Method>((uintptr_t)global<u32>(record + 0x10))(self + global<u32>(record + 0x14), nullptr);
}

}  // namespace

void AdvisorScore_SetEnv(const AdvisorScoreEnv *env) { g_env = env ? env : &kGameEnv; }

u8 *AdvisorScoring_Instance() {
    u8 &guard = global<u8>(0x00fb2766);
    const u8 bits = guard;
    u8 *const scoring = reinterpret_cast<u8 *>(0x00fb3540);
    if (!(bits & 1)) {
        guard = bits | 1;
        AdvisorScoring_Construct(scoring);
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>(0x00415e70));   // (AdvisorScoring_StaticDtor)
    }
    return scoring;
}

extern "C" {

s32 __fastcall AdvisorScoring_Score(u8 *self, void *, u32 id) {
    const u32 method = Table76e300_FindRecord(id);
    if (!method)
        return -1;
    const u32 record = Table76e300_FindRecord(id);
    if (!record || !applies(record))
        return -1;
    return call(self, method);
}

s32 __fastcall AdvisorScoring_ScoreDetail(u8 *self, void *, u32 id, u16 *word, u32 *dword) {
    const u32 method = Table76e300_FindRecord(id);
    if (!method)
        return -1;
    at<u16>(self, 4) = 0;
    at<u32>(self, 8) = 0;
    const u32 record = Table76e300_FindRecord(id);
    const s32 score = record && applies(record) ? call(self, method) : -1;
    *word = at<u16>(self, 4);
    *dword = at<u32>(self, 8);
    return score;
}

u8 *__fastcall AdvisorMsg_Init(u8 *self, void *, u32 id, u16 type, u32 a, u32 b, u8 flag, s32 score) {
    at<u16>(self, 0) = type;
    at<u32>(self, 0xc) = id;
    at<u32>(self, 4) = a;
    if (score >= 0) {
        at<s32>(self, 0x10) = score;
        at<u8>(self, 0x16) = 1;
    } else {
        at<s32>(self, 0x10) = AdvisorScoring_Score(AdvisorScoring_Instance(), nullptr, id);
        at<u8>(self, 0x16) = 0;
    }
    at<u8>(self, 0x15) = flag;
    at<u32>(self, 8) = b;
    at<u8>(self, 0x14) = 1;
    return self;
}
}
