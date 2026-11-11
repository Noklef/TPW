// The dialog [0x7cc4b4] brought up to date: see dialog7cc4b4update.h.
#include "dialog7cc4b4update.h"

#include <cstdint>

#include "../core/balancefile.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globalsave.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "../render/particles.h"
#include "dialogs.h"
#include "guirect.h"
#include "window.h"
#include "dialog7cc4b4sparkle.h"
#include "dialog7cc4b4open.h"
#include "dialog74fb20.h"
#include "../coaster/arreplayaction.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 __cdecl Swprintf(u16 *to, const u16 *format, s32 value) { return Crt().swprintf(to, format, value); }

const Dialog7cc4b4UpdateEnv kGameEnv = {GameAddress<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0), &GlobalSave_Construct, GameAddress<u8 *(__fastcall *)(u8 *, void *, const String *)>(0x005b09c0), &GlobalSave_Instance, &GlobalSave_CountSetLevel, &ThemeRecord_GlobalSamField14, &Window_SetShown, &Window_Notify1, &Window_GetRect, &Particle_Spawn, &ParticleSystem_End, &Swprintf, &Dialog7cc4b4_SetFlag, GameAddress<void(__fastcall *)(u8 *, void *, u16 *, u32)>(0x005b12e0)};
const Dialog7cc4b4UpdateEnv *g_env = &kGameEnv;

u8 *&dialog() { return gDialog7cc4b4open_7cc4b4; }
u8 *child(u32 id) { return Window_FindChild(dialog(), nullptr, id); }

// The save info, made the first time, as GlobalSave_Instance does inline here.
u8 *info() {
    u8 *&saved = gArreplayaction_f7c58c;
    if (!saved) {
        u8 *const memory = static_cast<u8 *>(GameHeap_Alloc(0x50));
        saved = memory ? g_env->construct(memory) : nullptr;
    }
    return saved;
}

bool mode2() {
    u8 &guard = gDialog7cc4b4sparkle_fb34bd;
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>(&gDialog74fb20_fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    return gDialog74fb20_fb3b7c == 2;
}

void showFour(u32 shown) {
    vcall<void>(child(0x1e0f0), 0x14, shown);
    vcall<void>(child(0x1e0f1), 0x14, shown);
    g_env->setShown(child(0x1e0ea), nullptr, shown);
    g_env->setShown(child(0x1e0ec), nullptr, shown);
}

// The panel's figures: reached the theme's level, the effect up; else the effect ended.
void figures(u8 *theme, u8 *panel, bool reached) {
    if (reached) {
        g_env->notify1(panel, nullptr, 0);
        const u32 value = g_env->field14(theme) - 1;
        g_env->notify1(Window_FindChild(panel, nullptr, 0x1e0eb), nullptr, value);
        u8 rect[12];
        GuiRect_Ctor(rect);
        g_env->getRect(panel, nullptr, rect, 1);
        const s32 top = at<s16>(rect, 6), bottom = at<s16>(rect, 0xa);
        const s32 y = (s32)((u32)(((bottom - top) >> 1) + top) * 75000) / 0x600;
        if (!reinterpret_cast<s32 &>(gDialog7cc4b4update_7cc4bc)) reinterpret_cast<s32 &>(gDialog7cc4b4update_7cc4bc) = g_env->spawn(0x61, 50000, 0, y);
        GuiRect_Dtor(rect);
    } else {
        g_env->notify1(panel, nullptr, 1);
        const u32 value = g_env->field14(theme) + 4;
        g_env->notify1(Window_FindChild(panel, nullptr, 0x1e0eb), nullptr, value);
        if (gDialog7cc4b4update_7cc4bc) {
            g_env->end(gDialog7cc4b4update_7cc4bc);
            gDialog7cc4b4update_7cc4bc = 0;
        }
    }
}

void themePart(u8 *theme, u8 *panel) {
    if (mode2()) {
        g_env->setShown(panel, nullptr, 0);
        if (dialog()) showFour(0);
        return;
    }
    g_env->setShown(panel, nullptr, 1);
    if (dialog()) showFour(1);
    info();
    const s32 need = g_env->field14(theme);
    figures(theme, panel, g_env->countLevels(gArreplayaction_f7c58c) >= need);
}

}  // namespace

void Dialog7cc4b4_SetUpdateEnv(const Dialog7cc4b4UpdateEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc4b4_Update() {
    if (!dialog()) return;
    const u8 *const game = global<const u8 *>(0x00f85614);
    if (game && at<const u8 *>(game, 4) && at<const u8 *>(at<const u8 *>(game, 4), 0xc)) {
        String name;
        String_CtorStr(&name, nullptr, at<const char *>(at<const u8 *>(at<const u8 *>(game, 4), 0xc), 0x30));
        u8 *const theme = g_env->findTheme(info(), nullptr, &name);
        String_Dtor(&name);
        u8 *const panel = child(0x1e0ea);
        if (theme) themePart(theme, panel);
    }
    u8 *const second = child(0x1e0ec);
    if (mode2()) {
        g_env->setShown(second, nullptr, 0);
        g_env->setFlag(0);
    } else if (g_env->countLevels(info()) > 0) {
        g_env->setShown(second, nullptr, 1);
        if (dialog()) showFour(1);
        g_env->swprintf(reinterpret_cast<u16 *>((uintptr_t)0x007ba058), reinterpret_cast<const u16 *>((uintptr_t)&gText_Dialog7cc4b4update_758194), g_env->countLevels(info()));
        vcall<void>(Window_FindChild(second, nullptr, 0x1e0ee), 0x24, 0x007ba058);
    } else {
        g_env->setShown(second, nullptr, 0);
    }
    gDialog7cc4b4_7cc4b8 = g_env->countLevels(g_env->instance());
}

void __cdecl Dialog7cc4b4_ShowTheme() {
    if (!dialog()) return;
    const u8 *const part = at<const u8 *>(at<const u8 *>(global<const u8 *>(0x00f85614), 4), 0xc);
    u32 text = at<u32>(part, 0x50);
    String name;
    String_CtorStr(&name, nullptr, at<const char *>(part, 0x30));
    u8 *const theme = g_env->findTheme(info(), nullptr, &name);
    String_Dtor(&name);
    u8 *const panel = child(0x1e0ea);
    if (theme) {
        if (mode2()) {
            g_env->setShown(panel, nullptr, 0);
        } else {
            const s32 need = g_env->field14(theme);
            figures(theme, panel, gDialog7cc4b4_7cc4b8 >= need);
        }
        g_env->copyName(theme, nullptr, reinterpret_cast<u16 *>((uintptr_t)0x007ba058), 0x3fff);
        text = 0x007ba058;
    }
    vcall<void>(child(0x1e0ef), 0x24, text);
}
