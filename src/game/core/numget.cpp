#include "numget.h"

#include <cstdint>

#include "buffers.h"
#include "crt.h"
#include "crtfloat.h"
#include "globals.h"
#include "iosbase.h"
#include "localefacet.h"
#include "moneyput.h"
#include "numput.h"
#include "stdstring.h"
#include "streambufiter.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// istreambuf_iterator's operator*, equal and _Inc, and string(n, c), append(n, c), operator[]: the exe's own functions (streambufiter, stdstring).
StreambufIter *iter(NumGetIter &it) { return reinterpret_cast<StreambufIter *>(&it); }

char deref(NumGetIter &it) { return static_cast<char>(*StreambufIter_Deref(iter(it), nullptr)); }

bool equal(NumGetIter &it, NumGetIter &other) { return StreambufIter_Equal(iter(it), nullptr, iter(other)) != 0; }

void increment(NumGetIter &it) { StreambufIter_Inc(iter(it), nullptr); }

// (the allocator byte its own: the original's is a stack byte of the caller's)
void string_fill(StdString &s, u32 n, u32 c) { StdString_CtorFill(&s, nullptr, n, c, reinterpret_cast<const u8 *>(&s)); }

void string_append_one(StdString &s, char c) { StdString_AppendFill(&s, nullptr, 1, static_cast<u32>(c)); }

char &string_at(StdString &s, u32 position) { return *StdString_At(&s, nullptr, position); }

u32 base_of(u32 flags) {
    const u32 field = flags & 0xe00;
    return field == 0x400 ? 8 : field == 0x800 ? 16 : field ? 10 : 0;
}

// _Getifld's body: the sign, a base's prefix, then up to 0x1f characters of digits into text, separators counted in groups and checked against the grouping. Frees the grouping; the answer is the base.
u32 scan_integer(char *text, NumGetIter &first, NumGetIter &last, u32 flags, StdString &grouping, char separator) {
    char *out = text;
    if (!equal(first, last)) {
        if (deref(first) == '+') {
            *out++ = '+';
            increment(first);
        } else if (deref(first) == '-') {
            *out++ = '-';
            increment(first);
        }
    }
    u32 base = base_of(flags);
    bool seen = false;
    if (!equal(first, last) && deref(first) == '0') {
        seen = true;
        increment(first);
        if (!equal(first, last) && (deref(first) == 'x' || deref(first) == 'X') && (!base || base == 16)) {
            base = 16;
            seen = false;
            increment(first);
        } else if (!base) {
            base = 8;
        }
    }
    const u32 digits = !base || base == 10 ? 10 : base == 8 ? 8 : 22;
    StdString groups;
    string_fill(groups, 1, seen ? 1 : 0);
    char *const end = text + 0x1f;
    bool nonzero = false;
    u32 group = 0;
    while (!equal(first, last)) {
        const char c = deref(first);
        *out = c;
        if (CRT_memchr(reinterpret_cast<const char *>((uintptr_t)0x00709b70), static_cast<s32>(c), digits)) {
            if ((nonzero || *out != '0') && out < end) {
                ++out;
                nonzero = true;
            }
            seen = true;
            if (string_at(groups, group) != 0x7f) ++string_at(groups, group);
        } else {
            if (!string_at(groups, group) || !separator || deref(first) != separator) break;
            string_append_one(groups, 0);
            ++group;
        }
        increment(first);
    }
    if (group) {
        if (string_at(groups, group) > 0) {
            ++group;
        } else {
            seen = false;
        }
    }
    const char *wanted = grouping.ptr ? grouping.ptr : &global<char>(0x006fe5b0);
    if (seen) {
        while (group && *wanted != 0x7f) {
            --group;
            if (group ? *wanted != string_at(groups, group) : *wanted < string_at(groups, group)) {
                seen = false;
            } else if (static_cast<signed char>(wanted[1]) > 0) {
                ++wanted;
            }
            --group;
            if (!seen) break;
        }
    }
    if (!seen) {
        out = text;
    } else if (!nonzero) {
        *out++ = '0';
    }
    *out = 0;
    StdString_Tidy(&groups, nullptr, 1);
    StdString_Tidy(&grouping, nullptr, 1);
    return base;
}

// What each do_get of a number begins with, up to the facet's answers.
template <typename Scan> void with_facet(u8 **locale, Scan scan) {
    u8 *const facet = reinterpret_cast<u8 *>(UseFacet_Numpunct(locale, 0, 1));
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    using Slot = u32(__fastcall *)(u8 *);
    const char separator = static_cast<char>(reinterpret_cast<const Slot *>((uintptr_t)at<u32>(facet, 0))[2](facet));
    scan(grouping, separator);
}

u32 *result(u32 *out, const NumGetIter &first) {
    const u32 *const from = reinterpret_cast<const u32 *>(&first);
    out[0] = from[0];
    out[1] = from[1];
    return out;
}

// do_get of an unsigned short or int: the '-' left out of what strtoul reads, and negated after.
template <typename T> u32 *get_unsigned(u32 *out, NumGetIter &first, NumGetIter &last, u8 *ios, u32 *state, T *value, u32 limit) {
    *CRT_errno() = 0;
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    const u32 flags = at<u32>(ios, 0xc);
    char text[0x20];
    u32 base = 0;
    with_facet(&locale, [&](StdString &grouping, char separator) { base = scan_integer(text, first, last, flags, grouping, separator); });
    CountedRef_Release(&locale);
    const char *const from = text[0] == '-' ? text + 1 : text;
    const char *end;
    u32 number = CRT_strtoul(from, &end, static_cast<s32>(base));
    if (equal(first, last)) *state |= 1;
    if (end == from || *CRT_errno() || number > limit) {
        *state |= 2;
    } else {
        if (text[0] == '-') number = 0u - number;
        *value = static_cast<T>(number);
    }
    return result(out, first);
}

// One character into text at out, which moves on while fewer than room digits are kept; false when it is not a digit (written all the same).
bool take_digit(char *&out, NumGetIter &first, s32 &digits, s32 room) {
    const char c = deref(first);
    *out = c;
    if (!CRT_isdigit(c)) return false;
    if (digits < room) {
        ++out;
        ++digits;
    }
    return true;
}

void take_sign(char *&out, NumGetIter &first, NumGetIter &last) {
    if (equal(first, last)) return;
    if (deref(first) == '+') {
        *out++ = '+';
        increment(first);
    } else if (deref(first) == '-') {
        *out++ = '-';
        increment(first);
    }
}

// Leading zeros dropped, one '0' written for them. Whether there were any.
bool take_zeros(char *&out, NumGetIter &first, NumGetIter &last) {
    bool seen = false;
    for (; !equal(first, last) && deref(first) == '0'; increment(first)) seen = true;
    if (seen) *out++ = '0';
    return seen;
}

// _Getffld, inlined in each float do_get: the sign, 0x24 digits kept (the rest counted in the power of ten), the facet's decimal point as the C locale's,
// zeros after it counted down before any digit, the fraction, then 'e' with a sign and up to 8 digits. Text empty without a digit. The answer is the power.
s32 scan_float(char *text, NumGetIter &first, NumGetIter &last, u8 *facet) {
    char *out = text;
    take_sign(out, first, last);
    bool seen = take_zeros(out, first, last);
    s32 power = 0;
    s32 digits = 0;
    for (; !equal(first, last); increment(first)) {
        const s32 kept = digits;
        if (!take_digit(out, first, digits, 0x24)) break;
        if (kept == digits) ++power;
        seen = true;
    }
    if (!equal(first, last)) {
        using Slot = u32(__fastcall *)(u8 *);
        const char point = static_cast<char>(reinterpret_cast<const Slot *>((uintptr_t)at<u32>(facet, 0))[1](facet));
        if (deref(first) == point) {
            *out++ = **reinterpret_cast<const char *const *>((uintptr_t)Global781fc8_Get_691530());
            increment(first);
        }
    }
    if (!digits) {
        for (; !equal(first, last) && deref(first) == '0'; increment(first)) {
            --power;
            seen = true;
        }
        if (power < 0) {
            *out++ = '0';
            ++power;
        }
    }
    for (; !equal(first, last) && take_digit(out, first, digits, 0x24); increment(first)) seen = true;
    if (seen && !equal(first, last) && (deref(first) == 'e' || deref(first) == 'E')) {
        *out++ = 'e';
        increment(first);
        digits = 0;
        take_sign(out, first, last);
        seen = take_zeros(out, first, last);
        for (; !equal(first, last) && take_digit(out, first, digits, 8); increment(first)) seen = true;
    }
    if (!seen) out = text;
    *out = 0;
    return power;
}

// do_get of a float, a double, a long double: the scanned text and its power to _Stod or _Stold, the locale kept over it.
template <typename T> u32 *get_float(u32 *out, NumGetIter &first, NumGetIter &last, u8 *ios, u32 *state, T *value, double(__cdecl *convert)(const char *, const char **, s32)) {
    *CRT_errno() = 0;
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    char text[0x3c];
    const s32 power = scan_float(text, first, last, reinterpret_cast<u8 *>(UseFacet_Numpunct(&locale, 0, 1)));
    const char *end;
    const T number = static_cast<T>(convert(text, &end, power));
    CountedRef_Release(&locale);
    if (equal(first, last)) *state |= 1;
    if (end != text && !*CRT_errno()) {
        *value = number;
    } else {
        *state |= 2;
    }
    return result(out, first);
}

// do_get of a long or an unsigned long: the whole text to the C library, the locale kept over it.
template <typename Convert> u32 *get_long(u32 *out, NumGetIter &first, NumGetIter &last, u8 *ios, u32 *state, u32 *value, Convert convert) {
    *CRT_errno() = 0;
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    const u32 flags = at<u32>(ios, 0xc);
    char text[0x20];
    u32 base = 0;
    with_facet(&locale, [&](StdString &grouping, char separator) { base = scan_integer(text, first, last, flags, grouping, separator); });
    const char *end;
    const u32 number = convert(text, &end, static_cast<s32>(base));
    CountedRef_Release(&locale);
    if (equal(first, last)) *state |= 1;
    if (end == text || *CRT_errno()) {
        *state |= 2;
    } else {
        *value = number;
    }
    return result(out, first);
}

// The alphabetic form: the names side by side, the input matched against them a character at a time. The answer is the index of the name read to its end, negative if none.
s32 scan_names(NumGetIter &first, NumGetIter &last, u8 *facet) {
    StdString names;
    string_fill(names, 1, 0);
    StdString name;
    StdString_AppendSub(&names, nullptr, MoneyPunct_CurrSymbol(facet, nullptr, &name), 0, global<u32>(0x006fe5ac));
    StdString_Tidy(&name, nullptr, 1);
    string_append_one(names, 0);
    StdString_AppendSub(&names, nullptr, MoneyPunct_PositiveSign(facet, nullptr, &name), 0, global<u32>(0x006fe5ac));
    StdString_Tidy(&name, nullptr, 1);
    const char *const list = names.ptr ? names.ptr : &global<char>(0x006fe5b0);
    u32 count = 2;
    if (*list) {
        for (const char *p = list; *p; ++p) {
            if (*p == *list) ++count;
        }
    }
    StdString lengths;
    string_fill(lengths, count, 0);
    s32 found = -2;
    u32 column = 1;
    for (;;) {
        u32 offset = 0;
        bool alive = false;
        for (u32 i = 0; i < count; ++i) {
            if (list[offset] != 0) {
                while (list[offset] != *list) {
                    ++offset;
                    if (!list[offset]) break;
                }
            }
            if (string_at(lengths, i)) {
                offset += static_cast<s32>(string_at(lengths, i));
                continue;
            }
            offset += column;
            const char wanted = list[offset];
            const char clamped = static_cast<char>(column < 0x7f ? column : 0x7f);
            if (wanted == *list || wanted == 0) {
                string_at(lengths, i) = clamped;
                found = static_cast<s32>(i);
            } else if (!equal(first, last) && wanted == deref(first)) {
                alive = true;
            } else {
                string_at(lengths, i) = clamped;
            }
        }
        if (!alive || equal(first, last)) break;
        ++column;
        increment(first);
        found = -1;
    }
    StdString_Tidy(&lengths, nullptr, 1);
    StdString_Tidy(&names, nullptr, 1);
    return found;
}

}  // namespace

extern "C" {

u32 __cdecl NumGet_Getifld(char *text, NumGetIter *first, NumGetIter *last, u32 flags, u8 **locale) {
    u8 *const facet = NumPut_NumpunctOf(locale);
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    using Slot = u32(__fastcall *)(u8 *);
    const char separator = static_cast<char>(reinterpret_cast<const Slot *>((uintptr_t)at<u32>(facet, 0))[2](facet));
    return scan_integer(text, *first, *last, flags, grouping, separator);
}

u32 *__fastcall NumGet_DoGetBool(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *value) {
    s32 answer = -1;
    if (at<u32>(ios, 0xc) & 0x4000) {
        u8 *locale;
        Obj_ShareMember20(ios, nullptr, &locale);
        u8 *const facet = NumPut_NumpunctOf(&locale);
        CountedRef_Release(&locale);
        answer = scan_names(first, last, facet);
    } else {
        *CRT_errno() = 0;
        u8 *locale;
        Obj_ShareMember20(ios, nullptr, &locale);
        char text[0x20];
        const u32 base = NumGet_Getifld(text, &first, &last, at<u32>(ios, 0xc), &locale);
        const char *end;
        const u32 number = CRT_strtoul(text, &end, static_cast<s32>(base));
        CountedRef_Release(&locale);
        if (end != text && !*CRT_errno() && number <= 1) answer = static_cast<s32>(number);
    }
    if (equal(first, last)) *state |= 1;
    if (answer < 0) {
        *state |= 2;
    } else {
        *value = answer != 0;
    }
    return result(out, first);
}

u32 *__fastcall NumGet_DoGetUshort(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u16 *value) { return get_unsigned(out, first, last, ios, state, value, 0xffff); }

u32 *__fastcall NumGet_DoGetUint(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value) { return get_unsigned(out, first, last, ios, state, value, 0xffffffff); }

u32 *__fastcall NumGet_DoGetLong(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value) {
    return get_long(out, first, last, ios, state, value, [](const char *text, const char **end, s32 base) { return static_cast<u32>(CRT_strtol(text, end, base)); });
}

u32 *__fastcall NumGet_DoGetUlong(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value) {
    return get_long(out, first, last, ios, state, value, [](const char *text, const char **end, s32 base) { return CRT_strtoul(text, end, base); });
}

// do_get of a pointer: hexadecimal whatever the stream's flags say. (The original's loop for a second word never runs: one word, then out.)
u32 *__fastcall NumGet_DoGetPointer(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u32 *value) {
    *CRT_errno() = 0;
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    char text[0x20];
    with_facet(&locale, [&](StdString &grouping, char separator) { scan_integer(text, first, last, 0x800, grouping, separator); });
    const char *end;
    const u32 number = CRT_strtoul(text, &end, 0x10);
    CountedRef_Release(&locale);
    if (equal(first, last)) *state |= 1;
    if (end == text || *CRT_errno()) *state |= 2;
    if (!(*state & 2)) *value = number;
    return result(out, first);
}

u32 *__fastcall NumGet_DoGetFloat(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, float *value) { return get_float(out, first, last, ios, state, value, CRT_Stold); }

u32 *__fastcall NumGet_DoGetDouble(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, double *value) { return get_float(out, first, last, ios, state, value, CRT_Stod); }

u32 *__fastcall NumGet_DoGetLongDouble(void *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, double *value) { return get_float(out, first, last, ios, state, value, CRT_Stold); }

}  // extern "C"
