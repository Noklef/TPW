#include "timeget.h"

#include "crt.h"
#include "stdstring.h"
#include "streambufiter.h"

namespace {

// istreambuf_iterator's equal, operator* and ++ (inlined in the originals, the same steps): the exe's own functions (streambufiter).
bool equal(NumGetIter *it, NumGetIter *x, char) { return StreambufIter_Equal(reinterpret_cast<StreambufIter *>(it), nullptr, reinterpret_cast<StreambufIter *>(x)) != 0; }
bool equal(NumGetIter *it, NumGetIter *x, u16) { return WStreambufIter_Equal(reinterpret_cast<WStreambufIter *>(it), nullptr, reinterpret_cast<WStreambufIter *>(x)) != 0; }
char deref(NumGetIter *it, char) { return static_cast<char>(*StreambufIter_Deref(reinterpret_cast<StreambufIter *>(it), nullptr)); }
u16 deref(NumGetIter *it, u16) { return *WStreambufIter_Deref(reinterpret_cast<WStreambufIter *>(it), nullptr); }
void increment(NumGetIter *it, char) { StreambufIter_Inc(reinterpret_cast<StreambufIter *>(it), nullptr); }
void increment(NumGetIter *it, u16) { WStreambufIter_Inc(reinterpret_cast<WStreambufIter *>(it), nullptr); }

// A literal as the stream's unit (wchar_t: btowc, called after the character is fetched), and a unit as a char (wchar_t: _Wctob).
char lit(char, char c) { return c; }
u16 lit(u16, char c) { return static_cast<u16>(CRT_btowc(static_cast<u32>(c))); }
char narrow(char c) { return c; }
char narrow(u16 unit) { return static_cast<char>(CRT_Wctob(unit)); }
template <typename C> bool is(NumGetIter &it, char c) {
    const C got = deref(&it, C());
    return got == lit(C(), c);
}

// The count string's operator[] (not const: it freezes the block).
char &at(StdString &s, u32 i) { return *StdString_At(&s, nullptr, i); }

template <typename C> s32 getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const C *names) {
    for (const C *p = names; *p; ++p) {
        if (*p == names[0]) ++n;
    }
    StdString counts;
    const u8 allocator = 0;   // (the original's byte is a stack byte nothing set: never read)
    StdString_CtorFill(&counts, nullptr, n, 0, &allocator);
    s32 answer = -2;
    for (u32 j = 1;; ++j) {
        bool prefix = false;
        for (u32 i = 0, k = 0; i < n; ++i) {
            while (names[k] && names[k] != names[0]) ++k;
            const char step = static_cast<char>(j < 0x7f ? j : 0x7f);
            if (at(counts, i)) {
                k += static_cast<s32>(at(counts, i));
            } else if (names[k += j] == names[0] || !names[k]) {
                at(counts, i) = step;
                answer = static_cast<s32>(i);
            } else if (equal(first, last, C()) || names[k] != deref(first, C())) {
                at(counts, i) = step;
            } else {
                prefix = true;
            }
        }
        if (!prefix || equal(first, last, C())) break;
        increment(first, C());
        answer = -1;
    }
    StdString_Tidy(&counts, nullptr, 1);
    return answer;
}

// _Getint: a sign, leading zeros as one, then up to 31 digits (more are read, not kept) to strtol; state 1 at the end, 2 for no number, errno or a
// value outside lo to hi (field kept).
template <typename C> u32 getint(NumGetIter &first, NumGetIter &last, s32 lo, s32 hi, s32 &field) {
    char text[32], *p = text;
    if (!equal(&first, &last, C())) {
        if (is<C>(first, '+')) {
            *p++ = '+';
            increment(&first, C());
        } else if (is<C>(first, '-')) {
            *p++ = '-';
            increment(&first, C());
        }
    }
    bool seen = false;
    while (!equal(&first, &last, C()) && is<C>(first, '0')) {
        seen = true;
        increment(&first, C());
    }
    if (seen) *p++ = '0';
    while (!equal(&first, &last, C())) {
        const char c = narrow(deref(&first, C()));
        *p = c;
        if (c < '0' || c > '9') break;
        if (p < text + 0x1f) ++p;
        seen = true;
        increment(&first, C());
    }
    if (!seen) p = text;
    *p = 0;
    *CRT_errno() = 0;
    const char *end;
    const s32 value = CRT_strtol(text, &end, 10);
    u32 state = equal(&first, &last, C()) ? 1 : 0;
    if (end == text || *CRT_errno() != 0 || value < lo || value > hi)
        state |= 2;
    else
        field = value;
    return state;
}

s32 &tm_field(u8 *tm, u32 offset) { return *reinterpret_cast<s32 *>(tm + offset); }

u32 *put_first(u32 *out, const NumGetIter &first) {
    const u32 *const from = reinterpret_cast<const u32 *>(&first);
    out[0] = from[0];
    out[1] = from[1];
    return out;
}

// The facet's own do_get_monthname (slot 5) or do_get_year (slot 6), by its vtable.
using Member = u32 *(__fastcall *)(u8 *, void *, u32 *, NumGetIter, NumGetIter, u8 *, u32 *, u8 *);
NumGetIter call_member(u8 *self, u32 slot, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm) {
    NumGetIter to;
    (*reinterpret_cast<Member *const *>(self))[slot](self, nullptr, reinterpret_cast<u32 *>(&to), first, last, ios, state, tm);
    return to;
}

template <typename C> u32 *get_time(u32 *out, NumGetIter &first, NumGetIter &last, u32 *state, u8 *tm) {
    const C colon = lit(C(), ':');   // (wchar_t: widened once, first)
    *state |= getint<C>(first, last, 0, 23, tm_field(tm, 8));
    if (*state != 0 || deref(&first, C()) != colon) {
        *state |= 2;
    } else {
        increment(&first, C());
        *state |= getint<C>(first, last, 0, 59, tm_field(tm, 4));
    }
    if (*state != 0 || deref(&first, C()) != colon) {
        *state |= 2;
    } else {
        increment(&first, C());
        *state |= getint<C>(first, last, 0, 59, tm_field(tm, 0));
    }
    return put_first(out, first);
}

template <typename C> u32 *get_date(u8 *self, u32 *out, NumGetIter &first, NumGetIter &last, u8 *ios, u32 *state, u8 *tm) {
    first = call_member(self, 5, first, last, ios, state, tm);
    if (*state != 0 || !is<C>(first, ' ')) {
        *state |= 2;
    } else {
        increment(&first, C());
        *state |= getint<C>(first, last, 1, 31, tm_field(tm, 0xc));
    }
    if (*state != 0 || !is<C>(first, ',')) {
        *state |= 2;
    } else {
        increment(&first, C());
        if (equal(&first, &last, C()) || !is<C>(first, ' ')) {
            *state |= 2;
        } else {
            increment(&first, C());
            first = call_member(self, 6, first, last, ios, state, tm);
        }
    }
    if (equal(&first, &last, C())) *state |= 1;
    return put_first(out, first);
}

// A year from 1900 on as years since it; 0 to 135 as they are.
template <typename C> u32 *get_year(u32 *out, NumGetIter &first, NumGetIter &last, u32 *state, u8 *tm) {
    s32 year = 0;
    *state |= getint<C>(first, last, 0, 2035, year);
    if (!(*state & 2)) {
        if (year >= 1900)
            year -= 1900;
        else if (year > 135)
            *state |= 2;
    }
    if (!(*state & 2)) tm_field(tm, 0x14) = year;
    return put_first(out, first);
}

template <typename C> u32 *get_name(u8 *self, u32 names_at, u32 *out, NumGetIter &first, NumGetIter &last, u32 *state, u8 *tm, u32 field) {
    const s32 index = getloctxt(&first, &last, 0, *reinterpret_cast<const C *const *>(self + names_at));
    if (index < 0) {
        *state |= 2;
    } else {
        tm_field(tm, field) = index >> 1;
    }
    return put_first(out, first);
}

}  // namespace

extern "C" {

s32 __cdecl TimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const char *names) { return getloctxt(first, last, n, names); }

s32 __cdecl WTimeGet_Getloctxt(NumGetIter *first, NumGetIter *last, u32 n, const u16 *names) { return getloctxt(first, last, n, names); }

u32 *__fastcall TimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<char>(self, 8, out, first, last, state, tm, 0x18); }

u32 *__fastcall TimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<char>(self, 0xc, out, first, last, state, tm, 0x10); }

u32 *__fastcall WTimeGet_DoGetWeekday(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<u16>(self, 8, out, first, last, state, tm, 0x18); }

u32 *__fastcall WTimeGet_DoGetMonthname(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_name<u16>(self, 0xc, out, first, last, state, tm, 0x10); }

u32 __cdecl TimeGet_Getint(NumGetIter *first, NumGetIter *last, s32 lo, s32 hi, s32 *field) { return getint<char>(*first, *last, lo, hi, *field); }

u32 *__fastcall TimeGet_DoGetTime(u8 *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_time<char>(out, first, last, state, tm); }

u32 *__fastcall TimeGet_DoGetDate(u8 *self, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *ios, u32 *state, u8 *tm) { return get_date<char>(self, out, first, last, ios, state, tm); }

u32 *__fastcall TimeGet_DoGetYear(u8 *, void *, u32 *out, NumGetIter first, NumGetIter last, u8 *, u32 *state, u8 *tm) { return get_year<char>(out, first, last, state, tm); }

}  // extern "C"
