// The dial-up dialog's handler: see dialog7cc530handler.h.
#include "dialog7cc530handler.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/crt.h"
#include "dialog7cc5f4.h"
#include "dialogbase.h"
#include "dialuplist.h"
#include "focus.h"
#include "window.h"
#include "window7059f0.h"
#include "../online/postcardsend.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const Dialog7cc530HandlerEnv kGameEnv = {&Window_SendMessage, &Focus_Restore, &Dialog_BaseHandler, &Window7059f0_NthItemData, &Dialog7cc5f4_Create, &Dialog7cc5f4_CreateFrom, &DialupList_Fill};
const Dialog7cc530HandlerEnv *g_env = &kGameEnv;

u8 *&dialog() { return global<u8 *>(0x007cc530); }
u32 &owner() { return global<u32>(0x007cc52c); }
u8 *frontWindow() { return *reinterpret_cast<u8 **>(global<u8 *>(0x00f82884) + 4); }

}  // namespace

void Dialog7cc530Handler_SetEnv(const Dialog7cc530HandlerEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

s32 __cdecl Dialog7cc530_Handler(u8 *window, u32 message, u32 a, u32 b) {
    if (message == 0x14) {
        dialog() = nullptr;
        g_env->send(frontWindow(), 6, 1, 0);
        g_env->restore();
    } else if (message == 0x15) {
        g_env->send(frontWindow(), 6, 0, 0);
    } else if (message == 0x101) {
        if (b == 1) g_env->fill(a == 0xe ? 0 : 1);
    } else if (message == 0x100 && a == 0xfffffffe) {
        g_env->send(dialog(), 4, 0, 0);
        if (owner()) reinterpret_cast<void(__cdecl *)()>((uintptr_t)owner())();
        return 0;
    } else if (message == 0x100 && a == 0xffffffff) {
        u8 *const list = Window_FindChild(dialog(), nullptr, 0xc);
        if (Fn_66a281(Window_FindChild(dialog(), nullptr, 0xd)) == 0xf) {
            u32 value;
            g_env->nth(list, nullptr, *reinterpret_cast<s32 *>(list + 0x14c), &value);
            if (static_cast<s32>(value) < 0) return 0;
            u8 properties[0x830];
            global<u32(__fastcall *)(void *, void *, u32, void *)>(0x006fd5bc)(reinterpret_cast<void *>((uintptr_t)0x007c24c0), nullptr, value, properties);
            if (*reinterpret_cast<u32 *>(properties) == 0) {
                g_env->send(dialog(), 4, 0, 0);
                g_env->createFrom(value, properties);
                return 0;
            }
            global<u8>(0x007cc53c) = 1;
            Crt().sprintf(reinterpret_cast<char *>((uintptr_t)0x007cc4f0), gPostcardsend_74f8c4, properties + 0x212);
        }
        g_env->send(dialog(), 4, 0, 0);
        g_env->create(owner());
        return 0;
    }
    return g_env->base(window, message, a, b);
}
}
