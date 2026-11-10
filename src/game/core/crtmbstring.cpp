#include "crtmbstring.h"

#include <cstdint>

#include "crt.h"
#include "crtlocale.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kMbcsLock = 0x19;

u8 type_of(u8 c) { return (&CRT_mbctype[1])[c]; }

bool is_lead(u8 c) { return (type_of(c) & 4) != 0; }

u32 code_page() { return CRT_mbcodepage; }

}  // namespace

extern "C" {

u32 __cdecl CRT_mbctolower(u32 c) {
    if (c > 0xff) {
        u8 from[2] = {static_cast<u8>(c >> 8), static_cast<u8>(c)};
        if (!is_lead(from[0])) return c;
        u8 to[2];
        if (!CRT_LCMapStringA(CRT_mblcid, 0x100, reinterpret_cast<const char *>(from), 2, reinterpret_cast<char *>(to), 2, code_page(), 1)) return c;
        return (static_cast<u32>(to[0]) << 8) + to[1];
    }
    if ((type_of(static_cast<u8>(c)) & 0x10) != 0x10) return c;
    return CRT_mbcasemap[c];
}

u32 __cdecl CRT_getSystemCP(u32 code_page) {
    u32 &from_system = CRT_mb_system_set_fb1884;
    from_system = 0;
    if (code_page == 0xfffffffe) {
        from_system = 1;
        return (*reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd1f4))();
    }
    if (code_page == 0xfffffffd) {
        from_system = 1;
        return (*reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd17c))();
    }
    if (code_page == 0xfffffffc) {
        from_system = 1;
        return CRT_lc_codepage;
    }
    return code_page;
}

u32 __cdecl CRT_CPtoLCID(u32 code_page) {
    switch (code_page) {
    case 932: return 0x411;
    case 936: return 0x804;
    case 949: return 0x412;
    case 950: return 0x404;
    default: return 0;
    }
}

u32 __cdecl CRT_setSBCS() {
    u8 *const table = CRT_mbctype;
    for (u32 i = 0; i < 0x101; ++i) table[i] = 0;
    CRT_mbcodepage = 0;
    CRT_ismbcodepage = 0;
    CRT_mblcid = 0;
    for (u32 &info : CRT_mbulinfo) info = 0;
    return 0;
}

u32 __cdecl CRT_ismbblead(u32 c) { return CRT_ismbbtype(c, 0, 4); }

u32 __cdecl CRT_ismbbtype(u32 c, u32 ctype_mask, u32 mask) {
    const u32 byte = c & 0xff;
    if ((&CRT_mbctype[1])[byte] & static_cast<u8>(mask)) return 1;
    return ctype_mask && reinterpret_cast<const u16 *>((uintptr_t)0x0077ea9a)[byte] & ctype_mask ? 1 : 0;
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
            const s32 length = CRT_LCMapStringA(CRT_mblcid, 0x200, reinterpret_cast<const char *>(at - 1), 2, reinterpret_cast<char *>(to), 2, code_page(), 1);
            if (length == 1)
                out = to[0];
            else if (length == 2)
                out = static_cast<u16>(to[0] << 8 | to[1]);
            else
                return false;
            ++at;
            return true;
        }
        out = (type_of(c) & 0x10) == 0x10 ? CRT_mbcasemap[c] : c;
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

void __cdecl CRT_setSBUpLow() {
    struct Frame {   // (the original's frame: a lead byte list without its end reads on into the characters)
        u8 info[0x14];
        u8 chars[0x100];
        u8 lower[0x100];
        u8 upper[0x100];
        u16 types[0x100];
    } f;
    u8 *const types = &CRT_mbctype[1];
    u8 *const casemap = &CRT_mbcasemap[0];
    if (reinterpret_cast<s32(__stdcall *const *)(u32, void *)>((uintptr_t)0x006fd180)[0](code_page(), f.info) == 1) {    // GetCPInfo
        for (u32 i = 0; i < 0x100; ++i) f.chars[i] = static_cast<u8>(i);
        f.chars[0] = 0x20;
        for (const u8 *lead = f.info + 6; *lead; lead += 2)
            for (u32 c = lead[0]; c <= lead[1]; ++c) f.chars[c] = 0x20;
        CRT_GetStringTypeA(1, reinterpret_cast<const char *>(f.chars), 0x100, f.types, code_page(), CRT_mblcid, 0);
        CRT_LCMapStringA(CRT_mblcid, 0x100, reinterpret_cast<const char *>(f.chars), 0x100, reinterpret_cast<char *>(f.lower), 0x100, code_page(), 0);
        CRT_LCMapStringA(CRT_mblcid, 0x200, reinterpret_cast<const char *>(f.chars), 0x100, reinterpret_cast<char *>(f.upper), 0x100, code_page(), 0);
        for (u32 i = 0; i < 0x100; ++i) {
            if (f.types[i] & 1) {
                types[i] |= 0x10;
                casemap[i] = f.lower[i];
            } else if (f.types[i] & 2) {
                types[i] |= 0x20;
                casemap[i] = f.upper[i];
            } else {
                casemap[i] = 0;
            }
        }
        return;
    }
    for (u32 i = 0; i < 0x100; ++i) {
        if (i >= 0x41 && i <= 0x5a) {
            types[i] |= 0x10;
            casemap[i] = static_cast<u8>(i + 0x20);
        } else if (i >= 0x61 && i <= 0x7a) {
            types[i] |= 0x20;
            casemap[i] = static_cast<u8>(i - 0x20);
        } else {
            casemap[i] = 0;
        }
    }
}

s32 __cdecl CRT_setmbcp(s32 page) {
    u8 *const types = &CRT_mbctype[0];
    const auto clear = [&] {
        for (u32 i = 0; i < 0x101; ++i) types[i] = 0;
    };
    // The ranges [first, last] of a list of byte pairs (to a first or a last of 0) given bits in the table.
    const auto mark = [&](const u8 *pairs, u8 bits) {
        for (; *pairs && pairs[1]; pairs += 2)
            for (u32 c = pairs[0]; c <= pairs[1]; ++c) types[c + 1] |= bits;
    };
    CrtInner().lock(kMbcsLock);
    const u32 wanted = CRT_getSystemCP(static_cast<u32>(page));
    if (wanted == code_page()) {
        CrtInner().unlock(kMbcsLock);
        return 0;
    }
    if (!wanted) {
        CRT_setSBCS();
        CRT_setSBUpLow();
        CrtInner().unlock(kMbcsLock);
        return 0;
    }
    for (u32 i = 0, at = 0x00781590; at < 0x00781680; at += 0x30, ++i) {
        if (global<u32>(at) != wanted) continue;
        clear();
        for (u32 range = 0; range < 4; ++range) mark(&global<u8>(0x007815a0 + i * 0x30 + range * 8), global<u8>(0x00781588 + range));
        CRT_ismbcodepage = 1;
        CRT_mbcodepage = wanted;
        CRT_mblcid = CRT_CPtoLCID(wanted);
        CRT_mbulinfo[0] = global<u32>(0x00781594 + i * 0x30);
        CRT_mbulinfo[1] = global<u32>(0x00781598 + i * 0x30);
        CRT_mbulinfo[2] = global<u32>(0x0078159c + i * 0x30);
        CRT_setSBUpLow();
        CrtInner().unlock(kMbcsLock);
        return 0;
    }
    u8 info[0x14];
    if (reinterpret_cast<s32(__stdcall *const *)(u32, void *)>((uintptr_t)0x006fd180)[0](wanted, info) == 1) {    // GetCPInfo
        clear();
        CRT_mbcodepage = wanted;
        CRT_mblcid = 0;
        if (*reinterpret_cast<const u32 *>(info) > 1) {
            mark(info + 6, 4);
            for (u32 c = 1; c < 0xff; ++c) types[c + 1] |= 8;
            CRT_mblcid = CRT_CPtoLCID(wanted);
            CRT_ismbcodepage = 1;
        } else {
            CRT_ismbcodepage = 0;
        }
        CRT_mbulinfo[0] = 0;
        CRT_mbulinfo[1] = 0;
        CRT_mbulinfo[2] = 0;
    } else {
        if (!CRT_mb_system_set_fb1884) {
            CrtInner().unlock(kMbcsLock);
            return -1;
        }
        CRT_setSBCS();
    }
    CRT_setSBUpLow();
    CrtInner().unlock(kMbcsLock);
    return 0;
}

s32 __cdecl CRT_initmbctable() { return CRT_setmbcp(-3); }

}  // extern "C"
