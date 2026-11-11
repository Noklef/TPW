// The sparkle on the dialog [0x7cc4b4]: see dialog7cc4b4sparkle.h.
#include "dialog7cc4b4sparkle.h"

#include <cstdint>

#include "../core/options.h"
#include "../core/soundplay.h"
#include "../render/particles.h"
#include "guirect.h"
#include "window.h"
#include "dialog7cc4b4open.h"
#include "dialog74fb20.h"
#include "../core/soundset.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Dialog7cc4b4SparkleEnv kGameEnv = {Setting_ReadMode, GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), Window_GetRect, Particle_Spawn, Sound_PlayAt};
const Dialog7cc4b4SparkleEnv *g_env = &kGameEnv;

}  // namespace

void Dialog7cc4b4_SetSparkleEnv(const Dialog7cc4b4SparkleEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __cdecl Dialog7cc4b4_Sparkle() {
    u8 *const dialog = gDialog7cc4b4open_7cc4b4;
    if (!dialog) return;
    u8 &guard = gDialog7cc4b4sparkle_fb34bd;
    u8 *const setting = reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(setting);
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (at<u32>(setting, 0) == 2) return;
    u8 rect[12];
    GuiRect_Ctor(rect);
    g_env->getRect(Window_FindChild(dialog, nullptr, 0x1e0ea), nullptr, rect, 1);
    const s32 top = at<s16>(rect, 6), bottom = at<s16>(rect, 0xa);
    g_env->spawn(0x62, 50000, 0, (s32)((u32)(((bottom - top) >> 1) + top) * 75000) / 0x600);
    g_env->playAt(0, gSoundCategories.lobbySfx, 4, 0, 0, 0);
    GuiRect_Dtor(rect);
}
}
