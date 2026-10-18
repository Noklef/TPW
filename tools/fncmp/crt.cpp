// Stand-ins for the game's MSVC6 CRT ("C" locale) in tpw_fncmp, which has no game code to call:
// ports reach them through CrtEnv, and the driver also answers their original addresses with
// them, so that tests/fncmp/crt.py can check each against the original under the emulator.
// Test tooling only: nothing here is part of the reimplementation.
#define _CRT_SECURE_NO_WARNINGS
#include "crt.h"

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwctype>

namespace msvc6 {

u8 lower(u8 c) { return (u8)(c - 'A') < 26 ? c + 0x20 : c; }

s32 __cdecl stricmp(const char *a, const char *b) {
    for (;;) {
        u8 x = (u8)*a++, y = (u8)*b++;
        if (x == y) {
            if (!x) return 0;
            continue;
        }
        x = lower(x);
        y = lower(y);
        if (x != y) return x < y ? -1 : 1;
    }
}

char *__cdecl strrchr(const char *s, s32 c) {
    const char *hit = nullptr;
    do {
        if (*s == (char)c) hit = s;
    } while (*s++);
    return const_cast<char *>(hit);
}

char *__cdecl strchr(const char *s, s32 c) {
    for (;; ++s) {
        if (*s == (char)c) return const_cast<char *>(s);
        if (!*s) return nullptr;
    }
}

s32 __cdecl strncmp(const char *a, const char *b, u32 n) {
    for (; n; --n) {
        const u8 x = (u8)*a++, y = (u8)*b++;
        if (x != y) return x < y ? -1 : 1;
        if (!x) return 0;
    }
    return 0;
}

char *__cdecl strstr(const char *s, const char *find) { return const_cast<char *>(std::strstr(s, find)); }

s32 __cdecl toupper(s32 c) { return c >= 'a' && c <= 'z' ? c - 0x20 : c; }

s32 __cdecl tolower(s32 c) { return c >= 'A' && c <= 'Z' ? c + 0x20 : c; }

bool space(u8 c) { return c == ' ' || (c >= 9 && c <= 13); }

s32 __cdecl atoi(const char *s) {
    while (space((u8)*s)) ++s;
    const char sign = *s;
    if (sign == '-' || sign == '+') ++s;
    u32 total = 0;
    for (; (u8)(*s - '0') < 10; ++s) total = total * 10 + (u8)(*s - '0');  // wraps, as atol does
    return (s32)(sign == '-' ? 0 - total : total);
}

// The host's: the same for integers and strings, not necessarily for floats.
s32 __cdecl sprintf(char *to, const char *format, ...) {
    va_list args;
    va_start(args, format);
    const int n = std::vsprintf(to, format, args);
    va_end(args);
    return n;
}

u32 __cdecl wcslen(const u16 *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}

char *__cdecl strncpy(char *to, const char *from, u32 n) {
    char *p = to;
    for (; n && *from; --n) *p++ = *from++;
    for (; n; --n) *p++ = 0;
    return to;
}

char *__cdecl strncat(char *to, const char *from, u32 n) {
    char *p = to;
    while (*p) ++p;
    for (; n && *from; --n) *p++ = *from++;
    *p = 0;
    return to;
}

// The host's (its classification of the C locale; MSVC6 asks the same tables for c < 0x100 and Windows for the rest).
u32 __cdecl iswctype(u32 c, u32 mask) { return (u32)::iswctype((wint_t)c, (wctype_t)mask); }

// (The host's classification: checked against the game's for c below 0x80; above that MSVC6 asks its Latin-1 table, then Windows.)
u32 __cdecl iswspace(u32 c) { return iswctype(c & 0xffff, 8); }

// The host's: the legacy _vsnprintf returns -1 and leaves the buffer unterminated when it does not fit, as MSVC6's does.
s32 __cdecl vsprintf(char *to, const char *format, void *args) { return std::vsprintf(to, format, (va_list)args); }
s32 __cdecl vsnprintf(char *to, u32 size, const char *format, void *args) { return _vsnprintf(to, size, format, (va_list)args); }
s32 __cdecl vswprintf(u16 *to, const u16 *format, void *args) {
    return _vsnwprintf((wchar_t *)to, 0x7fffffff, (const wchar_t *)format, (va_list)args);
}

// The host's (%s a wide string, as MSVC's): checked for strings and characters; MSVC6's integers are unchecked (they go through code the
// emulator does not run) and its %c of 0xffff (WEOF) fails with -1, which the host's does not.
s32 __cdecl swprintf(u16 *to, const u16 *format, ...) {
    va_list args;
    va_start(args, format);
    const s32 n = _vsnwprintf((wchar_t *)to, 0x7fffffff, (const wchar_t *)format, args);
    va_end(args);
    return n;
}

u16 *__cdecl wcscpy(u16 *to, const u16 *from) {
    u16 *p = to;
    while ((*p++ = *from++) != 0) {
    }
    return to;
}

u16 *__cdecl wcsncpy(u16 *to, const u16 *from, u32 n) {
    u16 *p = to;
    for (; n && *from; --n) *p++ = *from++;
    for (; n; --n) *p++ = 0;
    return to;
}

u16 *__cdecl wcscat(u16 *to, const u16 *from) {
    u16 *p = to;
    while (*p) ++p;
    while ((*p++ = *from++) != 0) {
    }
    return to;
}

u16 *__cdecl wcsncat(u16 *to, const u16 *from, u32 n) {
    u16 *p = to;
    while (*p) ++p;
    for (; n && *from; --n) *p++ = *from++;
    *p = 0;
    return to;
}

// ASCII digits only (unconfirmed: MSVC6's _wtol also takes other scripts' digits; a bench of Lib_WtoI with such text would show it).
s32 __cdecl wtoi(const u16 *s) {
    while (*s < 0x100 && space((u8)*s)) ++s;
    const u16 sign = *s;
    if (sign == '-' || sign == '+') ++s;
    u32 total = 0;
    for (; *s >= '0' && *s <= '9'; ++s) total = total * 10 + (*s - '0');
    return (s32)(sign == '-' ? 0 - total : total);
}

// The game's own table (the driver has the game's data at its addresses), read as MSVC6 reads it with __mb_cur_max 1:
// c indexes the words of _pctype as given, so a sign-extended byte from 0x80 reads the data before the table (sic).
s32 ctype(s32 c, u32 mask) { return (*reinterpret_cast<const u16 *const *>((uintptr_t)0x77ea90))[c] & mask; }
s32 __cdecl isalpha(s32 c) { return ctype(c, 0x103); }
s32 __cdecl isdigit(s32 c) { return ctype(c, 4); }
s32 __cdecl isspace(s32 c) { return ctype(c, 8); }
s32 __cdecl isalnum(s32 c) { return ctype(c, 0x107); }

// MSVC6's grammar (__strgtold12: up to 25 significant digits, d or e for the exponent) rewritten for the host's strtod,
// which rounds correctly: checked against the game's for short mantissas and moderate exponents (MSVC6 multiplies
// through a 12-byte float and may differ in the last bit beyond that).
double __cdecl atof(const char *s) {
    while (space((u8)*s)) ++s;
    char text[48];  // sign, 25 digits, "e", sign, 5 digits
    u32 n = 0;
    if (*s == '+' || *s == '-') text[n++] = *s++;
    auto digit = [](char c) { return c >= '0' && c <= '9'; };
    u32 significant = 0;
    s32 exponent = 0;  // of the last digit kept
    bool any = false;
    for (; digit(*s); ++s) {
        any = true;
        if (significant == 0 && *s == '0') continue;
        if (significant < 25) {
            text[n++] = *s;
            ++significant;
        } else {
            ++exponent;
        }
    }
    if (*s == '.') {
        const bool fraction = digit(s[1]);
        if (any || fraction) ++s;
        for (; digit(*s); ++s) {
            any = true;
            if (significant == 0 && *s == '0') {
                --exponent;
                continue;
            }
            if (significant < 25) {
                text[n++] = *s;
                ++significant;
                --exponent;
            }
        }
    }
    if (!any) return 0.0;
    if (significant == 0) text[n++] = '0';
    s32 power = 0;
    const char *e = s;
    if (*e == 'e' || *e == 'E' || *e == 'd' || *e == 'D') {
        ++e;
        bool negative = false;
        if (*e == '+' || *e == '-') negative = *e++ == '-';
        if (digit(*e)) {
            for (; digit(*e); ++e) {
                if (power < 5200) power = power * 10 + (*e - '0');
            }
            if (power > 5200) power = 5200;
            if (negative) power = -power;
        }
    }
    std::snprintf(text + n, sizeof text - n, "e%d", (int)(power + exponent));
    return std::strtod(text, nullptr);
}

}  // namespace msvc6

const CrtEnv kDriverCrt = {&msvc6::stricmp, &msvc6::strrchr, &msvc6::strchr, &msvc6::strncmp, &msvc6::strstr,
                           &msvc6::toupper, &msvc6::tolower, &msvc6::atoi,   &msvc6::sprintf, &msvc6::wcslen,
                           &msvc6::strncpy, &msvc6::iswspace, &msvc6::isalpha, &msvc6::isdigit, &msvc6::isspace,
                           &msvc6::isalnum, &msvc6::atof, &msvc6::swprintf, &msvc6::wcsncpy};

const LibcEnv kDriverLibc = {&msvc6::strncpy, &msvc6::strncat, &msvc6::atoi,    &msvc6::iswctype, &msvc6::vsprintf,
                             &msvc6::vsnprintf, &msvc6::wcslen,  &msvc6::wcscpy,  &msvc6::wcsncpy,  &msvc6::wcscat,
                             &msvc6::wcsncat, &msvc6::wtoi,     &msvc6::vswprintf};

const CrtStandIn kCrtStandIns[] = {
    {0x0067df40, (void *)&msvc6::stricmp}, {0x0067b5f0, (void *)&msvc6::strrchr}, {0x0067bfc0, (void *)&msvc6::strchr},
    {0x0067c250, (void *)&msvc6::strncmp}, {0x0067c080, (void *)&msvc6::strstr},  {0x0067f1a0, (void *)&msvc6::toupper},
    {0x0067fa60, (void *)&msvc6::tolower}, {0x0067a520, (void *)&msvc6::atoi},    {0x0067a650, (void *)&msvc6::sprintf},
    {0x0067acd0, (void *)&msvc6::wcslen},  {0x0067ae00, (void *)&msvc6::strncpy}, {0x0067a730, (void *)&msvc6::iswspace},
    {0x0067a530, (void *)&msvc6::isalpha}, {0x0067a560, (void *)&msvc6::isdigit}, {0x0067a590, (void *)&msvc6::isspace},
    {0x0067a5c0, (void *)&msvc6::isalnum}, {0x0067a6c0, (void *)&msvc6::atof},    {0x0067af00, (void *)&msvc6::swprintf},
    {0x0067b0d0, (void *)&msvc6::wcsncpy},
};
const u32 kCrtStandInCount = sizeof kCrtStandIns / sizeof kCrtStandIns[0];
