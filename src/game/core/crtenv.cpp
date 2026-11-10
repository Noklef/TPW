#include "crtenv.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "crtmbstring.h"
#include "crtlocale.h"
#include "crttime.h"
#include "imports.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

bool lead_byte(u8 c) { return (&CRT_mbctype[1])[c] & 4; }

char **&environment() { return CRT_environ; }

s32 compare_a(u32 locale, u32 flags, u32 a, s32 a_length, u32 b, s32 b_length) {
    return import_slot<s32(__stdcall *)(u32, u32, u32, s32, u32, s32)>(IMPORT(Kernel32_CompareStringA))(locale, flags, a, a_length, b, b_length);
}

s32 compare_w(u32 locale, u32 flags, u32 a, s32 a_length, u32 b, s32 b_length) {
    return import_slot<s32(__stdcall *)(u32, u32, u32, s32, u32, s32)>(IMPORT(Kernel32_CompareStringW))(locale, flags, a, a_length, b, b_length);
}

s32 to_wide(u32 code_page, u32 flags, const char *text, s32 length, u16 *out, s32 room) {
    return import_slot<s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32)>(IMPORT(Kernel32_MultiByteToWideChar))(code_page, flags, text, length, out, room);
}

s32 to_narrow(u32 code_page, u32 flags, const u16 *text, s32 length, char *out, s32 room, void *used, s32 *lossy) {
    return import_slot<s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, void *, s32 *)>(IMPORT(Kernel32_WideCharToMultiByte))(code_page, flags, text, length, out, room, used, lossy);
}

// A lead byte range of the code page's holds c.
bool in_lead_ranges(const u8 *ranges, u8 c) {
    for (; ranges[0] && ranges[1]; ranges += 2) {
        if (c >= ranges[0] && c <= ranges[1]) return true;
    }
    return false;
}

u32 unconvertible() {
    *CRT_errno() = 0x2a;
    return 0xffffffff;
}

bool digit(char c) { return c >= '0' && c <= '9'; }

}  // namespace

extern "C" {

s32 __cdecl CRT_crtCompareStringA(u32 locale, u32 flags, const char *a, s32 a_length, const char *b, s32 b_length, u32 code_page) {
    if (!CRT_CompareString_use_fb1910) {
        const u32 wide = (u32)(uintptr_t)CRT_text_empty_wide_708da8, narrow = (u32)(uintptr_t)CRT_text_empty_708da4;
        if (compare_w(0, 0, wide, 1, wide, 1)) CRT_CompareString_use_fb1910 = 1;
        else if (compare_a(0, 0, narrow, 1, narrow, 1)) CRT_CompareString_use_fb1910 = 2;
        else return 0;
    }
    if (a_length > 0) a_length = static_cast<s32>(CRT_strncnt(a, static_cast<u32>(a_length)));
    if (b_length > 0) b_length = static_cast<s32>(CRT_strncnt(b, static_cast<u32>(b_length)));
    const u32 mode = CRT_CompareString_use_fb1910;
    if (mode == 2) return compare_a(locale, flags, static_cast<u32>(reinterpret_cast<uintptr_t>(a)), a_length, static_cast<u32>(reinterpret_cast<uintptr_t>(b)), b_length);
    if (mode != 1) return static_cast<s32>(mode);
    if (!code_page) code_page = CRT_lc_codepage;
    if (!a_length || !b_length) {
        if (a_length == b_length) return 2;
        if (b_length > 1) return 1;
        if (a_length > 1) return 3;
        struct {
            u32 max_size;
            u8 fallback[2];
            u8 lead[12];
        } info;
        const s32 got = import_slot<s32(__stdcall *)(u32, void *)>(IMPORT(Kernel32_GetCPInfo))(code_page, &info);
        if (!got) return got;
        if (a_length > 0) {
            if (info.max_size < 2) return 3;
            return in_lead_ranges(info.lead, static_cast<u8>(*a)) ? 2 : 3;
        }
        if (b_length > 0) {
            if (info.max_size < 2) return 1;
            return in_lead_ranges(info.lead, static_cast<u8>(*b)) ? 2 : 1;
        }
    }
    const s32 a_units = to_wide(code_page, 9, a, a_length, nullptr, 0);
    if (!a_units) return 0;
    u16 *const a_wide = static_cast<u16 *>(CRT_malloc(static_cast<u32>(a_units) * 2));
    if (!a_wide) return 0;
    s32 result = 0;
    u16 *b_wide = nullptr;
    if (to_wide(code_page, 1, a, a_length, a_wide, a_units)) {
        const s32 b_units = to_wide(code_page, 9, b, b_length, nullptr, 0);
        if (b_units) {
            b_wide = static_cast<u16 *>(CRT_malloc(static_cast<u32>(b_units) * 2));
            if (b_wide && to_wide(code_page, 1, b, b_length, b_wide, b_units)) {
                result = compare_w(locale, flags, static_cast<u32>(reinterpret_cast<uintptr_t>(a_wide)), a_units, static_cast<u32>(reinterpret_cast<uintptr_t>(b_wide)), b_units);
            }
        }
    }
    Crt().free(a_wide);
    Crt().free(b_wide);
    return result;
}

s32 __cdecl CRT_mbsnbicoll(const char *a, const char *b, u32 n) {
    if (!n) return 0;
    const s32 result = CRT_crtCompareStringA(CRT_mblcid, 1, a, static_cast<s32>(n), b, static_cast<s32>(n), CRT_mbcodepage);
    return result ? result - 2 : 0x7fffffff;
}

const char *__cdecl CRT_mbschr(const char *text, u32 value) {
    if (!CRT_mbcodepage) return CRT_strchr(text, static_cast<s32>(value));
    CrtInner().lock(0x19);
    const char *at = text;
    u32 c = static_cast<u8>(*at);
    while (c) {
        if (lead_byte(static_cast<u8>(c))) {
            const u8 trail = static_cast<u8>(*++at);
            if (!trail) {
                CrtInner().unlock(0x19);
                return nullptr;
            }
            if (value == (c << 8 | trail)) {
                CrtInner().unlock(0x19);
                return at - 1;
            }
        } else if (value == c) {
            break;
        }
        c = static_cast<u8>(*++at);
    }
    CrtInner().unlock(0x19);
    return value == c ? at : nullptr;
}

char *__cdecl CRT_strdup(const char *text) {
    if (!text) return nullptr;
    char *const copy = static_cast<char *>(CRT_malloc(static_cast<u32>(std::strlen(text)) + 1));
    if (copy) std::strcpy(copy, text);
    return copy;
}

s32 __cdecl CRT_findenv(const char *name, u32 length) {
    char **entry = environment();
    for (; *entry; ++entry) {
        if (!CRT_mbsnbicoll(name, *entry, length)) {
            const char c = (*entry)[length];
            if (c == '=' || !c) return static_cast<s32>(entry - environment());
        }
    }
    return -static_cast<s32>(entry - environment());
}

char **__cdecl CRT_copy_environ(char **from) {
    if (!from) return nullptr;
    u32 count = 0;
    if (*from) {
        while (from[count + 1]) ++count;
        ++count;
    }
    char **const copy = static_cast<char **>(CRT_malloc((count + 1) * 4));
    if (!copy) CrtInner().amsgExit(9);
    char **out = copy;
    for (; *from; ++from) *out++ = CRT_strdup(*from);
    *out = nullptr;
    return copy;
}

s32 __cdecl CRT_crtsetenv(char *option, s32 primary) {
    if (!option) return -1;
    const char *const equals = CRT_mbschr(option, '=');
    if (!equals || equals == option) return -1;
    const bool remove = !equals[1];
    if (environment() == CRT_initenv) environment() = CRT_copy_environ(environment());
    if (!environment()) {
        if (primary && CRT_wenviron) {
            if (CRT_wtomb_environ()) return -1;
        } else {
            if (remove) return 0;
            environment() = static_cast<char **>(CRT_malloc(4));
            if (!environment()) return -1;
            *environment() = nullptr;
            if (!CRT_wenviron) {
                u16 **const wide = static_cast<u16 **>(CRT_malloc(4));
                CRT_wenviron = wide;
                if (!wide) return -1;
                *wide = nullptr;
            }
        }
    }
    char **const list = environment();
    const u32 name_length = static_cast<u32>(equals - option);
    s32 index = CRT_findenv(option, name_length);
    if (index >= 0 && *list) {
        if (remove) {
            Crt().free(list[index]);
            for (; list[index]; ++index) list[index] = list[index + 1];
            void *const smaller = CrtInner().realloc(list, static_cast<u32>(index) * 4);
            if (smaller) environment() = static_cast<char **>(smaller);
        } else {
            list[index] = option;
        }
    } else {
        if (remove) return 0;
        if (index < 0) index = -index;
        char **const larger = static_cast<char **>(CrtInner().realloc(list, static_cast<u32>(index) * 4 + 8));
        if (!larger) return -1;
        larger[index] = option;
        larger[index + 1] = nullptr;
        environment() = larger;
    }
    if (primary) {
        char *const pair = static_cast<char *>(CRT_malloc(static_cast<u32>(std::strlen(option)) + 2));
        if (pair) {
            std::strcpy(pair, option);
            pair[name_length] = 0;
            import_slot<s32(__stdcall *)(const char *, const char *)>(IMPORT(Kernel32_SetEnvironmentVariableA))(pair, remove ? nullptr : pair + name_length + 1);
            Crt().free(pair);
        }
    }
    return 0;
}

s32 __cdecl CRT_wtomb_environ() {
    u16 **entry = CRT_wenviron;
    for (; *entry; ++entry) {
        const s32 size = to_narrow(1, 0, *entry, -1, nullptr, 0, nullptr, nullptr);
        if (!size) return -1;
        char *const text = static_cast<char *>(CRT_malloc(static_cast<u32>(size)));
        if (!text) return -1;
        if (!to_narrow(1, 0, *entry, -1, text, size, nullptr, nullptr)) return -1;
        CRT_crtsetenv(text, 0);
    }
    return 0;
}

char *__cdecl CRT_getenv(const char *name) {
    char **entry = environment();
    if (!entry) {
        if (!CRT_wenviron) return nullptr;
        if (CRT_wtomb_environ()) return nullptr;
        entry = environment();
        if (!entry) return nullptr;
    }
    if (!name) return nullptr;
    const u32 length = static_cast<u32>(std::strlen(name));
    for (; *entry; ++entry) {
        if (std::strlen(*entry) > length && (*entry)[length] == '=' && !CRT_mbsnbicoll(*entry, name, length)) return *entry + length + 1;
    }
    return nullptr;
}

u32 __cdecl CRT_wcsncnt(const u16 *text, u32 n) {
    for (u32 i = 0; i < n; ++i) {
        if (!text[i]) return i + 1;
    }
    return n;
}

u32 __cdecl CRT_wcsnlen(const u16 *text, u32 n) {
    for (u32 i = 0; i < n; ++i) {
        if (!text[i]) return i;
    }
    return n;
}

void __cdecl CRT_setenvp() {
    char *&block = *reinterpret_cast<char **>((uintptr_t)0x00fb1354);
    u32 names = 0;
    for (const char *at = block; *at; at += std::strlen(at) + 1) {
        if (*at != '=') ++names;
    }
    char **next = static_cast<char **>(CRT_malloc(names * 4 + 4));
    CRT_environ = next;
    if (!next) CrtInner().amsgExit(9);
    for (const char *at = block; *at; at += std::strlen(at) + 1) {
        if (*at == '=') continue;
        const u32 size = static_cast<u32>(std::strlen(at)) + 1;
        *next = static_cast<char *>(CRT_malloc(size));
        if (!*next) CrtInner().amsgExit(9);
        std::memcpy(*next, at, size);
        ++next;
    }
    Crt().free(block);
    block = nullptr;
    *next = nullptr;
}

char *__cdecl CRT_crtGetEnvironmentStringsA() {
    using GetW = u16 *(__stdcall *)();
    using GetA = char *(__stdcall *)();
    using Release = s32(__stdcall *)(void *);
    u32 &mode = CRT_GetEnvironmentStrings_use_fb1660;
    u16 *wide = nullptr;
    char *narrow = nullptr;
    if (!mode) {
        if ((wide = import_slot<GetW>(IMPORT(Kernel32_GetEnvironmentStringsW))()) != nullptr)    // GetEnvironmentStringsW
            mode = 1;
        else if ((narrow = import_slot<GetA>(IMPORT(Kernel32_GetEnvironmentStrings))()) != nullptr)    // GetEnvironmentStrings
            mode = 2;
        else
            return nullptr;
    }
    if (mode == 1) {
        if (!wide && !(wide = import_slot<GetW>(IMPORT(Kernel32_GetEnvironmentStringsW))())) return nullptr;
        const u16 *end = wide;
        if (*end) {
            do {
                do ++end;
                while (*end);
                ++end;
            } while (*end);
        }
        const s32 units = static_cast<s32>(end - wide) + 1;
        char *copy = nullptr;
        const s32 size = to_narrow(0, 0, wide, units, nullptr, 0, nullptr, nullptr);
        if (size && (copy = static_cast<char *>(CRT_malloc(static_cast<u32>(size)))) != nullptr) {
            if (!to_narrow(0, 0, wide, units, copy, size, nullptr, nullptr)) {
                Crt().free(copy);
                copy = nullptr;
            }
        }
        import_slot<Release>(IMPORT(Kernel32_FreeEnvironmentStringsW))(wide);    // FreeEnvironmentStringsW
        return copy;
    }
    if (mode != 2) return nullptr;
    if (!narrow && !(narrow = import_slot<GetA>(IMPORT(Kernel32_GetEnvironmentStrings))())) return nullptr;
    const char *end = narrow;
    if (*end) {
        do {
            do ++end;
            while (*end);
            ++end;
        } while (*end);
    }
    const u32 size = static_cast<u32>(end - narrow) + 1;
    char *const copy = static_cast<char *>(CRT_malloc(size));
    if (copy) std::memcpy(copy, narrow, size);
    import_slot<Release>(IMPORT(Kernel32_FreeEnvironmentStringsA))(narrow);    // FreeEnvironmentStringsA
    return copy;
}

u32 __cdecl CRT_wcstombs_lk(char *to, const u16 *from, u32 n) {
    s32 lossy = 0;
    const u32 code_page = CRT_lc_codepage;
    if (to && !n) return 0;
    if (!to) {
        if (!CRT_lc_handle[2]) return Crt().wcslen(from);
        const s32 size = to_narrow(code_page, 0x220, from, -1, nullptr, 0, nullptr, &lossy);
        if (!size || lossy) return unconvertible();
        return static_cast<u32>(size) - 1;
    }
    if (!CRT_lc_handle[2]) {
        for (u32 i = 0; i < n; ++i) {
            if (from[i] > 0xff) return unconvertible();
            to[i] = static_cast<char>(from[i]);
            if (!from[i]) return i;
        }
        return n;
    }
    const s32 widest = CRT_mb_cur_max;
    if (widest == 1) {
        const u32 units = CRT_wcsncnt(from, n);
        const s32 size = to_narrow(code_page, 0x220, from, static_cast<s32>(units), to, static_cast<s32>(units), nullptr, &lossy);
        if (!size || lossy) return unconvertible();
        return static_cast<u32>(size) - (to[size - 1] ? 0 : 1);
    }
    const s32 size = to_narrow(code_page, 0x220, from, -1, to, static_cast<s32>(n), nullptr, &lossy);
    if (size) {
        if (lossy) return unconvertible();
        return static_cast<u32>(size) - 1;
    }
    if (lossy || import_slot<u32(__stdcall *)()>(IMPORT(Kernel32_GetLastError))() != 0x7a) return unconvertible();
    u32 done = 0;
    while (done < n) {
        char one[16];   // (sic: the original's is the argument's slot on the stack, room for 8 bytes before the return address)
        const s32 got = to_narrow(CRT_lc_codepage, 0, from, 1, one, CRT_mb_cur_max, nullptr, &lossy);
        if (!got || lossy) return unconvertible();
        if (static_cast<u32>(got) + done > n) return done;
        for (s32 i = 0; i < got; ++i) {
            to[done] = one[i];
            if (!one[i]) return done;
            ++done;
        }
        ++from;
    }
    return done;
}

u32 __cdecl CRT_wcstombs(char *to, const u16 *from, u32 n) {
    const u32 readers = (u32)(uintptr_t)&CRT_unguarded_readlc_active;
    const auto decrement = import_slot<s32(__stdcall *)(u32)>(IMPORT(Kernel32_InterlockedDecrement));
    import_slot<s32(__stdcall *)(u32)>(IMPORT(Kernel32_InterlockedIncrement))(readers);
    const bool locked = CRT_setlc_active != 0;
    if (locked) {
        decrement(readers);
        CrtInner().lock(0x13);
    }
    const u32 result = CRT_wcstombs_lk(to, from, n);
    if (locked) CrtInner().unlock(0x13);
    else decrement(readers);
    return result;
}

void __cdecl CRT_tzset() {
    CrtInner().lock(0xc);
    CRT_tzapiused = 0;
    CRT_dstend.year = -1;
    CRT_dststart.year = -1;
    const char *zone = CRT_getenv(CRT_text_TZ_708da0);
    char *&standard = CRT_tzname[0];
    char *&daylight = CRT_tzname[1];
    s32 &offset = CRT_timezone;
    if (!zone) {
        CrtInner().unlock(0xc);
        if (import_slot<s32(__stdcall *)(u32)>(IMPORT(Kernel32_GetTimeZoneInformation))((u32)(uintptr_t)&CRT_tzinfo) == -1) return;
        const s32 bias = CRT_tzinfo.standard_bias;
        CRT_tzapiused = 1;
        offset = CRT_tzinfo.bias * 60;
        if (CRT_tzinfo.standard_date[1]) offset += bias * 60;
        if (CRT_tzinfo.daylight_date[1] && CRT_tzinfo.daylight_bias) {
            CRT_daylight = 1;
            CRT_dstbias = (CRT_tzinfo.daylight_bias - bias) * 60;
        } else {
            CRT_daylight = 0;
            CRT_dstbias = 0;
        }
        CRT_wcstombs(standard, CRT_tzinfo.standard_name, 0x40);
        CRT_wcstombs(daylight, CRT_tzinfo.daylight_name, 0x40);
        daylight[0x3f] = 0;
        standard[0x3f] = 0;
        return;
    }
    char *const last = CRT_lastTZ;
    if (!*zone || (last && !std::strcmp(zone, last))) {
        CrtInner().unlock(0xc);
        return;
    }
    Crt().free(last);
    char *const copy = static_cast<char *>(CRT_malloc(static_cast<u32>(std::strlen(zone)) + 1));
    CRT_lastTZ = copy;
    if (!copy) {
        CrtInner().unlock(0xc);
        return;
    }
    std::strcpy(copy, zone);
    CrtInner().unlock(0xc);
    Crt().strncpy(standard, zone, 3);
    standard[3] = 0;
    zone += 3;
    const bool negative = *zone == '-';
    if (negative) ++zone;
    offset = CRT_atol(zone) * 3600;
    while (*zone == '+' || digit(*zone)) ++zone;
    if (*zone == ':') {
        ++zone;
        offset += CRT_atol(zone) * 60;
        while (digit(*zone)) ++zone;
        if (*zone == ':') {
            ++zone;
            offset += CRT_atol(zone);
            while (digit(*zone)) ++zone;
        }
    }
    if (negative) offset = -offset;
    const s32 more = *zone;
    CRT_daylight = more;
    if (more) {
        Crt().strncpy(daylight, zone, 3);
        daylight[3] = 0;
    } else {
        daylight[0] = 0;
    }
}

char *__cdecl CRT_getenv_locked(const char *name) {
    CrtInner().lock(0xc);
    char *const value = CRT_getenv(name);
    CrtInner().unlock(0xc);
    return value;
}

}
