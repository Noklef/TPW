// The window [0x7cb2cc]'s handler: see window7cb2cc.h.
#include "window7cb2cc.h"

#include <cstdint>

#include "../core/keymap.h"
#include "dialogbase.h"
#include "guirect.h"
#include "window.h"
#include "focus.h"
#include "../core/staticobjects.h"
#include "../core/addrcalls.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const Window7cb2ccEnv kGameEnv = {KeyMap_FindKey, GameAddress<void(__fastcall *)(u8 *, void *, u32)>(0x00409350), Window_GetRect, Window_SetWordsE4, Window_RunScript, Dialog_BaseHandler};
const Window7cb2ccEnv *g_env = &kGameEnv;

}  // namespace

void Window7cb2cc_SetEnv(const Window7cb2ccEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Window7cb2cc_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        reinterpret_cast<u32 &>(gFocus_7cb2cc) = 0;
    } else if (message == 0x15) {
        u8 rect[12];
        GuiRect_Ctor(rect);
        g_env->getRect(window, nullptr, rect, 1);
        g_env->setWords(window, nullptr, (u32)(s32)(s16)(at<s16>(rect, 8) >> 1), (u32)(s32)(s16)(at<s16>(rect, 0xa) >> 1));
        g_env->runScript(window, nullptr, gWindow7cb2cc_Handler_752ed0, 0x23);
        GuiRect_Dtor(rect);
    } else if (message == 0x1000b) {
        const u8 *const map = gKeymap_78718c;
        if (at<u16>(map, 6) == 0 && (u16)g_env->findKey(map, nullptr, (s16)a, (b & 0xff0000) ? 0 : 1)) g_env->key(reinterpret_cast<u8 *>((uintptr_t)&gNamedObjects_786b68), nullptr, 0);
    }
    return g_env->base(window, message, a, b);
}
}
