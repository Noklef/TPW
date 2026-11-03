#include "arreplay.h"

#include <cstdint>

#include "../core/actionrec.h"
#include "../world/mapclick.h"
#include "../world/mappress.h"
#include "cars.h"
#include "edittools.h"
#include "registry.h"
#include "train.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ArReplayEnv kGame = {&CoasterEdit_Tool1,
                           &CoasterEdit_Tool2,
                           reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00447d30),
                           reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x00447d90),
                           &GameMode_MapPress,
                           &GameMode_MapClick,
                           reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)0x00404d80),
                           reinterpret_cast<void(__fastcall *)(u8 *, void *, u32, u32, u32, const u16 *)>((uintptr_t)0x00404f70),
                           &Global8186e0_SetRecorded,
                           &Car_ApplyLengthAt,
                           &Car_ApplyE0At,
                           &Car_ApplyAnglesAt,
                           &CoasterEdit_DeleteSpanBetween,
                           &Coaster_SetMixAtPinned,
                           &CoasterEdit_PickPiece};
const ArReplayEnv *g_env = &kGame;

bool Read(u8 *stream, void *buffer, u32 bytes) {
    using Fn = u32(__fastcall *)(void *, void *, void *, u32);
    return reinterpret_cast<Fn>(at<void *>(at<void *>(stream, 0), 8))(stream, nullptr, buffer, bytes) == bytes;
}

}  // namespace

void ArReplay_SetEnv(const ArReplayEnv *env) { g_env = env ? env : &kGame; }

extern "C" u32 __fastcall ArReplay_Step(u8 *self) {
    u8 *const stream = self + 0xc;
    u32 raw = 0;
    if (!Read(stream, &raw, 1))
        return 0;
    const u32 action = raw & 0xff;
    u32 a = 0, b = 0, c = 0;
    u8 words[0x80];
    switch (at<u32>(self, 0x34 + action * 4)) {
    case 0:
        if (!Read(stream, &a, 1) || !Read(stream, &b, 1))
            return 0;
        if (action == 0xf)
            g_env->mapPress(a & 0xff, b & 0xff, 0, 0);
        else if (action == 0x14)
            g_env->mapClick(a & 0xff, b & 0xff, 0, 0);
        break;
    case 1:
        if (!Read(stream, &a, 2))
            return 0;
        g_env->f404d80(self, nullptr, action, a);
        break;
    case 2:
        if (!Read(stream, &a, 1) || !Read(stream, &b, 1) || !Read(stream, &c, 2))
            return 0;
        if (action == 0x28)
            g_env->mapPress(a & 0xff, b & 0xff, c, 0);
        else if (action == 0x2d)
            g_env->mapClick(a & 0xff, b & 0xff, c, 0);
        break;
    case 3:
        if (!Read(stream, &a, 2) || !Read(stream, &b, 1))
            return 0;
        if (action == 5) {
            const u16 id = (u16)a;
            g_env->setRecorded(&id, b & 0xff);
        }
        break;
    case 4:
        if (!Read(stream, &a, 4) || !Read(stream, &c, 2))
            return 0;
        if (action == 0x39)
            g_env->applyLength(a, c & 0xffff);
        else if (action == 0x3a)
            g_env->applyE0(a, c & 0xffff);
        break;
    case 5:
        if (!Read(stream, &a, 4) || !Read(stream, &b, 4) || !Read(stream, &c, 2))
            return 0;
        if (action == 0x3b)
            g_env->applyAngles(a, b, c & 0xffff);
        break;
    case 6:
        if (!Read(stream, &a, 2) || !Read(stream, &b, 2))
            return 0;
        if (action == 0x3f)
            g_env->deleteSpan(a, b);
        else if (action == 0x45)
            g_env->setMix(a & 0xffff, b & 0xffff);
        break;
    case 7:
        if (!Read(stream, &a, 2) || !Read(stream, &b, 2) || !Read(stream, &c, 2))
            return 0;
        if (action == 0x3c)
            g_env->pickPiece(a, b, c);
        break;
    case 8:
        if (action == 0x37)
            g_env->tool1();
        else if (action == 0x38)
            g_env->tool2();
        else if (action == 0x40)
            g_env->f447d30();
        else if (action == 0x41)
            g_env->f447d90();
        break;
    case 9:
        if (!Read(stream, &a, 1) || !Read(stream, words, (a & 0xff) * 2))
            return 0;
        break;
    case 10:
        if (!Read(stream, &a, 2) || !Read(stream, &b, 1) || !Read(stream, words, (b & 0xff) * 2))
            return 0;
        g_env->f404f70(self, nullptr, action, a, b, reinterpret_cast<const u16 *>(words));
        break;
    case 11:
        if (!Read(stream, words, 0x80))
            return 0;
        break;
    default:
        break;
    }
    return 1;
}
