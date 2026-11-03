// The text part of a password field: see passwordpart.h.
#include "passwordpart.h"

#include <cstdint>

#include "../core/crt.h"
#include "textparts.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const PasswordPartEnv kGameEnv = {TextPart_SetLayout};
const PasswordPartEnv *g_env = &kGameEnv;

}  // namespace

void PasswordPart_SetEnv(const PasswordPartEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall PasswordPart_SetLayout(u8 *part, void *, u32 flags, const u8 *info) {
    if (!(flags & 0x100)) {
        g_env->setLayout(part, nullptr, flags, info);
        return;
    }
    u16 *const wide = reinterpret_cast<u16 *>((uintptr_t)0x007ba058);
    Crt().wcscpy(wide, at<const u16 *>(at<const void *>(info, 0x10), 0));
    for (u16 *unit = wide; *unit; ++unit) *unit = 0x2a;
    u16 *text = wide;
    u32 copy[6];
    for (u32 i = 0; i < 6; ++i) copy[i] = at<u32>(info, i * 4);
    copy[4] = (u32)(uintptr_t)&text;
    g_env->setLayout(part, nullptr, 0x100, reinterpret_cast<const u8 *>(copy));
}
}
