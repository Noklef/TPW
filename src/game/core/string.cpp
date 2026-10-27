// The game's string classes: reimplementation of 0x5f5fc0-0x5f6c50, 0x5f7120/0x5f7150 and
// 0x5f78e0-0x5f7e60.
#include "string.h"

#include <cstdint>
#include <cstring>
#include <type_traits>

#include "crt.h"
#include "../files/file.h"
#include "gameheap.h"
#include "methods.h"
#include "stdstring.h"
#include "vcall.h"
#include "wtraits.h"

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

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

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

extern "C" {

s32 __fastcall WString_Compare(const u8 *self, void *, const u8 *other) {
    const u32 otherLength = at<u32>(other, 8), selfLength = at<u32>(self, 8);
    const u16 *b = at<const u16 *>(other, 4);
    if (!b)
        b = reinterpret_cast<const u16 *>(0x006fd6c4);
    const u16 *a = at<const u16 *>(self, 4);
    for (u32 n = selfLength < otherLength ? selfLength : otherLength; n; n--, a++, b++) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
    }
    if (selfLength < otherLength)
        return -1;
    return selfLength != otherLength ? 1 : 0;
}

s32 __fastcall WString_CompareSub(const u8 *self, void *, u32 pos, u32 n, const u16 *other, u32 count) {
    if (at<u32>(self, 8) < pos)
        StdStrings_Xran();
    const u32 left = at<u32>(self, 8) - pos;
    if (left < n)
        n = left;
    const u16 *a = at<const u16 *>(self, 4);
    if (a)
        a += pos;
    for (u32 k = n < count ? n : count; k; k--, a++, other++) {
        if (*a != *other)
            return *a < *other ? -1 : 1;
    }
    if (n < count)
        return -1;
    return n != count ? 1 : 0;
}

s32 __fastcall WString_CompareText(const u8 *self, void *, const u16 *text) {
    const u32 count = WTraits_Length(text);
    return WString_CompareSub(self, nullptr, 0, at<u32>(self, 8), text, count);
}

u32 __cdecl Names_FirstAfter(const u8 *a, const u8 *b) {
    return Crt().stricmp(at<const char *>(a, 8), at<const char *>(b, 8)) > 0 ? 1 : 0;
}

u32 __cdecl Names_Equal(const u8 *a, const u8 *b) {
    return Crt().stricmp(at<const char *>(a, 8), at<const char *>(b, 8)) == 0 ? 1 : 0;
}

u32 __cdecl WNames_Equal(const u8 *a, const u8 *b) {
    return Crt().wcsicmp(at<const u16 *>(a, 8), at<const u16 *>(b, 8)) == 0 ? 1 : 0;
}

u8 *__fastcall WNamedList_FindAfter(u8 *list, void *, const u8 *key) {
    u8 *node = at<u8 *>(list, 4);
    if (!node)
        return nullptr;
    for (;;) {
        u8 *const item = at<u8 *>(node, 0);
        node = at<u8 *>(node, 4);
        if (WNames_Equal(key, item) != 0 || !node)
            break;
    }
    return node ? at<u8 *>(node, 0) : nullptr;  // (the item after the match, sic)
}

u8 *__fastcall Record_CopyWithText(u8 *dest, void *, const u8 *source) {
    for (u32 offset = 0; offset < 0x20; offset += 4)
        at<u32>(dest, offset) = at<u32>(source, offset);
    const u8 *from = source + 0x24;
    u8 *to = dest + 0x24;
    u32 length = 0;
    while (from[length])
        ++length;
    for (u32 i = 0; i <= length; ++i)
        to[i] = from[i];
    at<u32>(dest, 0x20) = at<u32>(source, 0x20) - (u32)(uintptr_t)source + (u32)(uintptr_t)dest;
    return dest;
}

u8 *__fastcall Obj_NameToWString(u8 *object, void *, u8 *out) {
    WString_CtorStr((WString *)out, nullptr, at<const u16 *>(object, 0x54));
    return out;
}

u32 __fastcall Names_FindLastValue(u8 *object, void *, const char *name) {
    u32 value = 1;
    u8 *node = at<u8 *>(at<u8 *>(object, 0x2c), 4);
    while (node) {
        u8 *next = at<u8 *>(node, 4);
        u8 *entry = at<u8 *>(node, 0);
        if (Crt().stricmp(at<const char *>(entry, 0x14), name) == 0)
            value = at<u32>(entry, 8);
        node = next;
    }
    return value;
}

u8 *__fastcall Strings5_Ctor(u8 *object) {
    for (u32 offset = 0; offset <= 0x50; offset += 0x14)
        String_Ctor((String *)(object + offset));
    return object;
}

String *__stdcall Strings_CopyRange(const String *first, const String *last, String *dest) {
    while (first != last) {
        if (dest)
            String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(first));
        first = reinterpret_cast<const String *>((uintptr_t)first + 0x14);
        dest = reinterpret_cast<String *>((uintptr_t)dest + 0x14);
    }
    return dest;
}

void __stdcall Strings_FillCopy(String *dest, u32 count, const String *from) {
    for (; count != 0; --count) {
        if (dest)
            String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(from));
        dest = reinterpret_cast<String *>((uintptr_t)dest + 0x14);
    }
}

void __cdecl String_PlacementCopy(String *dest, const String *from) {
    if (dest)
        String_CtorCopy(dest, nullptr, reinterpret_cast<const StringView *>(from));
}

void __fastcall StringVector_InsertN(u8 *vec, void *, u8 *position, u32 count, const String *value) {
    const auto strings = [](const u8 *to, const u8 *from) { return (s32)(to - from) / 0x14; };
    const auto str = [](u8 *p) { return reinterpret_cast<String *>(p); };
    u8 *const last = at<u8 *>(vec, 8);
    if ((u32)strings(at<u8 *>(vec, 0xc), last) < count) {
        u8 *first = at<u8 *>(vec, 4);
        const u32 size = first ? (u32)strings(last, first) : 0;
        const u32 grow = count < size ? size : count;
        const s32 capacity = (s32)(size + grow);
        u8 *const block = static_cast<u8 *>(GameHeap_Alloc((u32)(capacity < 0 ? 0 : capacity) * 0x14));
        u8 *const at_position = reinterpret_cast<u8 *>(Strings_CopyRange(str(at<u8 *>(vec, 4)), str(position), str(block)));
        Strings_FillCopy(str(at_position), count, value);
        Strings_CopyRange(str(position), str(at<u8 *>(vec, 8)), str(at_position + count * 0x14));
        for (u8 *p = at<u8 *>(vec, 4), *end = at<u8 *>(vec, 8); p != end; p += 0x14)
            vcall<u32>(p, 0xc, 0u);
        GameHeap_Free(at<u8 *>(vec, 4));
        at<u8 *>(vec, 0xc) = block + capacity * 0x14;
        first = at<u8 *>(vec, 4);
        const u32 old = first ? (u32)strings(at<u8 *>(vec, 8), first) : 0;
        at<u8 *>(vec, 4) = block;
        at<u8 *>(vec, 8) = block + (count + old) * 0x14;
    } else if ((u32)strings(last, position) < count) {
        const u32 bytes = count * 0x14;
        Strings_CopyRange(str(position), str(last), str(position + bytes));
        u8 *const end = at<u8 *>(vec, 8);
        Strings_FillCopy(str(end), count - (u32)strings(end, position), value);
        for (u8 *p = position, *stop = at<u8 *>(vec, 8); p != stop; p += 0x14)
            vcall<u32>(p, 0x18, value);
        at<u8 *>(vec, 8) += bytes;
    } else if (count != 0) {
        const u32 bytes = count * 0x14;
        Strings_CopyRange(str(last - bytes), str(last), str(last));
        u8 *to = at<u8 *>(vec, 8);
        for (u8 *from = to - bytes; from != position;) {
            to -= 0x14;
            from -= 0x14;
            vcall<u32>(to, 0x18, from);
        }
        for (u8 *p = position, *stop = position + bytes; p != stop; p += 0x14)
            vcall<u32>(p, 0x18, value);
        at<u8 *>(vec, 8) += bytes;
    }
}

u32 __fastcall NamedList_Find(u8 **ref, void *, const u8 *key, u8 **out) {
    if (u8 *list = ref[0]) {
        for (u8 *node = at<u8 *>(list, 4); node;) {
            u8 *next = at<u8 *>(node, 4);
            u8 *object = at<u8 *>(node, 0);
            if (Names_Equal(object + 4, key) == 1) {
                *out = object;
                ref[1] = object;
                return 1;
            }
            node = next;
        }
    }
    ref[1] = nullptr;
    return 0;
}

u32 __fastcall Obj_HasNamedObject(u8 *object, void *, const u8 *key) {
    if (u8 *list = at<u8 *>(object, 0x4c)) {
        for (u8 *node = at<u8 *>(list, 4); node;) {
            u8 *next = at<u8 *>(node, 4);
            if (Names_Equal(at<u8 *>(node, 0) + 4, key) != 0)
                return 1;
            node = next;
        }
    }
    return 0;
}

void __fastcall Strings5_Dtor(u8 *object) {
    for (int offset = 0x50; offset >= 0; offset -= 0x14)
        String_Dtor((String *)(object + offset));
}

u32 __cdecl Text_NarrowToGlobalBuffer(const u16 *text) {
    if (const u32 table = global<u32>(0x0078ba80)) {
        u32 status;
        Narrow_FromWide(reinterpret_cast<const u8 *>(table), nullptr, &status, reinterpret_cast<u8 *>(0x007b5378), text, 0x3fff);
    }
    return 0x007b5378;
}

s32 __fastcall ByteString_Compare(const u8 *object, void *, const u8 *other) {
    const u32 otherLength = at<u32>(other, 8);
    const u8 *otherText = at<const u8 *>(other, 4);
    if (!otherText)
        otherText = reinterpret_cast<const u8 *>(0x006fe5b0);
    const u32 length = at<u32>(object, 8);
    const u8 *text = at<const u8 *>(object, 4);
    const u32 common = length < otherLength ? length : otherLength;
    for (u32 i = 0; i < common; ++i) {
        if (text[i] != otherText[i])
            return text[i] < otherText[i] ? -1 : 1;
    }
    if (length >= otherLength)
        return length != otherLength ? 1 : 0;
    return -1;
}

s32 __cdecl String_AssignFromWide(String *s, const WString *wide) {
    using WideToMulti = s32(__stdcall *)(u32, u32, const u16 *, s32, char *, s32, const char *, s32 *);
    const WideToMulti convert = *reinterpret_cast<const WideToMulti *>(0x006fd1dc);
    s32 result = 0;
    const s32 size = convert(0, 0, wide->data, -1, nullptr, 0, nullptr, nullptr);
    if (size > 0) {
        if (char *const block = static_cast<char *>(Mem_GlobalAlloc(static_cast<u32>(size)))) {
            result = convert(0, 0, wide->data, -1, block, size, nullptr, nullptr);
            vcall<void>(s, 0x14, static_cast<const char *>(block));
            Mem_GlobalFree(block);
        }
    }
    return result;
}

s32 __cdecl WString_AssignFromAnsi(WString *wide, const String *s) {
    using MultiToWide = s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32);
    const MultiToWide convert = *reinterpret_cast<const MultiToWide *>(0x006fd174);
    s32 result = 0;
    const s32 units = convert(0, 0, s->data, -1, nullptr, 0);
    if (units > 0) {
        if (u16 *const block = static_cast<u16 *>(Mem_GlobalAlloc(static_cast<u32>(units) * 2))) {
            result = convert(0, 0, s->data, -1, block, units);
            vcall<void>(wide, 0x14, static_cast<const u16 *>(block));
            Mem_GlobalFree(block);
        }
    }
    return result;
}

u16 *__cdecl String_AnsiToWide(const char *text, u16 *buffer, s32 size) {
    using MultiToWide = s32(__stdcall *)(u32, u32, const char *, s32, u16 *, s32);
    const MultiToWide convert = *reinterpret_cast<const MultiToWide *>(0x006fd174);
    if (!buffer) {
        u16 *const fixed = reinterpret_cast<u16 *>(0x007ba058);
        convert(0, 0, text, -1, fixed, 0x3fff);
        return fixed;
    }
    convert(0, 0, text, -1, buffer, size);
    return buffer;
}

String *__cdecl String_StripSuffix(const String *suffix, String *s) {
    StringView view;
    String tail;
    String_CtorCopy(&tail, nullptr, String_Right(s, nullptr, &view, suffix->length));
    const bool equal = Crt().stricmp(tail.data, suffix->data) == 0;
    String_Dtor(&tail);
    if (equal)
        String_Assign(s, nullptr, String_Left(s, nullptr, &view, s->length - suffix->length));
    return s;
}

String *__cdecl String_SkipDotPrefix(String *s) {
    const String *const dot = reinterpret_cast<const String *>((uintptr_t)0x0078c648);
    const String *const backslash = reinterpret_cast<const String *>((uintptr_t)0x0078bee0);
    StringView view;
    String part;
    String_CtorCopy(&part, nullptr, String_Left(s, nullptr, &view, dot->length));
    const u32 starts = Names_Equal(reinterpret_cast<const u8 *>(&part), reinterpret_cast<const u8 *>(dot));
    String_Dtor(&part);
    if (starts == 0)
        return s;
    u32 i = 1;
    for (;;) {
        String_CtorCopy(&part, nullptr, String_Mid(s, nullptr, &view, i, backslash->length));
        bool skip = Names_Equal(reinterpret_cast<const u8 *>(&part), reinterpret_cast<const u8 *>(backslash)) == 1;
        if (!skip) {
            String other;
            String_CtorCopy(&other, nullptr, String_Mid(s, nullptr, &view, i, dot->length));
            skip = Names_Equal(reinterpret_cast<const u8 *>(&other), reinterpret_cast<const u8 *>(dot)) == 1;
            String_Dtor(&other);
        }
        String_Dtor(&part);
        if (!skip)
            break;
        ++i;   // (one char on, whatever the globals' lengths: sic)
    }
    String_Assign(s, nullptr, String_From(s, nullptr, &view, i));
    return s;
}

void __cdecl String_EnsureTrailingBackslash(String *s) {
    const String *const backslash = reinterpret_cast<const String *>((uintptr_t)0x00f80be0);
    StringView view;
    String tail;
    String_CtorCopy(&tail, nullptr, String_Right(s, nullptr, &view, backslash->length));
    const u32 ends = Names_Equal(reinterpret_cast<const u8 *>(&tail), reinterpret_cast<const u8 *>(backslash));
    String_Dtor(&tail);
    if (ends == 0)
        vcall<void>(s, 0x28, backslash);
}

void __fastcall Strings5_JoinPath(u8 *object, void *, const StringView *name) {
    String *const path = reinterpret_cast<String *>(object + 0x3c);
    vcall<void>(path, 0x18, object + 0x28);
    String_EnsureTrailingBackslash(path);
    vcall<void>(path, 0x28, name);
}

String *__cdecl String_ReadToken(String *out, const char **cursor, u32 delimiter) {
    String token;
    String_Ctor(&token);
    while (**cursor != static_cast<char>(delimiter)) {
        vcall<void>(&token, 0x24, *cursor);
        ++*cursor;
    }
    ++*cursor;
    String_CtorCopy(out, nullptr, reinterpret_cast<const StringView *>(&token));
    String_Dtor(&token);
    return out;
}

WString *__cdecl WString_ReadToken(WString *out, const char **cursor, u32 delimiter) {
    WString token;
    WString_Ctor(&token);
    const char stop = static_cast<char>(delimiter);
    while (**cursor != stop) {
        // The original's MOVSX DX stores the whole EDX over the delimiter's argument slot and passes that: the unit, then EDX's upper half (the
        // cursor's after the first char; before it, what EDX held at the call: sic, AppendChar reads the unit only).
        delimiter = (static_cast<u32>(reinterpret_cast<uintptr_t>(*cursor)) & 0xffff0000u)
                  | static_cast<u16>(static_cast<s16>(static_cast<signed char>(**cursor)));
        vcall<void>(&token, 0x24, &delimiter);
        ++*cursor;
    }
    ++*cursor;
    WString_CtorCopy(out, nullptr, &token);
    WString_Dtor(&token);
    return out;
}

u8 *__fastcall WString_CtorReturning(u8 *string) {
    WString_Ctor(reinterpret_cast<WString *>(string));
    return string;
}

}  // extern "C"
