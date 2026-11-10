// The IME host taken to a text window and off it: see imehostattach.h.
#include "imehostattach.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/ime.h"
#include "window.h"
#include "window705a50.h"
#include "../core/gamewindow.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const ImeHostAttachEnv kGameEnv = {reinterpret_cast<void(__fastcall *)(u8 *, void *, s32 *, u32)>((uintptr_t)0x0047a3a0), reinterpret_cast<void(__fastcall *)(u8 *)>((uintptr_t)0x0047a570), Window705a50_TextLength, Ime_SetLimit, Fn_668564, Window_SendMessage};
const ImeHostAttachEnv *g_env = &kGameEnv;

}  // namespace

void ImeHost_SetAttachEnv(const ImeHostAttachEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ImeHost_Attach(u8 *host, void *, u8 *window) {
    u8 *const ime = host + 0x1c;
    s32 result;
    g_env->open(ime, nullptr, &result, gGamewindow_7a130c);
    if (result < 0) {
        at<u8>(host, 0xc) = 0;
        return;
    }
    at<u8>(host, 0xc) = 1;
    at<u8 *>(host, 4) = window;
    s32 limit = at<s16>(window, 0x13c);
    g_env->setLimit(ime, nullptr, (u32)(limit - (s16)g_env->textLength(window)));
    u8 *const again = at<u8 *>(host, 4);
    at<u16>(host, 0x1a8) = 0;
    at<u16>(host, 0x1aa) = 1;
    limit = at<s16>(again, 0x13c);
    g_env->setLimit(ime, nullptr, (u32)(limit - (s16)g_env->textLength(again)));
}

void __fastcall ImeHost_Detach(u8 *host) {
    g_env->close(host + 0x1c);
    if (u8 *const window = at<u8 *>(host, 4)) g_env->windowOff(window, nullptr, 0);
    u8 *const owner = at<u8 *>(host, 0);
    at<u8 *>(host, 4) = nullptr;
    if (owner) g_env->send(owner, 4, 0, 0);
    at<u8>(host, 0xc) = 0;
}
}
