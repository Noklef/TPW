#include "crtlocale.h"

#include <cstdarg>
#include <cstdint>
#include <cstring>

#include "crt.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

template <typename F> F import_slot(u32 slot) { return *reinterpret_cast<F *>((uintptr_t)slot); }

using LocaleInfo = s32(__stdcall *)(u32 locale, u32 what, char *out, s32 room);

// __lc_category: six entries of 12 bytes: the name, the locale string, the init routine.
const char *category_name(s32 category) { return global<const char *>(0x00780dd0 + category * 12); }
char *&category_locale(s32 category) { return global<char *>(0x00780dd4 + category * 12); }
s32 category_init(s32 category) { return global<s32(__cdecl *)()>(0x00780dd8 + category * 12)(); }
u32 &category_id(s32 category) { return global<u32>(0x00fb1438 + category * 4); }

char *const c_locale = reinterpret_cast<char *>((uintptr_t)0x00780dc8);
char *const cache_name = reinterpret_cast<char *>((uintptr_t)0x00780ea0);
char *const cache_text = reinterpret_cast<char *>((uintptr_t)0x00780e18);

const char *&language() { return global<const char *>(0x00fb18ec); }
const char *&country() { return global<const char *>(0x00fb18f0); }
u32 &lcid_state() { return global<u32>(0x00fb18f4); }
LocaleInfo &locale_info() { return global<LocaleInfo>(0x00fb18fc); }

void enum_locales(u32 callback) { import_slot<s32(__stdcall *)(u32, u32)>(0x006fd1f8)(callback, 1); }

// "LC_x=locale;LC_y=locale": each category named; other names skipped. The value's room is not checked (sic).
char *set_pairs(const char *text) {
    s32 set = 0;
    for (const char *at = CRT_strpbrk(text, "=;"); at;) {
        const u32 name_length = static_cast<u32>(at - text);
        if (!name_length || *at == ';') return nullptr;
        s32 which = 1;
        for (; which <= 5; ++which) {
            const char *const name = category_name(which);
            if (!CRT_strncmp(name, text, name_length) && name_length == std::strlen(name)) break;
        }
        const char *const value = at + 1;
        const u32 length = CRT_strcspn(value, ";");
        if (!length && *value != ';') return nullptr;
        if (which <= 5) {
            char one[0x84];
            Crt().strncpy(one, value, length);
            one[length] = 0;
            if (CRT_setlocale_set_cat(which, one)) ++set;
        }
        const char *const stop = value + length;
        if (!*stop || !stop[1]) return set ? CRT_setlocale_get_all() : nullptr;
        text = stop + 1;
        at = CRT_strpbrk(text, "=;");
    }
    return nullptr;
}

// One locale for all the categories.
char *set_every(const char *text) {
    char name[0x84];
    if (!CRT_expandlocale(text, name, nullptr, nullptr, 0)) return nullptr;
    bool all_good = true;
    s32 set = 0;
    for (s32 which = 1; which <= 5; ++which) {
        if (!std::strcmp(name, category_locale(which))) {
            ++set;
        } else if (CRT_setlocale_set_cat(which, name)) {
            ++set;
        } else {
            all_good = false;
        }
    }
    if (all_good) {
        char *const all = CRT_setlocale_get_all();
        Crt().free(category_locale(0));   // (the answer may be this block: sic)
        category_locale(0) = nullptr;
        return all;
    }
    return set ? CRT_setlocale_get_all() : nullptr;
}

char *setlocale_locked(s32 category, const char *locale) {
    if (category) return locale ? CRT_setlocale_set_cat(category, locale) : category_locale(category);
    if (!locale) return CRT_setlocale_get_all();
    if (locale[0] == 'L' && locale[1] == 'C' && locale[2] == '_') return set_pairs(locale);
    return set_every(locale);
}

}  // namespace

s32 __cdecl CRT_strncmp(const char *a, const char *b, u32 n) {
    if (!n) return 0;
    u32 count = 0;
    while (count < n && a[count++]) {
    }
    for (u32 i = 0; i < count; ++i) {
        const u8 x = static_cast<u8>(a[i]);
        const u8 y = static_cast<u8>(b[i]);
        if (x != y) return y > x ? -1 : 1;
    }
    return 0;
}

u32 __cdecl CRT_strcspn(const char *text, const char *set) {
    bool in_set[0x100] = {};
    for (; *set; ++set) in_set[static_cast<u8>(*set)] = true;
    u32 count = 0;
    for (; *text && !in_set[static_cast<u8>(*text)]; ++text) ++count;
    return count;
}

void __cdecl CRT_strcats(char *out, s32 n, ...) {
    va_list args;
    va_start(args, n);
    for (; n > 0; --n) std::strcat(out, va_arg(args, const char *));
    va_end(args);
}

s32 __cdecl CRT_lc_strtolc(char *names, const char *locale) {
    std::memset(names, 0, 0x88);
    if (!*locale) return 0;
    if (*locale == '.' && locale[1]) {
        std::strcpy(names + 0x80, locale + 1);
        return 0;
    }
    for (s32 part = 0;; ++part) {
        const u32 length = CRT_strcspn(locale, "_.,");
        if (!length) return -1;
        const char stop = locale[length];
        if (part == 0 && length < 0x40 && stop != '.') {
            Crt().strncpy(names, locale, length);
        } else if (part == 1 && length < 0x40 && stop != '_') {
            Crt().strncpy(names + 0x40, locale, length);
        } else if (part == 2 && (!stop || stop == ',')) {
            Crt().strncpy(names + 0x80, locale, length);   // (the room is not checked: sic)
        } else {
            return -1;
        }
        if (stop == ',' || !stop) return 0;
        locale += length + 1;
    }
}

void __cdecl CRT_lc_lctostr(char *out, const char *names) {
    std::strcpy(out, names);
    if (names[0x40]) CRT_strcats(out, 2, "_", names + 0x40);
    if (names[0x80]) CRT_strcats(out, 2, ".", names + 0x80);
}

s32 __cdecl CRT_GetPrimaryLen(const char *text) {
    s32 count = 0;
    for (;; ++count) {
        const s8 c = static_cast<s8>(*text++);
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) return count;
    }
}

s32 __cdecl CRT_IsThisWindowsNT() {
    u32 info[0x25];
    info[0] = 0x94;
    if (!import_slot<s32(__stdcall *)(u32 *)>(0x006fd264)(info)) return 0;
    return info[4] == 2 ? 1 : 0;
}

s32 __cdecl CRT_TranslateName(void *table, s32 high, const char **name) {
    u8 *const entries = static_cast<u8 *>(table);
    s32 low = 0;
    s32 compared = 1;
    if (high < 0) return compared;
    do {
        const s32 middle = (low + high) / 2;
        compared = CRT_stricmp(*name, *reinterpret_cast<const char **>(entries + middle * 8));
        if (!compared) {
            *name = reinterpret_cast<const char *>(entries + middle * 8 + 4);
            return compared;
        }
        if (compared < 0) {
            high = middle - 1;
        } else {
            low = middle + 1;
        }
    } while (low <= high);
    return compared;
}

void __cdecl CRT_GetLcidFromLangCountry() {
    const char *const lang = language();
    const u32 abbreviated = std::strlen(lang) == 3 ? 1 : 0;
    global<u32>(0x00fb18e8) = abbreviated;
    global<u32>(0x00fb18e0) = std::strlen(country()) == 3 ? 1 : 0;
    global<u32>(0x00fb18dc) = 0;
    global<s32>(0x00fb18e4) = abbreviated ? 2 : CRT_GetPrimaryLen(lang);
    enum_locales(0x0068de40);
    const u32 state = lcid_state();
    if (!(state & 0x100) || !(state & 0x200) || !(state & 7)) lcid_state() = 0;
}

void __cdecl CRT_GetLcidFromLanguage() {
    const char *const lang = language();
    const u32 abbreviated = std::strlen(lang) == 3 ? 1 : 0;
    global<u32>(0x00fb18e8) = abbreviated;
    global<s32>(0x00fb18e4) = abbreviated ? 2 : CRT_GetPrimaryLen(lang);
    enum_locales(0x0068e180);
    if (!(lcid_state() & 4)) lcid_state() = 0;
}

void __cdecl CRT_GetLcidFromCountry() {
    global<u32>(0x00fb18e0) = std::strlen(country()) == 3 ? 1 : 0;
    enum_locales(0x0068e2a0);
    if (!(lcid_state() & 4)) lcid_state() = 0;
}

void __cdecl CRT_GetLcidFromDefault() {
    lcid_state() |= 0x104;
    const u32 locale = import_slot<u32(__stdcall *)()>(0x006fd260)();
    global<u32>(0x00fb18f8) = locale;
    global<u32>(0x00fb18dc) = locale;
}

u32 __cdecl CRT_ProcessCodePage(const char *name) {
    char own[8];
    if (!name || !*name || !std::strcmp(name, "ACP")) {
        if (!locale_info()(global<u32>(0x00fb18f8), 0x1004, own, 8)) return 0;
        name = own;
    } else if (!std::strcmp(name, "OCP")) {
        if (!locale_info()(global<u32>(0x00fb18f8), 0xb, own, 8)) return 0;
        name = own;
    }
    return static_cast<u32>(CRT_atol(name));
}

s32 __cdecl CRT_get_qualified_locale(const char *names, u16 *id, char *out) {
    if (!locale_info()) {
        locale_info() = CRT_IsThisWindowsNT() ? import_slot<LocaleInfo>(0x006fd28c) : reinterpret_cast<LocaleInfo>((uintptr_t)0x0068e550);
    }
    if (names) {
        language() = names;
        if (*names) CRT_TranslateName(reinterpret_cast<void *>((uintptr_t)0x00781760), 0x40, &language());
        country() = names + 0x40;
        if (*country()) CRT_TranslateName(reinterpret_cast<void *>((uintptr_t)0x00781968), 0x16, &country());
        lcid_state() = 0;
        if (language() && *language()) {
            if (country() && *country()) {
                CRT_GetLcidFromLangCountry();
            } else {
                CRT_GetLcidFromLanguage();
            }
        } else if (country() && *country()) {
            CRT_GetLcidFromCountry();
        } else {
            CRT_GetLcidFromDefault();
        }
    } else {
        CRT_GetLcidFromDefault();
    }
    if (!lcid_state()) return 0;
    const u32 code_page = CRT_ProcessCodePage(names + 0x80);
    if (!code_page) return 0;
    if (!import_slot<s32(__stdcall *)(u32)>(0x006fd288)(code_page & 0xffff)) return 0;
    if (!import_slot<s32(__stdcall *)(u32, u32)>(0x006fd284)(global<u32>(0x00fb18dc), 1)) return 0;
    if (id) {
        id[0] = global<u16>(0x00fb18dc);
        id[1] = global<u16>(0x00fb18f8);
        id[2] = static_cast<u16>(code_page);
    }
    if (out) {
        if (!locale_info()(global<u32>(0x00fb18dc), 0x1001, out, 0x40)) return 0;
        if (!locale_info()(global<u32>(0x00fb18f8), 0x1002, out + 0x40, 0x40)) return 0;
        CRT_itoa(static_cast<s32>(code_page), out + 0x80, 10);
    }
    return 1;
}

char *__cdecl CRT_expandlocale(const char *text, char *out, u16 *id, u32 *code_page, s32) {
    if (!text) return nullptr;
    if (text[0] == 'C' && !text[1]) {
        out[0] = 'C';
        out[1] = 0;
        if (id) {
            id[0] = 0;
            id[1] = 0;
            id[2] = 0;
        }
        if (code_page) *code_page = 0;
        return out;
    }
    if (std::strcmp(cache_name, text) && std::strcmp(cache_text, text)) {
        char names[0x88];
        if (CRT_lc_strtolc(names, text)) return nullptr;
        if (!CRT_get_qualified_locale(names, &global<u16>(0x00fb142c), names)) return nullptr;
        global<u32>(0x00fb1434) = global<u16>(0x00fb1430);
        CRT_lc_lctostr(cache_name, names);
        std::strcpy(cache_text, *text ? text : cache_name);
    }
    if (id) {
        id[0] = global<u16>(0x00fb142c);
        id[1] = global<u16>(0x00fb142e);
        id[2] = global<u16>(0x00fb1430);
    }
    if (code_page) *code_page = global<u32>(0x00fb1434);
    std::strcpy(out, cache_name);
    return cache_name;
}

char *__cdecl CRT_setlocale_get_all() {
    if (!category_locale(0)) category_locale(0) = static_cast<char *>(CRT_malloc(0x351));
    bool same = true;
    *category_locale(0) = 0;
    CRT_strcats(category_locale(0), 3, category_name(1), "=", category_locale(1));
    for (s32 which = 1; which < 5; ++which) {
        std::strcat(category_locale(0), ";");
        if (std::strcmp(category_locale(which), category_locale(which + 1))) same = false;
        CRT_strcats(category_locale(0), 3, category_name(which + 1), "=", category_locale(which + 1));
    }
    if (!same) return category_locale(0);
    Crt().free(category_locale(0));
    category_locale(0) = nullptr;
    return category_locale(2);
}

char *__cdecl CRT_setlocale_set_cat(s32 category, const char *locale) {
    char name[0x84];
    u16 id[3];
    u32 code_page;
    if (!CRT_expandlocale(locale, name, id, &code_page, category)) return nullptr;
    char *const copy = static_cast<char *>(CRT_malloc(static_cast<u32>(std::strlen(name)) + 1));
    if (!copy) return nullptr;
    const u32 old_id = category_id(category);
    char *const old = category_locale(category);
    const u32 old_page = global<u32>(0x00fb1450);
    std::strcpy(copy, name);
    category_locale(category) = copy;
    category_id(category) = id[0];
    std::memcpy(reinterpret_cast<void *>((uintptr_t)(0x00fb18b8 + category * 6)), id, 6);
    if (category == 2) global<u32>(0x00fb1450) = code_page;
    if (category_init(category)) {
        category_locale(category) = old;
        Crt().free(copy);
        category_id(category) = old_id;
        global<u32>(0x00fb1450) = old_page;
        return nullptr;
    }
    if (old != c_locale) Crt().free(old);
    return category_locale(category);
}

char *__cdecl CRT_setlocale(s32 category, const char *locale) {
    if (category < 0 || category > 5) return nullptr;
    CrtInner().lock(0x13);
    ++global<s32>(0x00fb3ca4);
    while (global<u32>(0x00fb3ca8)) import_slot<void(__stdcall *)(u32)>(0x006fd1d4)(1);
    char *const result = setlocale_locked(category, locale);
    CrtInner().unlock(0x13);
    --global<s32>(0x00fb3ca4);
    return result;
}
