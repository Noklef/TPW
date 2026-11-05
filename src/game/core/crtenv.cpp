#include "crtenv.h"

#include <cstdint>
#include <cstring>

#include "crt.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

bool lead_byte(u8 c) { return reinterpret_cast<const u8 *>((uintptr_t)0x00fb1669)[c] & 4; }

char **&environment() { return global<char **>(0x00fb1380); }

s32 compare_a(u32 locale, u32 flags, u32 a, s32 a_length, u32 b, s32 b_length) {
    return import_slot<s32(__stdcall *)(u32, u32, u32, s32, u32, s32)>(0x006fd270)(locale, flags, a, a_length, b, b_length);
}

s32 compare_w(u32 locale, u32 flags, u32 a, s32 a_length, u32 b, s32 b_length) {
    return import_slot<s32(__stdcall *)(u32, u32, u32, s32, u32, s32)>(0x006fd274)(locale, flags, a, a_length, b, b_length);
}

s32 to_wide(u32 code_page, u32 flags, const char *text, s32 length, u16 *out, s32 room) {
    return import_slot<s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32)>(0x006fd174)(code_page, flags, text, length, out, room);
}

s32 to_narrow(u32 code_page, u32 flags, const u16 *text, s32 length, char *out, s32 room, void *used, s32 *lossy) {
    return import_slot<s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, void *, s32 *)>(0x006fd1dc)(code_page, flags, text, length, out, room, used, lossy);
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
    if (!global<u32>(0x00fb1910)) {
        if (compare_w(0, 0, 0x00708da8, 1, 0x00708da8, 1)) global<u32>(0x00fb1910) = 1;
        else if (compare_a(0, 0, 0x00708da4, 1, 0x00708da4, 1)) global<u32>(0x00fb1910) = 2;
        else return 0;
    }
    if (a_length > 0) a_length = static_cast<s32>(CRT_strncnt(a, static_cast<u32>(a_length)));
    if (b_length > 0) b_length = static_cast<s32>(CRT_strncnt(b, static_cast<u32>(b_length)));
    const u32 mode = global<u32>(0x00fb1910);
    if (mode == 2) return compare_a(locale, flags, static_cast<u32>(reinterpret_cast<uintptr_t>(a)), a_length, static_cast<u32>(reinterpret_cast<uintptr_t>(b)), b_length);
    if (mode != 1) return static_cast<s32>(mode);
    if (!code_page) code_page = global<u32>(0x00fb1450);
    if (!a_length || !b_length) {
        if (a_length == b_length) return 2;
        if (b_length > 1) return 1;
        if (a_length > 1) return 3;
        struct {
            u32 max_size;
            u8 fallback[2];
            u8 lead[12];
        } info;
        const s32 got = import_slot<s32(__stdcall *)(u32, void *)>(0x006fd180)(code_page, &info);
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
    const s32 result = CRT_crtCompareStringA(global<u32>(0x00fb1874), 1, a, static_cast<s32>(n), b, static_cast<s32>(n), global<u32>(0x00fb1870));
    return result ? result - 2 : 0x7fffffff;
}

const char *__cdecl CRT_mbschr(const char *text, u32 value) {
    if (!global<u32>(0x00fb1870)) return CRT_strchr(text, static_cast<s32>(value));
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
    if (environment() == global<char **>(0x00fb1384)) environment() = CRT_copy_environ(environment());
    if (!environment()) {
        if (primary && global<void *>(0x00fb1388)) {
            if (CRT_wtomb_environ()) return -1;
        } else {
            if (remove) return 0;
            environment() = static_cast<char **>(CRT_malloc(4));
            if (!environment()) return -1;
            *environment() = nullptr;
            if (!global<void *>(0x00fb1388)) {
                void **const wide = static_cast<void **>(CRT_malloc(4));
                global<void **>(0x00fb1388) = wide;
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
            import_slot<s32(__stdcall *)(const char *, const char *)>(0x006fd278)(pair, remove ? nullptr : pair + name_length + 1);
            Crt().free(pair);
        }
    }
    return 0;
}

s32 __cdecl CRT_wtomb_environ() {
    const u16 **entry = global<const u16 **>(0x00fb1388);
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
        if (!global<void *>(0x00fb1388)) return nullptr;
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
    *reinterpret_cast<char ***>((uintptr_t)0x00fb1380) = next;
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

u32 __cdecl CRT_wcstombs_lk(char *to, const u16 *from, u32 n) {
    s32 lossy = 0;
    const u32 code_page = global<u32>(0x00fb1450);
    if (to && !n) return 0;
    if (!to) {
        if (!global<u32>(0x00fb1440)) return Crt().wcslen(from);
        const s32 size = to_narrow(code_page, 0x220, from, -1, nullptr, 0, nullptr, &lossy);
        if (!size || lossy) return unconvertible();
        return static_cast<u32>(size) - 1;
    }
    if (!global<u32>(0x00fb1440)) {
        for (u32 i = 0; i < n; ++i) {
            if (from[i] > 0xff) return unconvertible();
            to[i] = static_cast<char>(from[i]);
            if (!from[i]) return i;
        }
        return n;
    }
    const s32 widest = global<s32>(0x0077e7dc);
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
    if (lossy || import_slot<u32(__stdcall *)()>(0x006fd138)() != 0x7a) return unconvertible();
    u32 done = 0;
    while (done < n) {
        char one[16];   // (sic: the original's is the argument's slot on the stack, room for 8 bytes before the return address)
        const s32 got = to_narrow(global<u32>(0x00fb1450), 0, from, 1, one, global<s32>(0x0077e7dc), nullptr, &lossy);
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
    const u32 readers = 0x00fb3ca8;
    const auto decrement = import_slot<s32(__stdcall *)(u32)>(0x006fd12c);
    import_slot<s32(__stdcall *)(u32)>(0x006fd0c0)(readers);
    const bool locked = global<u32>(0x00fb3ca4) != 0;
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
    global<u32>(0x00fb1458) = 0;
    global<u32>(0x00781480) = 0xffffffff;
    global<u32>(0x00781470) = 0xffffffff;
    const char *zone = CRT_getenv(reinterpret_cast<const char *>((uintptr_t)0x00708da0));
    char *&standard = global<char *>(0x00781468);
    char *&daylight = global<char *>(0x0078146c);
    s32 &offset = global<s32>(0x007813d8);
    if (!zone) {
        CrtInner().unlock(0xc);
        if (import_slot<s32(__stdcall *)(u32)>(0x006fd21c)(0x00fb1460) == -1) return;
        const s32 bias = global<s32>(0x00fb14b4);
        global<u32>(0x00fb1458) = 1;
        offset = global<s32>(0x00fb1460) * 60;
        if (global<u16>(0x00fb14a6)) offset += bias * 60;
        if (global<u16>(0x00fb14fa) && global<s32>(0x00fb1508)) {
            global<u32>(0x007813dc) = 1;
            global<s32>(0x007813e0) = (global<s32>(0x00fb1508) - bias) * 60;
        } else {
            global<u32>(0x007813dc) = 0;
            global<u32>(0x007813e0) = 0;
        }
        CRT_wcstombs(standard, reinterpret_cast<const u16 *>((uintptr_t)0x00fb1464), 0x40);
        CRT_wcstombs(daylight, reinterpret_cast<const u16 *>((uintptr_t)0x00fb14b8), 0x40);
        daylight[0x3f] = 0;
        standard[0x3f] = 0;
        return;
    }
    char *const last = global<char *>(0x00fb150c);
    if (!*zone || (last && !std::strcmp(zone, last))) {
        CrtInner().unlock(0xc);
        return;
    }
    Crt().free(last);
    char *const copy = static_cast<char *>(CRT_malloc(static_cast<u32>(std::strlen(zone)) + 1));
    global<char *>(0x00fb150c) = copy;
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
    global<s32>(0x007813dc) = more;
    if (more) {
        Crt().strncpy(daylight, zone, 3);
        daylight[3] = 0;
    } else {
        daylight[0] = 0;
    }
}

}
