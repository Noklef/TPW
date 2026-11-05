#include "crtlocaleinfo.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using Info = s32(__stdcall *)(u32, u32, void *, s32);
using ToWide = s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32);
using ToMulti = s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, const char *, s32 *);

Info info_wide() { return global<Info>(0x006fd204); }
Info info_multi() { return global<Info>(0x006fd28c); }

// Which call works: 1 GetLocaleInfoW, 2 GetLocaleInfoA, 0 neither.
u32 probe() {
    if (info_wide()(0, 1, nullptr, 0)) return 1;
    return info_multi()(0, 1, nullptr, 0) ? 2 : 0;
}

}  // namespace

extern "C" {

s32 __cdecl CRT_GetLocaleInfoW(u32 locale, u32 type, u16 *buffer, s32 size, u32 code_page) {
    u32 mode = global<u32>(0x00fb1944);
    const Info wide = info_wide();
    if (!mode) {
        mode = probe();
        if (!mode) return 0;
        global<u32>(0x00fb1944) = mode;
    }
    if (mode == 1) return wide(locale, type, buffer, size);
    if (mode != 2) return static_cast<s32>(mode);
    if (!code_page) code_page = global<u32>(0x00fb1450);
    const s32 length = info_multi()(locale, type, nullptr, 0);
    if (!length) return 0;
    char *const text = static_cast<char *>(CRT_malloc(static_cast<u32>(length)));
    if (!text) return 0;
    s32 result = 0;
    if (info_multi()(locale, type, text, length)) {
        result = global<ToWide>(0x006fd174)(code_page, 1, text, -1, size ? buffer : nullptr, size);
    }
    Crt().free(text);
    return result;
}

s32 __cdecl CRT_GetLocaleInfoA(u32 locale, u32 type, char *buffer, s32 size, u32 code_page) {
    u32 mode = global<u32>(0x00fb1948);
    const Info multi = info_multi();
    if (!mode) {
        mode = probe();
        if (!mode) return 0;
        global<u32>(0x00fb1948) = mode;
    }
    if (mode == 2) return multi(locale, type, buffer, size);
    if (mode != 1) return static_cast<s32>(mode);
    if (!code_page) code_page = global<u32>(0x00fb1450);
    const s32 length = info_wide()(locale, type, nullptr, 0);
    if (!length) return 0;
    u16 *const text = static_cast<u16 *>(CRT_malloc(static_cast<u32>(length) * 2));
    if (!text) return 0;
    s32 result = 0;
    if (info_wide()(locale, type, text, length)) {
        result = global<ToMulti>(0x006fd1dc)(code_page, 0x220, text, -1, size ? buffer : nullptr, size, nullptr, nullptr);
    }
    Crt().free(text);
    return result;
}

s32 __cdecl CRT_Getlocaleinfo(s32 kind, u32 locale, u32 type, void *out) {
    if (kind == 1) {
        char frame[0x80];
        char *text = frame;
        bool allocated = false;
        s32 size = CRT_GetLocaleInfoA(locale, type, frame, 0x80, 0);
        if (!size) {
            if (global<u32(__stdcall *)()>(0x006fd138)() != 0x7a) return -1;
            size = CRT_GetLocaleInfoA(locale, type, nullptr, 0, 0);
            if (!size) return -1;
            text = static_cast<char *>(CRT_malloc(static_cast<u32>(size)));
            if (!text) return -1;
            allocated = true;
            size = CRT_GetLocaleInfoA(locale, type, text, size, 0);
            if (!size) {
                Crt().free(text);
                return -1;
            }
        }
        char *const copy = static_cast<char *>(CRT_malloc(static_cast<u32>(size)));
        *static_cast<char **>(out) = copy;
        if (!copy) {
            if (allocated) Crt().free(text);
            return -1;
        }
        Crt().strncpy(copy, text, static_cast<u32>(size));
        if (allocated) Crt().free(text);
        return 0;
    }
    if (kind != 0) return -1;
    if (!CRT_GetLocaleInfoW(locale, type, reinterpret_cast<u16 *>((uintptr_t)0x00fb1938), 4, 0)) return -1;
    u8 *const value = static_cast<u8 *>(out);
    *value = 0;
    for (u32 i = 0; i < 4; ++i) {
        const u8 c = global<u8>(0x00fb1938 + 2 * i);
        const s32 digit = global<s32>(0x0077e7dc) > 1 ? CRT_isctype(c, 4) : global<const u8 *>(0x0077ea90)[c * 2] & 4;
        if (!digit) return 0;
        *value = static_cast<u8>(*value * 10 + c - 0x30);
    }
    return 0;
}

}  // extern "C"
