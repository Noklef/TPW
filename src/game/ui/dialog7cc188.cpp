// The list of the dialog [0x7cc188]: see dialog7cc188.h.
#include "dialog7cc188.h"

#include <cstdint>

#include "../core/crt.h"
#include "../core/language.h"
#include "../core/options.h"
#include "../core/stdlist.h"
#include "../core/trivial.h"
#include "../online/session.h"
#include "window.h"
#include "window7059f0.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

s32 __cdecl Swprintf(u16 *to, const u16 *format, const char *text) { return Crt().swprintf(to, format, text); }

const Dialog7cc188ListEnv kGameEnv = {reinterpret_cast<u8 *(__cdecl *)()>((uintptr_t)0x0048bcd0), &Swprintf, reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0), &Window7059f0_Clear, &Window7059f0_AddItem, &Window7059f0_RefreshRows, &Language_WideText};
const Dialog7cc188ListEnv *g_env = &kGameEnv;

const char *textOf(const StdString *string) {
    const char *const text = at<const char *>(string, 4);
    return text ? text : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0);
}

}  // namespace

void Dialog7cc188_SetListEnv(const Dialog7cc188ListEnv *env) { g_env = env ? env : &kGameEnv; }

void __cdecl Dialog7cc188_FillList(u32 clear) {
    u8 *const strings = g_env->source() + 0x100;
    const char *text = textOf(StdListString_First(strings));
    u8 *const list = Window_FindChild(global<u8 *>(0x007cc188), nullptr, 0x26);
    if (clear) g_env->clear(list);
    u32 texts[1];
    for (u32 index = 0; *text; ++index) {
        u16 buffer[0x40];
        g_env->swprintf(buffer, reinterpret_cast<const u16 *>((uintptr_t)0x0075442c), text);
        texts[0] = (u32)(uintptr_t)buffer;
        g_env->add(list, nullptr, texts, index, -1, 0);
        text = textOf(StdListString_Next(strings));
    }
    u8 &guard = global<u8>(0x00fb34bf);
    if (!(guard & 1)) {
        guard |= 1;
        Setting_ReadMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x005d1750));
    }
    if (global<u32>(0x00fb3b7c) == 1) {
        const u16 *const name = at<const u16 *>(reinterpret_cast<const void *>((uintptr_t)Obj_Get3a4(Online_Instance())), 0x9c);
        if (name && Crt().wcslen(name)) {
            texts[0] = (u32)(uintptr_t)g_env->wideText(0x121);
            g_env->add(list, nullptr, texts, 0xffffffff, -1, 0);
        }
    }
    g_env->refresh(list);
}
