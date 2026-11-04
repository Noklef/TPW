#include "themenames.h"

#include <cstdint>

#include "balancefile.h"
#include "crt.h"
#include "../world/objectdefs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u16 *text(u32 address) { return reinterpret_cast<u16 *>((uintptr_t)address); }
u16 *a_of(u8 *self) { return reinterpret_cast<u16 *>(self + 0x34); }
u16 *b_of(u8 *self) { return reinterpret_cast<u16 *>(self + 0x76); }

u8 *const kDefs = reinterpret_cast<u8 *>((uintptr_t)0x007890a0);
u16 *const kNone = text(0x0078a374);   // (the global empty text)
u16 *const kOne = text(0x0075c0cc);    // "%s"

const ThemeNamesEnv kGameEnv = {&CRT_snwprintf, &CRT_swprintf, &ObjectDefs_GetFullName, &ObjectDefs_GetName, &ObjectDefs_GetSuffix};
const ThemeNamesEnv *g_env = &kGameEnv;

using Defs = void(__fastcall *)(u8 *, void *, u32, u16 *, u32);

// The name of the definitions for the global.sam's id, or the empty text.
void from_defs(u8 *self, Defs get, u16 *to, u32 count) {
    if (ThemeRecord_LoadGlobalSam(self))
        get(kDefs, nullptr, at<u32>(at<u8 *>(self, 0x1c), 0x18), to, count);
    else
        g_env->swprintf(to, kNone);
}

}  // namespace

void ThemeNames_SetEnv(const ThemeNamesEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ThemeRecord_FullName(u8 *self, void *, u16 *to, u32 count) {
    if (!at<u8>(self, 0xb8)) {
        from_defs(self, g_env->fullName, to, count);
        return;
    }
    u16 *const a = a_of(self);
    u16 *const b = b_of(self);
    const u32 length = Crt().wcslen(a);
    if (length == 0) {
        Crt().wcsncpy(to, b, count);
        return;
    }
    if (a[length - 1] == '-') {
        u16 *const copy = text(0x00f7bf78);
        copy[0x20] = 0;
        g_env->snwprintf(copy, 0x20, kOne, a);
        copy[length - 1] = 0;
        g_env->snwprintf(to, count, text(0x00749850), copy, b);
        return;
    }
    if (Crt().wcslen(b) == 0) {
        Crt().wcsncpy(to, a, count);
        return;
    }
    g_env->snwprintf(to, count, text(0x0075c0c0), a, b);
}

void __fastcall ThemeRecord_Name(u8 *self, void *, u16 *to, u32 count) {
    if (!at<u8>(self, 0xb8)) {
        from_defs(self, g_env->name, to, count);
        return;
    }
    u16 *const a = a_of(self);
    const s32 length = static_cast<s32>(Crt().wcslen(a));
    if (length == 0) {
        g_env->snwprintf(to, count, kNone);
        return;
    }
    if (length > 1 && a[length - 1] == '-' && a[length - 2] == '-') {
        u16 *const copy = text(0x00f7c800);
        g_env->snwprintf(copy, 0x20, kOne, a);
        copy[length - 1] = 0;
        g_env->snwprintf(to, count, kOne, copy);
        return;
    }
    g_env->snwprintf(to, count, kOne, a);
}

void __fastcall ThemeRecord_Suffix(u8 *self, void *, u16 *to, u32 count) {
    if (at<u8>(self, 0xb8))
        g_env->snwprintf(to, count, kOne, b_of(self));
    else
        from_defs(self, g_env->suffix, to, count);
}
}
