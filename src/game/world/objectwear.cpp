// A ride or shop wearing down as it runs: see objectwear.h.
#include "objectwear.h"

#include <cstdint>

#include "../core/options.h"
#include "../math/x87.h"
#include "advisor.h"
#include "advisorscore.h"
#include "object.h"
#include "objectdefs.h"
#include "ridelist.h"
#include "things.h"
#include "world.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ObjectWearEnv kGameEnv = {ObjectDefs_Find, Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), Rides_FindById, Rides_GetElement, Rides_GetWordC0, Object_DefRowScore, Rides_SetElement, Thing_GetHandle, GameTable7cfb90_Get1da71c_5194d0, Advisor_Post};
const ObjectWearEnv *g_env = &kGameEnv;

bool modeIsOne() {
    u8 &guard = global<u8>(0x00fb34b8);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return global<u32>(0x00fb3b7c) == 1;
}

void store(float &field, X87Reg value) {
    if (value.v > 100.0) field = 100.0f;
    else if (!(value.v >= 0.0)) field = 0.0f;   // (NaN too)
    else field = (float)value;
}

// The float's integer part's low byte (__ftol).
s32 percent(float value) { return (u8)(long long)value; }

}  // namespace

void ObjectWear_SetEnv(const ObjectWearEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" void __fastcall Object_WearTick(u8 *self) {
    const u8 *const def = g_env->findDef(reinterpret_cast<u8 *>((uintptr_t)0x007890a0), nullptr, at<u16>(self, 0xe));
    u32 rate = at<u32>(def, ((u32)at<u8>(self, 0x50) << 6) + 0x1b0);
    if (modeIsOne()) rate = 0;
    if (!rate) return;
    if ((s32)g_env->element(g_env->ride(at<s32>(self, 0x24)), 9) <= 0 && !(at<u8>(self, 0x33) & 1)) return;
    if (at<u8>(global<u8 *>(0x0080239c), 0x1da70c) & 0x3f) return;
    u8 *const ride = g_env->ride(at<s32>(self, 0x24));
    float wear = 0.0f;
    if (g_env->element(ride, 5)) {
        const u8 *const again = g_env->ride(at<s32>(self, 0x24));
        const float a = (float)g_env->wordC0(again);
        const float b = (float)(s32)g_env->element(again, 2);
        const float c = (float)(s32)g_env->element(again, 3);
        wear = (float)g_env->score(self, nullptr, a, b, *reinterpret_cast<const u32 *>(&c), 0);
    }
    store(at<float>(self, 0x44), X87Reg(-(double)wear) + at<float>(self, 0x44));
    store(at<float>(self, 0x48), X87Reg(-(X87Reg((double)wear) * X87Reg((double)0.02f)).v) + at<float>(self, 0x48));
    if (percent(at<float>(self, 0x44)) < 20) g_env->setElement(ride, 8, 1);
    const s32 left = percent(at<float>(self, 0x44));
    if (left < 10 && (X87Reg((double)wear) + X87Reg((double)left)).v >= 10.0) {
        u8 *const world = global<u8 *>(0x0080239c);
        const u32 word = at<u16>(self, 0xe);
        u16 out;
        const u16 type = *g_env->handle(self, nullptr, &out);
        u8 message[0x18];
        AdvisorMsg_Init(message, nullptr, 0x42, type, word, 0xffffffffu, 0, -1);
        g_env->post(reinterpret_cast<u8 *>((uintptr_t)g_env->advisor(world)), nullptr, message);
    }
}
