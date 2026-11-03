// The window [0x7cb2a4]: see window7cb2a4.h.
#include "window7cb2a4.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/globalsave.h"
#include "../core/options.h"
#include "../core/vcall.h"
#include "window.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

s32 __cdecl Format(u16 *to, const u16 *format, s32 value) { return Crt().swprintf(to, format, value); }

const Window7cb2a4Env kGameEnv = {Setting_ReadMode, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), GlobalSave_Instance, GlobalSave_CountSetLevel, GlobalSave_CountSetLeft, Window_SetShown, Format};
const Window7cb2a4Env *g_env = &kGameEnv;

void show(u8 *window, u32 number, u32 back, u32 shown) {
    g_env->setShown(Window_FindChild(window, nullptr, number), nullptr, shown);
    g_env->setShown(Window_FindChild(window, nullptr, back), nullptr, shown);
}

void tell(u8 *window, u32 number, s32 count) {
    u16 text[8];
    g_env->format(text, reinterpret_cast<const u16 *>((uintptr_t)0x00752ef8), count);
    vcall<void>(Window_FindChild(window, nullptr, number), 0x24, text);
}

}  // namespace

void Window7cb2a4_SetEnv(const Window7cb2a4Env *env) { g_env = env ? env : &kGameEnv; }

extern "C" s32 __cdecl Window7cb2a4_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x15) {
        global<s32>(0x00752ec4) = -1;
        global<s32>(0x00752ec8) = -1;
    } else if (message == 0x1e) {
        u8 &guard = global<u8>(0x00fb34c1);
        if (!(guard & 1)) {
            guard |= 1;
            g_env->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
            g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
        }
        if (global<u32>(0x00fb3b7c) != 2 && g_env->level(g_env->save()) != global<s32>(0x00752ec8)) {
            global<s32>(0x00752ec8) = g_env->level(g_env->save());
            if (g_env->level(g_env->save())) {
                show(window, 0x36, 0x34, 1);
                tell(window, 0x36, g_env->level(g_env->save()));
            } else {
                show(window, 0x36, 0x34, 0);
            }
        }
        const s32 left = g_env->left(g_env->save());
        if (left != global<s32>(0x00752ec4)) {
            global<s32>(0x00752ec4) = left;
            show(window, 0x37, 0x35, left ? 1 : 0);
            if (left) tell(window, 0x37, left);
        }
    }
    return Window_CallDefault(window, nullptr, message, a, b);
}
