#include "numget.h"

#include <cstdint>

#include "buffers.h"
#include "crt.h"
#include "iosbase.h"
#include "localefacet.h"
#include "moneyput.h"
#include "numput.h"
#include "stdstring.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The read pointer (through +0x1c) and its count (through +0x2c), as streambuf::sgetc and sbumpc read them. The vtable's slot 4 is underflow, slot 5 uflow.
u8 *&next_of(u8 *buffer) { return **reinterpret_cast<u8 ***>(buffer + 0x1c); }
u32 &left_of(u8 *buffer) { return **reinterpret_cast<u32 **>(buffer + 0x2c); }

bool waiting(u8 *buffer) {
    const u32 next = static_cast<u32>(reinterpret_cast<uintptr_t>(next_of(buffer)));
    return next && next < next + left_of(buffer);
}

u32 buffer_slot(u8 *buffer, u32 offset) {
    using Slot = u32(__fastcall *)(u8 *);
    return reinterpret_cast<const Slot *>((uintptr_t)at<u32>(buffer, 0))[offset / 4](buffer);
}

// istreambuf_iterator's operator*, == and ++ (0x6962d0, 0x696330, 0x6963f0: A's, here because they keep the iterator's state, which a stand-in cannot).
void fetch(NumGetIter &it) {
    if (it.got) return;
    if (it.buffer) {
        const u32 c = waiting(it.buffer) ? *next_of(it.buffer) : buffer_slot(it.buffer, 0x10);
        if (c != 0xffffffff) {
            it.value = static_cast<u8>(c);
            it.got = 1;
            return;
        }
    }
    it.buffer = nullptr;
    it.got = 1;
}

char deref(NumGetIter &it) {
    fetch(it);
    return static_cast<char>(it.value);
}

bool equal(NumGetIter &it, NumGetIter &other) {
    fetch(it);
    fetch(other);
    return !it.buffer == !other.buffer;
}

void increment(NumGetIter &it) {
    u8 *const buffer = it.buffer;
    if (buffer) {
        u32 c;
        if (waiting(buffer)) {
            --left_of(buffer);
            u8 *&next = next_of(buffer);
            const u8 *const from = next;
            next = next + 1;
            c = *from;
        } else {
            c = buffer_slot(buffer, 0x14);
        }
        if (c != 0xffffffff) {
            it.got = 0;
            return;
        }
    }
    it.buffer = nullptr;
    it.got = 1;
}

// string(n, c), string::append(n, c), string::operator[] (0x696090, 0x696160, 0x696230: A's, on the string's own ports).
void string_fill(StdString &s, u32 n, u32 c) {
    StdString_Tidy(&s, nullptr, 0);
    StdString_AssignFill(&s, nullptr, n, c);
}

void string_append_one(StdString &s, char c) {
    if (global<u32>(0x006fe5ac) - s.len <= 1) Std_Xlen();
    const u32 total = s.len + 1;
    if (StdString_Grow(&s, nullptr, total, 0)) {
        s.ptr[s.len] = c;
        s.len = total;
        s.ptr[total] = 0;
    }
}

char &string_at(StdString &s, u32 position) {
    if (s.len < position || !s.ptr) return global<char>(0x006fe5b0);
    StdString_Freeze(&s, nullptr);
    return s.ptr[position];
}

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

}  // extern "C"
