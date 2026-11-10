// A sign's two lines of text: see signtext.h.
#include "signtext.h"

#include <cstdint>

#include "../online/chatlists.h"
#include "signtext.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
template <typename F> F game(u32 address) { return reinterpret_cast<F>((uintptr_t)address); }

const SignTextEnv kGameEnv = {
    game<u16 *(__cdecl *)(u16 *, const u16 *, u32)>(0x0067b0d0),
    game<u8 *(__fastcall *)(u8 *)>(0x00550ca0),
    game<s32(__cdecl *)(void(__cdecl *)())>(0x0067a7d0),
    game<u16 *(__fastcall *)(const u8 *, void *, u16 *)>(0x0050b350),
    game<void(__fastcall *)(u8 *, void *, const u16 *)>(0x005b0940),
    game<void(__fastcall *)(u8 *, void *, const u16 *)>(0x005b0970),
    game<u8 *(__cdecl *)()>(0x00409410),
    game<void(__fastcall *)(u8 *, void *, u32, u32, u32, const u16 *)>(0x00403fd0),
    game<s32(__cdecl *)(u16 *, u32, u32, const u16 *)>(0x0067c830),
    game<s32(__cdecl *)(u16 *, u32, u32, const u16 *, const u16 *)>(0x0067c830),
};
const SignTextEnv *g_env = &kGameEnv;

bool mode_is_one() {
    u8 &guard = global<u8>(0x00fb34b8);
    if (!(guard & 1)) {
        guard |= 1;
        g_env->readMode(reinterpret_cast<u8 *>((uintptr_t)0x00fb3b7c));
        g_env->atExit(game<void(__cdecl *)()>(0x005d1750));
    }
    return global<u32>(0x00fb3b7c) == 1;
}

// The player's own sign (the game's +0x1da732 against the sign's handle).
bool is_own(const u8 *sign) {
    const u16 own = at<u16>(global<u8 *>(0x0080239c), 0x1da732);
    u16 handle;
    return *g_env->handle(sign, nullptr, &handle) == own;
}

void set_line(u8 *sign, const u16 *text, u32 offset, void(__fastcall *save_line)(u8 *, void *, const u16 *), u32 message) {
    u16 *const line = reinterpret_cast<u16 *>(sign + offset);
    g_env->copy(line, text, 0x21);
    if (!mode_is_one() && is_own(sign)) save_line(g_env->save(), nullptr, text);
    u8 *const net = reinterpret_cast<u8 *>((uintptr_t)0x00785a08);
    const u32 cell = is_own(sign) ? 0xea60 : (((u32)at<u8>(sign, 7) << 7) + at<u8>(sign, 5) + 1) & 0xffff;
    g_env->send(net, nullptr, message, cell, CRT_wcslen(line), text);
}

}  // namespace

void SignText_SetEnv(const SignTextEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall SignText_Format(const u8 *sign, void *, u16 *to, u32 count) {
    const u16 *const first = reinterpret_cast<const u16 *>(sign + 0x3c8);
    const u16 *const second = reinterpret_cast<const u16 *>(sign + 0x40a);
    const u32 length = CRT_wcslen(first);
    if (!length) {
        g_env->copy(to, second, count);
    } else if (first[length - 1] == '-') {
        u16 *const buffer = reinterpret_cast<u16 *>((uintptr_t)0x007cdc10);
        buffer[0x20] = 0;
        g_env->format(buffer, 0x20, (u32)&gText_Signtext_75c0cc, first);
        buffer[length - 1] = 0;
        g_env->format2(to, count, 0x00749850, buffer, second);
    } else if (!CRT_wcslen(second)) {
        g_env->copy(to, first, count);
    } else {
        g_env->format2(to, count, (u32)&gText_Signtext_75c0c0, first, second);
    }
}

void __fastcall SignText_SetFirst(u8 *sign, void *, const u16 *text) { set_line(sign, text, 0x3c8, g_env->saveLineA, 0x50); }

void __fastcall SignText_SetSecond(u8 *sign, void *, const u16 *text) { set_line(sign, text, 0x40a, g_env->saveLineB, 0x55); }
}
