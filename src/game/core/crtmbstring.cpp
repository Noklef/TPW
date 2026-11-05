#include "crtmbstring.h"

#include <cstdint>

#include "crt.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kMbcsLock = 0x19;

u8 type_of(u8 c) { return (&global<u8>(0x00fb1669))[c]; }

bool is_lead(u8 c) { return (type_of(c) & 4) != 0; }

u32 code_page() { return global<u32>(0x00fb1870); }

}  // namespace

extern "C" {

u32 __cdecl CRT_mbctolower(u32 c) {
    if (c > 0xff) {
        u8 from[2] = {static_cast<u8>(c >> 8), static_cast<u8>(c)};
        if (!is_lead(from[0])) return c;
        u8 to[2];
        if (!CRT_LCMapStringA(global<u32>(0x00fb1874), 0x100, reinterpret_cast<const char *>(from), 2, reinterpret_cast<char *>(to), 2, code_page(), 1)) return c;
        return (static_cast<u32>(to[0]) << 8) + to[1];
    }
    if ((type_of(static_cast<u8>(c)) & 0x10) != 0x10) return c;
    return (&global<u8>(0x00fb1770))[c];
}

s32 __cdecl CRT_mbsicmp(const char *a, const char *b) {
    if (!code_page()) return CRT_stricmp(a, b);
    // One character, lower; false for a map of the wrong length.
    const auto next = [](const u8 *&at, u16 &out) {
        const u8 c = *at++;
        if (is_lead(c)) {
            if (!*at) {
                out = 0;
                return true;
            }
            u8 to[4];
            const s32 length = CRT_LCMapStringA(global<u32>(0x00fb1874), 0x200, reinterpret_cast<const char *>(at - 1), 2, reinterpret_cast<char *>(to), 2, code_page(), 1);
            if (length == 1)
                out = to[0];
            else if (length == 2)
                out = static_cast<u16>(to[0] << 8 | to[1]);
            else
                return false;
            ++at;
            return true;
        }
        out = (type_of(c) & 0x10) == 0x10 ? (&global<u8>(0x00fb1770))[c] : c;
        return true;
    };
    CrtInner().lock(kMbcsLock);
    const u8 *x = reinterpret_cast<const u8 *>(a);
    const u8 *y = reinterpret_cast<const u8 *>(b);
    s32 result;
    for (;;) {
        u16 first, second;
        if (!next(x, first) || !next(y, second)) {
            result = 0x7fffffff;
            break;
        }
        if (first != second) {
            result = second < first ? 1 : -1;
            break;
        }
        if (!first) {
            result = 0;
            break;
        }
    }
    CrtInner().unlock(kMbcsLock);
    return result;
}

char *__cdecl CRT_mbspbrk(const char *text, const char *set) {
    if (!code_page()) return CRT_strpbrk(text, set);
    CrtInner().lock(kMbcsLock);
    const u8 *at = reinterpret_cast<const u8 *>(text);
    const u8 *const in = reinterpret_cast<const u8 *>(set);
    if (*at) {
        for (;;) {
            const u8 *s = in;
            if (*in) {
                for (;;) {
                    const u8 c = *s;
                    if (is_lead(c)) {
                        if (c == at[0] && s[1] == at[1]) break;
                        if (!s[1]) break;
                        ++s;
                    } else if (c == at[0]) {
                        break;
                    }
                    ++s;
                    if (!*s) break;
                }
            }
            if (*s) break;
            if (is_lead(*at)) {
                ++at;
                if (!*at) break;
            }
            ++at;
            if (!*at) break;
        }
    }
    CrtInner().unlock(kMbcsLock);
    return *at ? const_cast<char *>(reinterpret_cast<const char *>(at)) : nullptr;
}

char *__cdecl CRT_mbsrchr(const char *text, u32 c) {
    if (!code_page()) return CRT_strrchr(text, static_cast<s32>(c));
    const u8 *found = nullptr;
    CrtInner().lock(kMbcsLock);
    const u8 *at = reinterpret_cast<const u8 *>(text);
    u8 byte;
    do {
        byte = *at;
        const u32 first = byte;
        if (is_lead(byte)) {
            byte = at[1];
            ++at;
            if (byte) {
                if (c == ((first << 8) | byte)) found = at - 1;
            } else if (!found) {
                found = at;
            }
        } else if (c == first) {
            found = at;
        }
        ++at;
    } while (byte);
    CrtInner().unlock(kMbcsLock);
    return const_cast<char *>(reinterpret_cast<const char *>(found));
}

}  // extern "C"
