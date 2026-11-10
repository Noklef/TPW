#include "moneyget.h"

#include <cstdint>

#include "crt.h"
#include "crtfloat.h"
#include "moneyput.h"
#include "streambufiter.h"

namespace {

// istreambuf_iterator<char>'s members and string's fills: the exe's own functions (streambufiter, stdstring).
using Iter = StreambufIter;

u32 npos() { return *reinterpret_cast<const u32 *>((uintptr_t)0x006fe5ac); }

char *null_string() { return reinterpret_cast<char *>((uintptr_t)0x006fe5b0); }

u32 slot(u8 *object, u32 offset) {
    const u32 *const vtable = reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(object));
    return reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)vtable[offset / 4])(object);
}

u8 value_of(Iter *it) { return *StreambufIter_Deref(it, nullptr); }

bool more(Iter *first, Iter *last) { return !StreambufIter_Equal(first, nullptr, last); }

void advance(Iter *it) { StreambufIter_Inc(it, nullptr); }

void append(StdString *s, u32 n, char c) { StdString_AppendFill(s, nullptr, n, static_cast<u32>(c)); }

void insert_minus(StdString *s) { StdString_InsertFill(s, nullptr, 0, 1, '-'); }

char *char_at(StdString *s, u32 at) { return StdString_At(s, nullptr, at); }

bool is(u8 *ctype, u8 c, u8 mask) { return (*reinterpret_cast<const u8 *const *>(ctype + 0x10))[static_cast<u32>(c) * 2] & mask; }

bool at_end_of(StdString *s, const char *p) {
    StdString_Freeze(s, nullptr);
    return p == (s->ptr ? s->ptr + s->len : nullptr);
}

void take_digits(Iter *first, Iter *last, u8 *ctype, StdString *value) {
    while (more(first, last) && is(ctype, value_of(first), 4)) {
        const u8 c = value_of(first);
        advance(first);
        append(value, 1, static_cast<char>(c));
    }
}

// The integer part with separators: counts holds the digits of each group. Returns whether the groups are bad by the grouping's sizes (the index goes down by 2 a round: sic).
bool take_groups(Iter *first, Iter *last, u8 *ctype, StdString *value, u8 *facet, const char *sizes) {
    const u8 separator = static_cast<u8>(slot(facet, 8));
    StdString counts;
    StdString_Tidy(&counts, nullptr, 0);
    StdString_AssignFill(&counts, nullptr, 1, 0);
    u32 group = 0;
    while (more(first, last)) {
        if (is(ctype, value_of(first), 4)) {
            append(value, 1, static_cast<char>(value_of(first)));
            char *const count = char_at(&counts, group);
            if (*count != 0x7f) ++*count;
        } else {
            if (!*char_at(&counts, group) || value_of(first) != separator) break;
            append(&counts, 1, 0);
            ++group;
        }
        advance(first);
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

bool is_decimal_point(u8 c, u8 *facet) { return c == static_cast<u8>(slot(facet, 4)); }

// The digits, the decimal point and the fraction, zeros added up to the facet's count. Returns whether the field is bad.
bool take_value(Iter *first, Iter *last, u8 *ctype, StdString *value, u8 *facet) {
    s32 count = 0;
    const s32 fraction = static_cast<s32>(slot(facet, 0x1c));
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    const char *const sizes = grouping.ptr ? grouping.ptr : null_string();
    bool bad = false;
    if (static_cast<s8>(*sizes) <= 0) {
        take_digits(first, last, ctype, value);
    } else {
        bad = take_groups(first, last, ctype, value, facet, sizes);
    }
    if (!bad) {
        if (more(first, last) && is_decimal_point(value_of(first), facet)) {
            advance(first);
            while (more(first, last) && is(ctype, value_of(first), 4)) {
                if (count < fraction) {
                    append(value, 1, static_cast<char>(value_of(first)));
                    ++count;
                }
                advance(first);
            }
        }
        if (!value->len) {
            bad = true;
        } else if (count < fraction) {
            for (s32 zeros = fraction - count; zeros; --zeros) append(value, 1, '0');
        }
    }
    StdString_Tidy(&grouping, nullptr, 1);
    return bad;
}

// The symbol, when it is required or the next character starts it. Returns whether it failed to match.
bool take_symbol(Iter *first, Iter *last, u8 *facet, u8 *ios, bool final_place, const StdString *sign) {
    StdString symbol;
    MoneyPunct_CurrSymbol(facet, nullptr, &symbol);
    if (!(*reinterpret_cast<const u32 *>(ios + 0xc) & 8)) {
        if ((final_place && !sign->len) || !more(first, last) || value_of(first) != static_cast<u8>(*(symbol.ptr ? symbol.ptr : null_string()))) {
            StdString_Erase(&symbol, nullptr, 0, npos());
        }
    }
    StdString_Freeze(&symbol, nullptr);
    const char *p = symbol.ptr;
    while (more(first, last) && !at_end_of(&symbol, p) && value_of(first) == static_cast<u8>(*p)) {
        ++p;
        advance(first);
    }
    const bool bad = !at_end_of(&symbol, p);
    StdString_Tidy(&symbol, nullptr, 1);
    return bad;
}

// Whether the next character starts the sign the method gives; if so, past it and the sign kept.
bool take_sign(Iter *first, u8 *facet, StdString *sign, StdString *(__fastcall *method)(u8 *, void *, StdString *)) {
    StdString held;
    method(facet, nullptr, &held);
    bool match = false;
    if (held.len) {
        StdString again;
        method(facet, nullptr, &again);
        match = static_cast<u8>(*char_at(&again, 0)) == value_of(first);
        StdString_Tidy(&again, nullptr, 1);
    }
    StdString_Tidy(&held, nullptr, 1);
    if (match) {
        advance(first);
        StdString whole;
        method(facet, nullptr, &whole);
        StdString_AssignSub(sign, nullptr, &whole, 0, npos());
        StdString_Tidy(&whole, nullptr, 1);
    }
    return match;
}

}  // namespace

extern "C" {

StdString *__fastcall MoneyGet_Getmfld(void *, void *, StdString *out, u8 *first_iterator, u8 *last_iterator, u8 international, u8 *ios) {
    Iter *const first = reinterpret_cast<Iter *>(first_iterator);
    Iter *const last = reinterpret_cast<Iter *>(last_iterator);
    u8 *const facet = MoneyPut_Facet(ios, international);
    u8 *const ctype = MoneyPut_Ctype(ios);
    bool bad = false;
    bool negative = false;
    StdString sign;
    StdString value;
    value.allocator = 0;
    StdString_Tidy(&sign, nullptr, 0);
    StdString_Tidy(&value, nullptr, 0);
    u8 pattern[4];
    const u32 *const vtable = reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(facet));
    reinterpret_cast<void(__fastcall *)(u8 *, void *, void *)>((uintptr_t)vtable[0x24 / 4])(facet, nullptr, pattern);
    for (u32 i = 0; i < 4 && !bad; ++i) {
        switch (static_cast<s8>(pattern[i])) {
            case '$':
                bad = take_symbol(first, last, facet, ios, i == 3, &sign);
                break;
            case '+':
                if (more(first, last) && !take_sign(first, facet, &sign, MoneyPunct_PositiveSign) && take_sign(first, facet, &sign, MoneyPunct_NegativeSign)) negative = true;
                if (sign.len == 1) StdString_Erase(&sign, nullptr, 0, npos());
                break;
            case 'v':
                bad = take_value(first, last, ctype, &value, facet);
                break;
            default:
                if (i != 3) {
                    while (more(first, last) && is(ctype, value_of(first), 0x48)) advance(first);
                }
                break;
        }
    }
    if (!bad && sign.len) {
        StdString_Freeze(&sign, nullptr);
        const char *p = sign.ptr;
        while (more(first, last)) {
            ++p;
            if (at_end_of(&sign, p) || value_of(first) != static_cast<u8>(*p)) break;
            advance(first);
        }
        if (!at_end_of(&sign, p)) bad = true;
    }
    if (bad) {
        StdString_Erase(&value, nullptr, 0, npos());
    } else if (negative) {
        insert_minus(&value);
    }
    *reinterpret_cast<u8 *>(out) = static_cast<u8>(value.allocator);
    StdString_Tidy(out, nullptr, 0);
    StdString_AssignSub(out, nullptr, &value, 0, npos());
    StdString_Tidy(&value, nullptr, 1);
    StdString_Tidy(&sign, nullptr, 1);
    return out;
}

u32 *__fastcall MoneyGet_DoGetLongDouble(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, double *value) {
    StdString digits;
    MoneyGet_Getmfld(self, nullptr, &digits, reinterpret_cast<u8 *>(&first), reinterpret_cast<u8 *>(&last), international, ios);
    if (!more(reinterpret_cast<Iter *>(&first), reinterpret_cast<Iter *>(&last))) *state |= 1;
    if (!digits.len) {
        *state |= 2;
    } else {
        StdString copy;
        StdString_Tidy(&copy, nullptr, 0);
        if (copy.res < digits.len) StdString_Grow(&copy, nullptr, digits.len, 0);
        for (u32 i = 0; i < digits.len; ++i) append(&copy, 1, *char_at(&digits, i));
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
    StdString_Tidy(&digits, nullptr, 1);
    return out;
}

u32 *__fastcall MoneyGet_DoGetString(void *self, void *, u32 *out, MoneyGetIter first, MoneyGetIter last, u8 international, u8 *ios, u32 *state, StdString *value) {
    StdString digits;
    MoneyGet_Getmfld(self, nullptr, &digits, reinterpret_cast<u8 *>(&first), reinterpret_cast<u8 *>(&last), international, ios);
    if (!more(reinterpret_cast<Iter *>(&first), reinterpret_cast<Iter *>(&last))) *state |= 1;
    if (!digits.len) {
        *state |= 2;
    } else {
        StdString_AssignSub(value, nullptr, &digits, 0, npos());
    }
    out[0] = first.word[0];
    out[1] = first.word[1];
    StdString_Tidy(&digits, nullptr, 1);
    return out;
}
}
