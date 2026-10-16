// The game's string classes: reimplementation of 0x5f5fc0-0x5f6c50, 0x5f7120/0x5f7150 and
// 0x5f78e0-0x5f7e60.
#include "string.h"

#include "gameheap.h"

#include <cstdint>
#include <cstring>
#include <type_traits>

namespace {

const void *const kStringVtable = reinterpret_cast<const void *>(0x00703090);
const void *const kWStringVtable = reinterpret_cast<const void *>(0x007030c8);
const void *const kViewVtable = reinterpret_cast<const void *>(0x007030c0);
const void *const kWViewVtable = reinterpret_cast<const void *>(0x007030f8);

// The game's operator new / delete (0x67acc0 / 0x67acb0), through core/gameheap.
void *__cdecl game_alloc(u32 size) { return GameHeap_Alloc(size); }
void __cdecl game_free(void *block) { GameHeap_Free(block); }

const StringEnv kGameEnv = {
    &game_alloc,
    &game_free,
    reinterpret_cast<s32(__cdecl *)(char *, const char *, va_list)>(0x0067c1e0),
    reinterpret_cast<s32(__cdecl *)(u16 *, const u16 *, va_list)>(0x0067f4d0),
};
const StringEnv *g_env = &kGameEnv;

// The C library as the game's CRT has it, for chars (T = char) and UTF-16 units (u16).
// Characters compare unsigned. The copies here are the wide ones (0x67bf10, 0x67bed0,
// 0x67b0d0, 0x67f450), a unit at a time; the char copies are below.
template <typename T> u32 str_len(const T *s) {
    u32 n = 0;
    while (s[n]) ++n;
    return n;
}
template <typename T> void str_cpy(T *to, const T *from) {
    while ((*to++ = *from++) != 0) {}
}
template <typename T> void str_cat(T *to, const T *from) { str_cpy(to + str_len(to), from); }
// strncpy: count units, NUL-padded after the end of from.
template <typename T> void str_ncpy(T *to, const T *from, u32 count) {
    for (; count && *from; --count) *to++ = *from++;
    for (; count; --count) *to++ = 0;
}
// strncat: at most count units of from, then a NUL.
template <typename T> void str_ncat(T *to, const T *from, u32 count) {
    to += str_len(to);
    for (; count && *from; --count) *to++ = *from++;
    *to = 0;
}
// The char forms copy as the game's do, which matters when they overlap (a string appended
// to itself): a dword at a time, each read whole before it is written. strcpy and strcat
// are inline (0x5f76e0, 0x5f7730): the length of from (with its NUL) first, then that many
// chars forwards, dwords and then the last 1-3 chars.
void copy_dwords(char *to, const char *from, u32 count) {
    for (u32 i = 0; i < count / 4 * 4; i += 4) {
        char d[4];
        std::memcpy(d, from + i, 4);
        std::memcpy(to + i, d, 4);
    }
    for (u32 i = count / 4 * 4; i < count; ++i) to[i] = from[i];
}
void str_cpy(char *to, const char *from) { copy_dwords(to, from, str_len(from) + 1); }
void str_cat(char *to, const char *from) {
    const u32 n = str_len(from) + 1;
    copy_dwords(to + str_len(to), from, n);
}
// strncpy and strncat (0x67ae00, 0x67b8c0) copy from up to its NUL or count chars: a char at
// a time until from is dword aligned, then dwords while 4 or more are left, then chars. The
// index of the NUL copied, or count if there was none.
u32 copy_until_nul(char *to, const char *from, u32 count) {
    u32 n = 0;
    for (; n < count && (reinterpret_cast<uintptr_t>(from + n) & 3); ++n)
        if (!(to[n] = from[n])) return n;
    for (; count - n >= 4; n += 4) {
        char d[4];
        std::memcpy(d, from + n, 4);
        for (u32 i = 0; i < 4; ++i)
            if (!(to[n + i] = d[i])) return n + i;
    }
    for (; n < count; ++n)
        if (!(to[n] = from[n])) return n;
    return count;
}
void str_ncpy(char *to, const char *from, u32 count) {
    for (u32 n = copy_until_nul(to, from, count); n < count; ++n) to[n] = 0;
}
void str_ncat(char *to, const char *from, u32 count) {
    to += str_len(to);
    if (copy_until_nul(to, from, count) == count) to[count] = 0;
}
template <typename T> s32 str_ncmp(const T *a, const T *b, u32 count) {
    using U = typename std::conditional<sizeof(T) == 1, u8, u16>::type;
    for (; count; --count, ++a, ++b) {
        if (*a != *b) return (U)*a < (U)*b ? -1 : 1;
        if (!*a) break;
    }
    return 0;
}
const char *str_str(const char *hay, const char *needle) {
    const u32 n = str_len(needle);
    for (; *hay; ++hay)
        if (!str_ncmp(hay, needle, n)) return hay;
    return n ? nullptr : hay;
}
// Format's digit test: _isctype(c, _DIGIT), which past 0xff (a sign-extended char, or a
// UTF-16 unit) classifies the low byte (no lead bytes in the game's code page).
bool is_digit(u32 c) { return (c & 0xff) >= '0' && (c & 0xff) <= '9'; }
// atoi / _wtoi: spaces, a sign, then digits; wraps on overflow. (_wtoi converts to the
// code page first, so non-ASCII digits are not digits.)
template <typename T> u32 to_int(const T *s) {
    using U = typename std::conditional<sizeof(T) == 1, u8, u16>::type;
    auto c = [&] { return (u32)(U)*s; };
    while (c() == ' ' || (c() >= 9 && c() <= 13)) ++s;
    const u32 sign = c();
    if (sign == '-' || sign == '+') ++s;
    u32 total = 0;
    for (; c() >= '0' && c() <= '9'; ++s) total = total * 10 + (c() - '0');
    return sign == '-' ? 0u - total : total;
}

u32 capacity_for(u32 length) { return (length & ~7u) + 8; }

// Format's length estimate (MFC's CString::FormatV). The wide form reads its format as
// unsigned units and takes plain %s as wide; the narrow one sign-extends and takes %S as
// narrow too. Arguments are walked as stack words.
template <typename T, bool Wide> u32 format_estimate(const T *p, const u32 *arg) {
    using U = typename std::conditional<Wide, u16, s8>::type;
    auto unit = [](T c) { return (u32)(s32)(U)c; };
    u32 total = 0;
    for (; *p; ++p) {
        if (*p != '%' || *++p == '%') {
            ++total;
            continue;
        }
        u32 item = 0, width = 0;
        for (; *p; ++p) {
            if (*p == '#') total += 2;
            else if (*p == '*') width = *arg++;
            else if (*p != '-' && *p != '+' && *p != '0' && *p != ' ') break;
        }
        if (width == 0) {
            width = to_int(p);
            while (*p && is_digit(unit(*p))) ++p;
        }
        u32 precision = 0;
        if (*p == '.') {
            if (*++p == '*') {
                precision = *arg++;
                ++p;
            } else {
                precision = to_int(p);
                while (*p && is_digit(unit(*p))) ++p;
            }
        }
        u32 modifier = 0;
        switch (unit(*p)) {
        case 'F': case 'L': case 'N': ++p; break;
        case 'h': modifier = 0x10000; ++p; break;
        case 'l': modifier = 0x20000; ++p; break;
        default: break;
        }
        const u32 type = modifier | unit(*p);
        auto text = [&](u32 length) { item = length == 0 ? 1 : length; };
        switch (type) {
        case 'c': case 'C': case 0x10000 | 'c': case 0x10000 | 'C': case 0x20000 | 'c': case 0x20000 | 'C':
            item = 2;
            ++arg;
            break;
        case 's': case 'S': case 0x10000 | 's': case 0x10000 | 'S': case 0x20000 | 's': case 0x20000 | 'S': {
            const u32 a = *arg++;
            const bool wide = (type & 0x20000) || (Wide && !(type & 0x10000));
            if (!a) item = 6;  // "(null)"
            else if (wide) text(str_len(reinterpret_cast<const u16 *>(a)));
            else text(str_len(reinterpret_cast<const char *>(a)));
            break;
        }
        default: break;
        }
        if (item) {
            if (item < width) item = width;
            total += precision && precision <= item ? precision : item;
            continue;
        }
        switch (unit(*p)) {
        case 'X': case 'd': case 'i': case 'o': case 'u': case 'x': case 'p':
            ++arg;
            total += precision + width < 0x20 ? 0x20 : precision + width;
            break;
        case 'G': case 'e': case 'f': case 'g':
            arg += 2;
            total += precision + width < 0x80 ? 0x80 : precision + width;
            break;
        case 'n': ++arg; break;
        default: break;
        }
    }
    return total;
}

}  // namespace

void Strings_SetEnv(const StringEnv *e) { g_env = e ? e : &kGameEnv; }

// --- String ---

extern "C" void __fastcall String_Alloc(String *s, void *, u32 length) {
    s->length = length;
    s->grow = 8;
    s->capacity = capacity_for(length);
    s->data = static_cast<char *>(g_env->alloc(s->capacity + 1));
    s->data[s->length] = 0;
}

extern "C" void __fastcall String_SetLength(String *s, void *, u32 length) {
    if (length <= s->capacity) {
        s->length = length;
        s->data[length] = 0;
        return;
    }
    char *const old = s->data;
    String_Alloc(s, nullptr, length);
    str_cpy(s->data, old);
    g_env->free(old);
}

extern "C" String *__fastcall String_Ctor(String *s) {
    s->data = nullptr;
    s->vtable = kStringVtable;
    String_Alloc(s, nullptr, 0);
    return s;
}

extern "C" String *__fastcall String_CtorStr(String *s, void *, const char *text) {
    s->data = nullptr;
    s->length = 0;
    s->vtable = kStringVtable;
    String_Alloc(s, nullptr, text ? str_len(text) : 0);
    if (text) str_cpy(s->data, text);
    return s;
}

extern "C" String *__fastcall String_CtorCopy(String *s, void *, const StringView *from) {
    s->data = nullptr;
    s->length = 0;
    s->vtable = kStringVtable;
    String_Alloc(s, nullptr, from->length);
    str_ncpy(s->data, from->data, from->length);
    s->data[from->length] = 0;
    return s;
}

extern "C" String *__fastcall String_CtorChar(String *s, void *, const char *c) {
    s->data = nullptr;
    s->vtable = kStringVtable;
    String_Alloc(s, nullptr, 1);
    s->data[0] = *c;
    return s;
}

extern "C" void __fastcall String_Dtor(String *s) {
    s->vtable = kStringVtable;
    g_env->free(s->data);
}

extern "C" String *__fastcall String_DeletingDtor(String *s, void *, u32 flags) {
    String_Dtor(s);
    if (flags & 1) g_env->free(s);
    return s;
}

extern "C" String *__fastcall String_AssignChar(String *s, void *, const char *c) {
    if (!*c) {
        String_SetLength(s, nullptr, 0);
        return s;
    }
    String_SetLength(s, nullptr, 1);
    s->data[0] = *c;
    return s;
}

extern "C" String *__fastcall String_AssignStr(String *s, void *, const char *text) {
    if (!text) {
        s->length = 0;
        s->data[0] = 0;
        return s;
    }
    String_SetLength(s, nullptr, str_len(text));
    str_cpy(s->data, text);
    return s;
}

extern "C" String *__fastcall String_Assign(String *s, void *, const StringView *from) {
    if (reinterpret_cast<const void *>(s) == from) return s;
    if (s->length < from->length) String_SetLength(s, nullptr, from->length);
    str_ncpy(s->data, from->data, from->length);
    String_SetLength(s, nullptr, from->length);
    return s;
}

extern "C" String *__fastcall String_AssignWide(String *s, void *, const WString *from) {
    const StringView view = {kViewVtable, from->length, reinterpret_cast<const char *>(from->data), from->capacity};
    return String_Assign(s, nullptr, &view);
}

extern "C" String *__fastcall String_AppendStr(String *s, void *, const char *text) {
    if (text) {
        String_SetLength(s, nullptr, str_len(text) + s->length);
        str_cat(s->data, text);
    }
    return s;
}

extern "C" String *__fastcall String_AppendChar(String *s, void *, const char *c) {
    if (*c) {
        String_SetLength(s, nullptr, s->length + 1);
        s->data[s->length - 1] = *c;
    }
    return s;
}

extern "C" String *__fastcall String_Append(String *s, void *, const StringView *from) {
    String_SetLength(s, nullptr, s->length + from->length);
    str_ncat(s->data, from->data, from->length);
    return s;
}

extern "C" void __cdecl String_Format(String *s, const char *format, ...) {
    if (!format) {
        String_SetLength(s, nullptr, 0);
        return;
    }
    va_list args;
    va_start(args, format);
    String_SetLength(s, nullptr, format_estimate<char, false>(format, reinterpret_cast<const u32 *>(args)));
    const s32 n = g_env->vsprintf(s->data, format, args);
    va_end(args);
    s->data[n] = 0;
    s->length = (u32)n;
}

extern "C" char *__fastcall String_GetBuffer(String *s, void *, u32 length) {
    String_SetLength(s, nullptr, length);
    return s->data;
}

extern "C" void __fastcall String_ReleaseBuffer(String *s, void *, u32 length) {
    if (length != 0x7fffffff) {
        String_SetLength(s, nullptr, length);
        s->data[length] = 0;
        return;
    }
    const u32 n = str_len(s->data);
    String_SetLength(s, nullptr, n < s->capacity ? n : s->capacity);
}

extern "C" bool __fastcall String_Equals(const String *s, void *, const StringView *other) {
    return s->length == other->length && str_ncmp(s->data, other->data, s->length) == 0;
}

extern "C" bool __fastcall String_EqualsStr(const String *s, void *, const char *text) {
    if (!text) return s->length == 0;
    return str_len(text) == s->length && str_ncmp(s->data, text, s->length) == 0;
}

extern "C" bool __fastcall String_NotEqualsStr(const String *s, void *, const char *text) {
    return !String_EqualsStr(s, nullptr, text);
}

extern "C" bool __fastcall String_NotEqualsChar(const String *s, void *, const char *c) {
    return s->length > 1 || s->data[0] != *c;
}

extern "C" bool __fastcall String_Less(const String *s, void *, const StringView *other) {
    const s32 r = str_ncmp(s->data, other->data, s->length < other->length ? s->length : other->length);
    return r ? r < 0 : other->length > s->length;
}

extern "C" s32 __fastcall String_FindChar(const String *s, void *, const char *c, u32 start) {
    if (start > s->length) start = s->length;
    if (!*c) return -1;
    for (u32 i = start; i < s->length; ++i)
        if (s->data[i] == *c) return (s32)i;
    return -1;
}

extern "C" s32 __fastcall String_Find(const String *s, void *, const char *text, u32 start) {
    if (!text) return -1;
    const char *const at = str_str(s->data + (start < s->length ? start : s->length), text);
    return at ? (s32)(at - s->data) : -1;
}

extern "C" s32 __fastcall String_ReverseFindChar(const String *s, void *, const char *c, u32 start) {
    if (!*c || !s->length) return -1;
    const u32 last = s->length - 1;
    for (s32 i = (s32)(start < last ? start : last); i >= 0; --i)
        if (s->data[i] == *c) return i;
    return -1;
}

extern "C" s32 __fastcall String_ReverseFind(const String *s, void *, const char *text, u32 start) {
    if (!text || !s->length) return -1;
    const u32 n = str_len(text);
    if (start == 0xffffffff || start > s->length) start = s->length - n;
    if (n >= s->length || start > s->length) return -1;
    for (s32 i = (s32)start; i >= 0; --i)
        if (s->data[i] == *text && !str_ncmp(s->data + i, text, n)) return i;
    return -1;
}

extern "C" s32 __fastcall String_FindOneOf(const String *s, void *, const char *set, u32 start) {
    if (start > s->length) start = s->length;
    if (!set) return -1;
    for (u32 i = start; i < s->length; ++i)
        for (const char *c = set; *c; ++c)
            if (*c == s->data[i]) return (s32)i;
    return -1;
}

extern "C" s32 __fastcall String_ReverseFindOneOf(const String *s, void *, const char *set, u32 start) {
    if (!set || !s->length) return -1;
    const u32 last = s->length - 1;
    for (s32 i = (s32)(start < last ? start : last); i >= 0; --i)
        for (const char *c = set; *c; ++c)
            if (*c == s->data[i]) return i;
    return -1;
}

extern "C" StringView *__fastcall String_Left(const String *s, void *, StringView *out, u32 count) {
    const u32 n = count < s->length ? count : s->length;
    out->vtable = kViewVtable;
    out->data = s->data;
    out->length = n;
    return out;
}

extern "C" StringView *__fastcall String_Right(const String *s, void *, StringView *out, u32 count) {
    const u32 n = count < s->length ? count : s->length;
    out->vtable = kViewVtable;
    out->length = n;
    out->data = s->data + (s->length - n);
    return out;
}

extern "C" StringView *__fastcall String_From(const String *s, void *, StringView *out, u32 start) {
    const u32 at = start < s->length ? start : s->length;
    out->vtable = kViewVtable;
    out->length = s->length - at;
    out->data = s->data + at;
    return out;
}

extern "C" StringView *__fastcall String_Mid(const String *s, void *, StringView *out, u32 start, u32 count) {
    const u32 at = start < s->length ? start : s->length;
    const u32 rest = s->length - at;
    out->vtable = kViewVtable;
    out->data = s->data + at;
    out->length = count < rest ? count : rest;
    return out;
}

// --- WString ---

extern "C" void __fastcall WString_Alloc(WString *s, void *, u32 length) {
    s->length = length;
    s->grow = 8;
    s->capacity = capacity_for(length);
    s->data = static_cast<u16 *>(g_env->alloc(2 * s->capacity + 2));
    s->data[s->length] = 0;
}

extern "C" void __fastcall WString_SetLength(WString *s, void *, u32 length) {
    if (length <= s->capacity) {
        s->length = length;
        s->data[length] = 0;
        return;
    }
    u16 *const old = s->data;
    WString_Alloc(s, nullptr, length);
    str_cpy(s->data, old);
    g_env->free(old);
}

extern "C" WString *__fastcall WString_Ctor(WString *s) {
    s->data = nullptr;
    s->vtable = kWStringVtable;
    WString_Alloc(s, nullptr, 0);
    return s;
}

extern "C" WString *__fastcall WString_CtorStr(WString *s, void *, const u16 *text) {
    s->data = nullptr;
    s->length = 0;
    s->vtable = kWStringVtable;
    WString_Alloc(s, nullptr, text ? str_len(text) : 0);
    if (text) str_cpy(s->data, text);
    return s;
}

extern "C" WString *__fastcall WString_CtorCopy(WString *s, void *, const WString *from) {
    s->data = nullptr;
    s->length = 0;
    s->vtable = kWStringVtable;
    WString_Alloc(s, nullptr, from->length);
    str_ncpy(s->data, from->data, from->length);
    s->data[from->length] = 0;
    return s;
}

extern "C" void __fastcall WString_Dtor(WString *s) {
    s->vtable = kWStringVtable;
    g_env->free(s->data);
}

extern "C" WString *__fastcall WString_DeletingDtor(WString *s, void *, u32 flags) {
    WString_Dtor(s);
    if (flags & 1) g_env->free(s);
    return s;
}

extern "C" WString *__fastcall WString_AssignChar(WString *s, void *, const u16 *c) {
    if (!*c) {
        WString_SetLength(s, nullptr, 0);
        return s;
    }
    WString_SetLength(s, nullptr, 1);
    s->data[0] = *c;
    return s;
}

extern "C" WString *__fastcall WString_AssignStr(WString *s, void *, const u16 *text) {
    if (!text) {
        s->length = 0;
        s->data[0] = 0;
        return s;
    }
    WString_SetLength(s, nullptr, str_len(text));
    str_cpy(s->data, text);
    return s;
}

extern "C" WString *__fastcall WString_Assign(WString *s, void *, const WString *from) {
    if (s == from) return s;
    if (s->length < from->length) WString_SetLength(s, nullptr, from->length);
    str_ncpy(s->data, from->data, from->length);
    WString_SetLength(s, nullptr, from->length);
    return s;
}

extern "C" WString *__fastcall WString_AssignNarrow(WString *s, void *, const String *from) {
    const WString view = {kWViewVtable, from->length, reinterpret_cast<u16 *>(from->data), from->capacity, 0};
    return WString_Assign(s, nullptr, &view);
}

extern "C" WString *__fastcall WString_AppendStr(WString *s, void *, const u16 *text) {
    if (text) {
        WString_SetLength(s, nullptr, str_len(text) + s->length);
        str_cat(s->data, text);
    }
    return s;
}

extern "C" WString *__fastcall WString_AppendChar(WString *s, void *, const u16 *c) {
    if (*c) {
        WString_SetLength(s, nullptr, s->length + 1);
        s->data[s->length - 1] = *c;
    }
    return s;
}

extern "C" WString *__fastcall WString_Append(WString *s, void *, const WString *from) {
    WString_SetLength(s, nullptr, s->length + from->length);
    str_ncat(s->data, from->data, from->length);
    return s;
}

extern "C" void __cdecl WString_Format(WString *s, const u16 *format, ...) {
    if (!format) {
        WString_SetLength(s, nullptr, 0);
        return;
    }
    va_list args;
    va_start(args, format);
    WString_SetLength(s, nullptr, format_estimate<u16, true>(format, reinterpret_cast<const u32 *>(args)));
    const s32 n = g_env->vswprintf(s->data, format, args);
    va_end(args);
    s->data[n] = 0;
    s->length = (u32)n;
}

extern "C" bool __fastcall WString_NotEqualsStr(const WString *s, void *, const u16 *text) {
    if (!text) return s->length != 0;
    return str_len(text) != s->length || str_ncmp(s->data, text, s->length) != 0;
}
