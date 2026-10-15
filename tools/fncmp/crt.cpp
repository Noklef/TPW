// Stand-ins for the game's MSVC6 CRT ("C" locale) in tpw_fncmp, which has no game code to call:
// ports reach them through CrtEnv, and the driver also answers their original addresses with
// them, so that tests/fncmp/crt.py can check each against the original under the emulator.
// Test tooling only: nothing here is part of the reimplementation.
#define _CRT_SECURE_NO_WARNINGS
#include "crt.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

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

}  // namespace msvc6

const CrtEnv kDriverCrt = {&msvc6::stricmp, &msvc6::strrchr, &msvc6::strchr, &msvc6::strncmp, &msvc6::strstr,
                           &msvc6::toupper, &msvc6::tolower, &msvc6::atoi,   &msvc6::sprintf, &msvc6::wcslen};

const CrtStandIn kCrtStandIns[] = {
    {0x0067df40, (void *)&msvc6::stricmp}, {0x0067b5f0, (void *)&msvc6::strrchr}, {0x0067bfc0, (void *)&msvc6::strchr},
    {0x0067c250, (void *)&msvc6::strncmp}, {0x0067c080, (void *)&msvc6::strstr},  {0x0067f1a0, (void *)&msvc6::toupper},
    {0x0067fa60, (void *)&msvc6::tolower}, {0x0067a520, (void *)&msvc6::atoi},    {0x0067a650, (void *)&msvc6::sprintf},
    {0x0067acd0, (void *)&msvc6::wcslen},
};
const u32 kCrtStandInCount = sizeof kCrtStandIns / sizeof kCrtStandIns[0];
