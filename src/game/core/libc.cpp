// The game's __stdcall wrappers around the C library: see libc.h.
#include "libc.h"

#include <cstring>

namespace {

template <typename T>
T at(u32 address) {
    return reinterpret_cast<T>(static_cast<uintptr_t>(address));
}
const LibcEnv kGameEnv = {
    at<char *(__cdecl *)(char *, const char *, u32)>(0x0067ae00),
    at<char *(__cdecl *)(char *, const char *, u32)>(0x0067b8c0),
    at<s32(__cdecl *)(const char *)>(0x0067a520),
    at<u32(__cdecl *)(u32, u32)>(0x0067f330),
    at<s32(__cdecl *)(char *, const char *, void *)>(0x0067c1e0),
    at<s32(__cdecl *)(char *, u32, const char *, void *)>(0x0067f3a0),
    at<u32(__cdecl *)(const u16 *)>(0x0067acd0),
    at<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bf10),
    at<u16 *(__cdecl *)(u16 *, const u16 *, u32)>(0x0067b0d0),
    at<u16 *(__cdecl *)(u16 *, const u16 *)>(0x0067bed0),
    at<u16 *(__cdecl *)(u16 *, const u16 *, u32)>(0x0067f450),
    at<s32(__cdecl *)(const u16 *)>(0x0067f4a0),
    at<s32(__cdecl *)(u16 *, const u16 *, void *)>(0x0067f4d0),
};
const LibcEnv *g_env = &kGameEnv;

constexpr u32 kDigit = 4;  // _DIGIT

u32 length(const char *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}

// REP MOVSD then REP MOVSB: forward, a dword at a time, as the originals copy.
void copy(char *dst, const char *src, u32 n) {
    for (u32 i = 0; i < n / 4; ++i, dst += 4, src += 4) {
        u32 v;
        std::memcpy(&v, src, 4);
        std::memcpy(dst, &v, 4);
    }
    for (u32 i = 0; i < n % 4; ++i) *dst++ = *src++;
}

}  // namespace

void Libc_SetEnv(const LibcEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

u32 __stdcall Lib_StrLen(const char *s) { return length(s); }

char *__stdcall Lib_StrCpy(char *dst, const char *src) {
    copy(dst, src, length(src) + 1);
    return dst;
}

char *__stdcall Lib_StrNCpy(char *dst, const char *src, u32 count) { return g_env->strncpy(dst, src, count); }

char *__stdcall Lib_StrCat(char *dst, const char *src) {
    const u32 n = length(src) + 1;
    copy(dst + length(dst), src, n);
    return dst;
}

char *__stdcall Lib_StrNCat(char *dst, const char *src, u32 count) { return g_env->strncat(dst, src, count); }

s32 __stdcall Lib_AtoI(const char *s) { return g_env->atoi(s); }

u32 __stdcall Lib_IsDigit(char c) {
    return g_env->iswctype(static_cast<u16>(static_cast<s16>(static_cast<signed char>(c))), kDigit);
}

s32 __stdcall Lib_VSPrintf(char *dst, const char *format, void *args) { return g_env->vsprintf(dst, format, args); }

s32 __stdcall Lib_VSNPrintf(char *dst, u32 size, const char *format, void *args) {
    return g_env->vsnprintf(dst, size, format, args);
}

u32 __stdcall Lib_WcsLen(const u16 *s) { return g_env->wcslen(s); }

u16 *__stdcall Lib_WcsCpy(u16 *dst, const u16 *src) { return g_env->wcscpy(dst, src); }

u16 *__stdcall Lib_WcsNCpy(u16 *dst, const u16 *src, u32 count) { return g_env->wcsncpy(dst, src, count); }

u16 *__stdcall Lib_WcsCat(u16 *dst, const u16 *src) { return g_env->wcscat(dst, src); }

u16 *__stdcall Lib_WcsNCat(u16 *dst, const u16 *src, u32 count) { return g_env->wcsncat(dst, src, count); }

s32 __stdcall Lib_WtoI(const u16 *s) { return g_env->wtoi(s); }

u32 __stdcall Lib_IsDigitW(u32 c) { return g_env->iswctype(c, kDigit); }

s32 __stdcall Lib_VSWPrintf(u16 *dst, const u16 *format, void *args) { return g_env->vswprintf(dst, format, args); }

}  // extern "C"
