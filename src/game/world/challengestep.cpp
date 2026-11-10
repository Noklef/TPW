#include "challengestep.h"

#include <cstdint>

#include "../core/filetime.h"
#include "../core/funnytime.h"
#include "../core/idmaps.h"
#include "../core/options.h"
#include "challenges.h"
#include "messagecentre.h"
#include "thingmsg.h"
#include "world.h"
#include "../ui/dialog7ca2e8.h"
#include "../ui/dialog74fb20.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const ChallengeStepEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &Challenge_Met, &Challenges_Won, &Challenges_Lost, &Challenges_OfferNext, &MessageCentre_Post};
const ChallengeStepEnv *g_env = &kGameEnv;

u8 *world() { return global<u8 *>(0x0080239c); }

s32 days(const FileTime &span) { return static_cast<s32>(static_cast<long long>((static_cast<unsigned long long>(span.high) << 32) | span.low) / 0xc92a69c000LL); }

FileTime now() {
    FileTime t;
    FunnyTime_Now(world() + 0x2a0, nullptr, &t, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    return t;
}

bool timePast(u8 *challenges) {
    const FileTime t = now();
    return FileTime_Greater(&t, nullptr, &at<FileTime>(challenges, 0x3d0)) != 0;
}

s32 daysLeft(u8 *challenges) {
    const FileTime t = now();
    FileTime left;
    FileTime_Sub(&at<FileTime>(challenges, 0x3d0), nullptr, &left, &t);
    return days(left);
}

}  // namespace

void ChallengeStep_SetEnv(const ChallengeStepEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Challenges_Step(u8 *challenges) {
    u8 &guard = gDialog7ca2e8_fb34c3;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (gDialog74fb20_fb3b7c) return;
    if (!at<u8>(challenges, 0x3d8)) {
        GameTable7cfb90_Get1da720_519510(world());
        if (!Game_Field1da710IsZero()) return;
        FileTime elapsed;
        FunnyTime_Elapsed(world() + 0x2a0, nullptr, &elapsed);
        if (days(elapsed) < global<s32>(0x007857c8)) return;
        at<u8>(challenges, 0x3d8) = 1;
        g_env->offerNext(challenges, nullptr, 1);
    }
    s32 *const value = &at<s32>(challenges, 0x3dc);
    u8 msg[0x30];
    if (!at<u8>(challenges, 0x3d9)) {
        if (!at<u8>(challenges, 0x3da) && timePast(challenges)) g_env->offerNext(challenges, nullptr, 0);
        return;
    }
    if (!Challenges_Current(challenges)) return;
    if (!(at<u8>(world(), 0x1da70c) & 3)) {
        const s32 left = daysLeft(challenges);
        u8 *const current = Challenges_Current(challenges);
        ThingMsg19_Ctor(msg, nullptr, Kind_ToIdB(reinterpret_cast<const u32 *>(current)), 0x160, 0, at<u32>(current, 0x14), at<u32>(current, 4), at<u32>(current, 8), static_cast<u32>(left), static_cast<u32>(*value),
                        at<u16>(current, 0xc), at<u16>(current, 0xe), at<u32>(current, 0x10));
        g_env->post(MessageCentre_Instance(), nullptr, msg);
        return;
    }
    if (timePast(challenges)) {
        if (!Challenges_Current(challenges)) return;  // (a debug print left out)
        if (g_env->met(Challenges_Current(challenges), nullptr, value, 1))
            g_env->won(challenges);
        else
            g_env->lost(challenges);
        return;
    }
    if (g_env->met(Challenges_Current(challenges), nullptr, value, 0)) {
        g_env->won(challenges);
        return;
    }
    if (!at<s32>(Challenges_Current(challenges), 4)) return;
    const s32 left = daysLeft(challenges);
    if (static_cast<s32>(static_cast<u32>(left) * 100) / at<s32>(Challenges_Current(challenges), 4) >= global<s32>(0x007857d0)) return;
    ThingMsg19_Ctor(msg, nullptr, 0x17f, 0x10f, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    g_env->post(MessageCentre_Instance(), nullptr, msg);
}
}
