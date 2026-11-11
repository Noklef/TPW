#include "moneyget.h"

#include <cstdint>

#include "buffers.h"
#include "crt.h"
#include "crtfloat.h"
#include "moneyput.h"
#include "streambufiter.h"

namespace {

u32 slot(u8 *object, u32 offset) {
    const u32 *const vtable = reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(object));
    return reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)vtable[offset / 4])(object);
}

char *null_string() { return reinterpret_cast<char *>((uintptr_t)0x006fe5b0); }

// The digit counts of the groups: a string of char for either unit.
void narrow_append(StdString *s, char c) { StdString_AppendFill(s, nullptr, 1, static_cast<u32>(c)); }

char *char_at(StdString *s, u32 at) { return StdString_At(s, nullptr, at); }

// What differs between money_get<char> and money_get<wchar_t>: the units, their strings and iterator (the exe's own, streambufiter, stdstring, stdwstring), the literals (wide:
// btowc's, at each use), ctype's is() (wide: its slot +8), the facets, and how do_get narrows the digits for _Stold (wide: wctob).
struct Narrow {
    using Unit = u8;
    using Char = char;
    using String = StdString;
    using Iter = StreambufIter;
    static u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac); }
    static const Char *text(const Char *p) { return p ? p : null_string(); }
    static Unit unit(Char c) { return static_cast<u8>(c); }
    static Unit value_of(Iter *it) { return *StreambufIter_Deref(it, nullptr); }
    static bool more(Iter *first, Iter *last) { return !StreambufIter_Equal(first, nullptr, last); }
    static void advance(Iter *it) { StreambufIter_Inc(it, nullptr); }
    static void append(String *s, u32 c) { StdString_AppendFill(s, nullptr, 1, static_cast<u32>(static_cast<char>(c))); }
    static u32 zero() { return '0'; }
    static void insert_minus(String *s) { StdString_InsertFill(s, nullptr, 0, 1, '-'); }
    static void erase(String *s) { StdString_Erase(s, nullptr, 0, npos()); }
    static Char *at(String *s, u32 i) { return StdString_At(s, nullptr, i); }
    static void freeze(String *s) { StdString_Freeze(s, nullptr); }
    static void tidy(String *s, u8 release) { StdString_Tidy(s, nullptr, release); }
    static void assign(String *s, const String *x) { StdString_AssignSub(s, nullptr, x, 0, npos()); }
    static bool is(u8 *ctype, Unit c, u8 mask) { return (*reinterpret_cast<const u8 *const *>(ctype + 0x10))[static_cast<u32>(c) * 2] & mask; }
    static char narrow(Char c) { return c; }
    static u8 *facet(u8 *ios, u8 international) { return MoneyPut_Facet(ios, international); }
    static u8 *ctype(u8 *ios) { return MoneyPut_Ctype(ios); }
};

struct Wide {
    using Unit = u16;
    using Char = u16;
    using String = StdWString;
    using Iter = WStreambufIter;
    static u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fef00); }
    static const Char *text(const Char *p) { return p ? p : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4); }
    static Unit unit(Char c) { return c; }
    static Unit value_of(Iter *it) { return *WStreambufIter_Deref(it, nullptr); }
    static bool more(Iter *first, Iter *last) { return !WStreambufIter_Equal(first, nullptr, last); }
    static void advance(Iter *it) { WStreambufIter_Inc(it, nullptr); }
    static void append(String *s, u32 c) { StdWString_AppendFill(s, nullptr, 1, c); }
    static u32 zero() { return CRT_btowc('0'); }
    static void insert_minus(String *s) { StdWString_InsertFill(s, nullptr, 0, 1, CRT_btowc('-')); }
    static void erase(String *s) { StdWString_Erase(s, nullptr, 0, npos()); }
    // operator[] (inlined in the exe): the empty string's unit past the length or with no block; else the block made its own first.
    static Char *at(String *s, u32 i) {
        if (s->len < i || !s->ptr) return const_cast<Char *>(text(nullptr));
        StdWString_Freeze(s, nullptr);
        return s->ptr + i;
    }
    static void freeze(String *s) { StdWString_Freeze(s, nullptr); }
    static void tidy(String *s, u8 release) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }
    static void assign(String *s, const String *x) { StdWString_AssignSub(s, nullptr, x, 0, npos()); }
    // ctype<wchar_t>::is(mask, c): its slot +8.
    static bool is(u8 *ctype, Unit c, u8 mask) {
        const u32 *const vtable = reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(ctype));
        return static_cast<u8>(reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)vtable[2])(ctype, nullptr, mask, c)) != 0;
    }
    static char narrow(Char c) { return static_cast<char>(CRT_Wctob(c)); }
    static u8 *facet(u8 *ios, u8 international) { return MoneyPutW_Facet(ios, international); }
    static u8 *ctype(u8 *ios) { return MoneyPutW_Ctype(ios); }
};

template <typename C> bool at_end_of(typename C::String *s, const typename C::Char *p) {
    C::freeze(s);
    return p == (s->ptr ? s->ptr + s->len : nullptr);
}

template <typename C> void take_digits(typename C::Iter *first, typename C::Iter *last, u8 *ctype, typename C::String *value) {
    while (C::more(first, last) && C::is(ctype, C::value_of(first), 4)) {
        const typename C::Unit c = C::value_of(first);
        C::advance(first);
        C::append(value, c);
    }
}

// The integer part with separators: counts holds the digits of each group. Returns whether the groups are bad by the grouping's sizes (the index goes down by 2 a round: sic).
template <typename C> bool take_groups(typename C::Iter *first, typename C::Iter *last, u8 *ctype, typename C::String *value, u8 *facet, const char *sizes) {
    const typename C::Unit separator = static_cast<typename C::Unit>(slot(facet, 8));
    StdString counts;
    StdString_Tidy(&counts, nullptr, 0);
    StdString_AssignFill(&counts, nullptr, 1, 0);
    u32 group = 0;
    while (C::more(first, last)) {
        if (C::is(ctype, C::value_of(first), 4)) {
            C::append(value, C::value_of(first));
            char *const count = char_at(&counts, group);
            if (*count != 0x7f) ++*count;
        } else {
            if (!*char_at(&counts, group) || C::value_of(first) != separator) break;
            narrow_append(&counts, 0);
            ++group;
        }
        C::advance(first);
    }
    bool bad = false;
    if (group) {
        if (static_cast<s8>(*char_at(&counts, group)) > 0) {
            ++group;
        } else {
            bad = true;
        }
    }
    while (!bad && group && *sizes != 0x7f) {
        --group;
        const char count = *char_at(&counts, group);
        if (group ? *sizes != count : static_cast<s8>(*sizes) < static_cast<s8>(count)) {
            bad = true;
        } else if (static_cast<s8>(sizes[1]) > 0) {
            ++sizes;
        }
        --group;
    }
    StdString_Tidy(&counts, nullptr, 1);
    return bad;
}

// The digits, the decimal point and the fraction, zeros added up to the facet's count. Returns whether the field is bad.
template <typename C> bool take_value(typename C::Iter *first, typename C::Iter *last, u8 *ctype, typename C::String *value, u8 *facet) {
    s32 count = 0;
    const s32 fraction = static_cast<s32>(slot(facet, 0x1c));
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    const char *const sizes = grouping.ptr ? grouping.ptr : null_string();
    bool bad = false;
    if (static_cast<s8>(*sizes) <= 0) {
        take_digits<C>(first, last, ctype, value);
    } else {
        bad = take_groups<C>(first, last, ctype, value, facet, sizes);
    }
    if (!bad) {
        if (C::more(first, last)) {
            const typename C::Unit c = C::value_of(first);
            if (c == static_cast<typename C::Unit>(slot(facet, 4))) {
                C::advance(first);
                while (C::more(first, last) && C::is(ctype, C::value_of(first), 4)) {
                    if (count < fraction) {
                        C::append(value, C::value_of(first));
                        ++count;
                    }
                    C::advance(first);
                }
            }
        }
        if (!value->len) {
            bad = true;
        } else if (count < fraction) {
            for (s32 zeros = fraction - count; zeros; --zeros) C::append(value, C::zero());
        }
    }
    StdString_Tidy(&grouping, nullptr, 1);
    return bad;
}

// The symbol, when it is required or the next character starts it. Returns whether it failed to match. (The facet's slots write a string of its units.)
template <typename C> bool take_symbol(typename C::Iter *first, typename C::Iter *last, u8 *facet, u8 *ios, bool final_place, const typename C::String *sign) {
    typename C::String symbol;
    MoneyPunct_CurrSymbol(facet, nullptr, reinterpret_cast<StdString *>(&symbol));
    if (!(*reinterpret_cast<const u32 *>(ios + 0xc) & 8)) {
        if ((final_place && !sign->len) || !C::more(first, last) || C::value_of(first) != C::unit(*C::text(symbol.ptr))) C::erase(&symbol);
    }
    C::freeze(&symbol);
    const typename C::Char *p = symbol.ptr;
    while (C::more(first, last) && !at_end_of<C>(&symbol, p) && C::value_of(first) == C::unit(*p)) {
        ++p;
        C::advance(first);
    }
    const bool bad = !at_end_of<C>(&symbol, p);
    C::tidy(&symbol, 1);
    return bad;
}

// Whether the next character starts the sign the method gives; if so, past it and the sign kept.
template <typename C> bool take_sign(typename C::Iter *first, u8 *facet, typename C::String *sign, StdString *(__fastcall *method)(u8 *, void *, StdString *)) {
    typename C::String held;
    method(facet, nullptr, reinterpret_cast<StdString *>(&held));
    bool match = false;
    if (held.len) {
        typename C::String again;
        method(facet, nullptr, reinterpret_cast<StdString *>(&again));
        const typename C::Unit c = C::unit(*C::at(&again, 0));
        match = c == C::value_of(first);
        C::tidy(&again, 1);
    }
    C::tidy(&held, 1);
    if (match) {
        C::advance(first);
        typename C::String whole;
        method(facet, nullptr, reinterpret_cast<StdString *>(&whole));
        C::assign(sign, &whole);
        C::tidy(&whole, 1);
    }
    return match;
}

template <typename C> typename C::String *getmfld(typename C::String *out, u8 *first_iterator, u8 *last_iterator, u8 international, u8 *ios) {
    using Iter = typename C::Iter;
    Iter *const first = reinterpret_cast<Iter *>(first_iterator);
    Iter *const last = reinterpret_cast<Iter *>(last_iterator);
    u8 *const facet = C::facet(ios, international);
    u8 *const ctype = C::ctype(ios);
    bool bad = false;
    bool negative = false;
    typename C::String sign;
    typename C::String value;
    value.allocator = 0;
    C::tidy(&sign, 0);
    C::tidy(&value, 0);
    u8 pattern[4];
    const u32 *const vtable = reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(facet));
    reinterpret_cast<void(__fastcall *)(u8 *, void *, void *)>((uintptr_t)vtable[0x24 / 4])(facet, nullptr, pattern);
    for (u32 i = 0; i < 4 && !bad; ++i) {
        switch (static_cast<s8>(pattern[i])) {
            case '$':
                bad = take_symbol<C>(first, last, facet, ios, i == 3, &sign);
                break;
            case '+':
                if (C::more(first, last) && !take_sign<C>(first, facet, &sign, MoneyPunct_PositiveSign) && take_sign<C>(first, facet, &sign, MoneyPunct_NegativeSign)) negative = true;
                if (sign.len == 1) C::erase(&sign);
                break;
            case 'v':
                bad = take_value<C>(first, last, ctype, &value, facet);
                break;
            default:
                if (i != 3) {
                    while (C::more(first, last) && C::is(ctype, C::value_of(first), 0x48)) C::advance(first);
                }
                break;
        }
    }
    if (!bad && sign.len) {
        C::freeze(&sign);
        const typename C::Char *p = sign.ptr;
        while (C::more(first, last)) {
            ++p;
            if (at_end_of<C>(&sign, p) || C::value_of(first) != C::unit(*p)) break;
            C::advance(first);
        }
        if (!at_end_of<C>(&sign, p)) bad = true;
    }
    if (bad) {
        C::erase(&value);
    } else if (negative) {
        C::insert_minus(&value);
    }
    *reinterpret_cast<u8 *>(out) = static_cast<u8>(value.allocator);
    C::tidy(out, 0);
    C::assign(out, &value);
    C::tidy(&value, 1);
    C::tidy(&sign, 1);
    return out;
}

template <typename C> u32 *do_get_long_double(u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value) {
    using Iter = typename C::Iter;
    typename C::String digits;
    getmfld<C>(&digits, reinterpret_cast<u8 *>(&first), reinterpret_cast<u8 *>(&last), international, ios);
    if (!C::more(reinterpret_cast<Iter *>(&first), reinterpret_cast<Iter *>(&last))) *state |= 1;
    if (!digits.len) {
        *state |= 2;
    } else {
        StdString copy;
        StdString_Tidy(&copy, nullptr, 0);
        if (copy.res < digits.len) StdString_Grow(&copy, nullptr, digits.len, 0);
        for (u32 i = 0; i < digits.len; ++i) narrow_append(&copy, C::narrow(*C::at(&digits, i)));
        const char *const text = copy.ptr ? copy.ptr : null_string();
        *CRT_errno() = 0;
        const char *end;
        const double number = CRT_Stold(text, &end, 0);
        if (end != text && !*CRT_errno()) {
            *value = number;
        } else {
            *state |= 2;
        }
        StdString_Tidy(&copy, nullptr, 1);
    }
    out[0] = first.word[0];
    out[1] = first.word[1];
    C::tidy(&digits, 1);
    return out;
}

template <typename C> u32 *do_get_string(u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, typename C::String *value) {
    using Iter = typename C::Iter;
    typename C::String digits;
    getmfld<C>(&digits, reinterpret_cast<u8 *>(&first), reinterpret_cast<u8 *>(&last), international, ios);
    if (!C::more(reinterpret_cast<Iter *>(&first), reinterpret_cast<Iter *>(&last))) *state |= 1;
    if (!digits.len) {
        *state |= 2;
    } else {
        C::assign(value, &digits);
    }
    out[0] = first.word[0];
    out[1] = first.word[1];
    C::tidy(&digits, 1);
    return out;
}

}  // namespace

extern "C" {

StdString *__fastcall MoneyGet_Getmfld(void *, void *, StdString *out, u8 *first, u8 *last, u8 international, u8 *ios) { return getmfld<Narrow>(out, first, last, international, ios); }

u32 *__fastcall MoneyGet_DoGetLongDouble(void *, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value) {
    return do_get_long_double<Narrow>(out, first, last, international, ios, state, value);
}

u32 *__fastcall MoneyGet_DoGetString(void *, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, StdString *value) {
    return do_get_string<Narrow>(out, first, last, international, ios, state, value);
}

StdWString *__fastcall MoneyGetW_Getmfld(void *, void *, StdWString *out, u8 *first, u8 *last, u8 international, u8 *ios) { return getmfld<Wide>(out, first, last, international, ios); }

u32 *__fastcall MoneyGetW_DoGetLongDouble(void *, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value) {
    return do_get_long_double<Wide>(out, first, last, international, ios, state, value);
}

u32 *__fastcall MoneyGetW_DoGetString(void *, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, StdWString *value) {
    return do_get_string<Wide>(out, first, last, international, ios, state, value);
}
}
