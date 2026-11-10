#include "moneyput.h"

#include <cstdint>
#include <cstring>

#include "buffers.h"
#include "crt.h"
#include "crtoutput.h"
#include "globals.h"
#include "iosbase.h"
#include "locinfo.h"
#include "moneypunct.h"
#include "sharedglobals.h"
#include "stdlocale.h"
#include "streambufiter.h"
#include "vobj3.h"
#include "localefacet.h"
#include "wstream.h"
#include "wtraits.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kIntlId = (u32)(uintptr_t)&gLocalefacet_fb1e80;
const u32 kIntlCache = (u32)(uintptr_t)&gLocalefacet_fb1eb0;
const u32 kLocalId = (u32)(uintptr_t)&gLocalefacet_fb1e90;
const u32 kLocalCache = (u32)(uintptr_t)&gLocalefacet_fb1eac;
const u32 kCtypeId = (u32)(uintptr_t)&gLocalefacet_fb1da4;
const u32 kCtypeCache = (u32)(uintptr_t)&gLocalefacet_f827e0;

const unsigned long long kZeroBits = 0;
const unsigned long long kLimitBits = 0x4733426172c74d82;   // 1e35
const unsigned long long kStepBits = 0x3ddb7cdfd9d7bdbb;    // 1e-10

u32 npos() { return global<u32>(0x006fe5ac); }

u32 call_slot(const u32 *vtable, u32 offset, u8 *facet) { return reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)vtable[offset / 4])(facet); }

void call_slot_out(const u32 *vtable, u32 offset, u8 *facet, void *out) { reinterpret_cast<void(__fastcall *)(u8 *, void *, void *)>((uintptr_t)vtable[offset / 4])(facet, nullptr, out); }

const u32 *vtable_of(const u8 *object) { return reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(object)); }

void *moneypunct_ctor(u8 *facet, u32 refs, u8 international, u32 base, void(__fastcall *init)(u8 *, void *, const void *), u32 vtable) {
    u32 info[0x10];
    *reinterpret_cast<u32 *>(facet + 4) = refs;
    facet[0x28] = international;
    *reinterpret_cast<u32 *>(facet) = base;
    Locinfo_Ctor(info, nullptr, gLocalefacet_C_75c74c);
    init(facet, nullptr, info);
    Locinfo_Dtor(info);
    *reinterpret_cast<u32 *>(facet) = vtable;
    return facet;
}

void throw_missing() {
    u32 bad[3];
    BadCast_Ctor(bad, nullptr, gLocalefacet_MissingFacet_75c734);
    IosBase_Env().cxxThrow(bad, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
}

// The facet of the stream's locale. A missing one comes from the classic locale: make() once, kept in the dword at cache; other locales throw.
template <typename Make> u8 *use_facet(u8 *ios, u32 id_variable, u32 cache, Make make) {
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    u32 lock[2];
    IosBase_Env().lockit(lock);
    u8 *facet = Locale_Getfacet(&locale, nullptr, Id_Get(&global<u32>(id_variable)), 1);
    if (!facet) {
        if (Locale_Iscloc(&locale)) {
            facet = global<u8 *>(cache);
            if (!facet) {
                facet = make();
                global<u8 *>(cache) = facet;
            }
        } else {
            throw_missing();
        }
    }
    IosBase_Env().unlockit(lock);
    CountedRef_Release(&locale);
    return facet;
}

u8 *money_facet(u8 *ios, u8 international) {
    if (international) {
        return use_facet(ios, kIntlId, kIntlCache, [] {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x2c));
            return Globalfb1eb4_Set(object ? static_cast<u8 *>(MoneyPunctIntl_Ctor(object, nullptr, 0)) : nullptr);
        });
    }
    return use_facet(ios, kLocalId, kLocalCache, [] {
        u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x2c));
        return Globalfb1eb8_Set(object ? static_cast<u8 *>(MoneyPunctLocal_Ctor(object, nullptr, 0)) : nullptr);
    });
}

u8 *ctype_facet(u8 *ios) {
    return use_facet(ios, kCtypeId, kCtypeCache, [] {
        void *const object = IosBase_Env().memAlloc(0x18);
        return Globalf827dc_Set(object ? static_cast<u8 *>(IosBase_Env().ctypeCtor(object, nullptr, 0, 0, 0)) : nullptr);
    });
}

// What differs between money_put<char> and money_put<wchar_t>: the units, their strings and iterator, the literals (wide: btowc's, at each use), the facets.
struct Narrow {
    using Unit = char;
    using String = StdString;
    static u32 npos() { return ::npos(); }
    static void xlen() { StdStrings_Xlen(); }
    static void xran() { StdStrings_Xran(); }
    static const Unit *text(const Unit *p) { return p ? p : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }
    static u32 lit(char c) { return static_cast<u8>(c); }
    static u32 unit(Unit c) { return static_cast<u8>(c); }
    static void put(u32 *it, u32 c) { OstreamIter_Put(it, nullptr, c); }
    static void freeze(String *s) { StdString_Freeze(s, nullptr); }
    static void tidy(String *s, u8 release) { StdString_Tidy(s, nullptr, release); }
    static void assign(String *s, const String *x) { StdString_AssignSub(s, nullptr, x, 0, npos()); }
    static bool grow(String *s, u32 n, u8 trim) { return StdString_Grow(s, nullptr, n, trim) != 0; }
    static void copy(Unit *to, const Unit *from, u32 n) { std::memcpy(to, from, n); }
    static void eos(String *s, u32 n) { StdString_Eos(s, nullptr, n); }
    static void insert(String *s, u32 at, u32 c) {
        CRT_memmove(s->ptr + at + 1, s->ptr + at, s->len - at);
        s->ptr[at] = static_cast<char>(c);
    }
    static u8 *facet(u8 *ios, u8 international) { return money_facet(ios, international); }
    static u8 *ctype(u8 *ios) { return ctype_facet(ios); }
    static bool digit(const u8 *ctype, Unit c) { return (*reinterpret_cast<const u8 *const *>(ctype + 0x10))[static_cast<u32>(static_cast<u8>(c)) * 2] & 4; }
    static u32 *putmfld(void *self, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, const String &s) {
        return MoneyPut_Putmfld(self, nullptr, out, failed, buffer, international, ios, fill, negative, s.allocator, s.ptr, s.len, s.res);
    }
};

struct Wide {
    using Unit = u16;
    using String = StdWString;
    static u32 npos() { return global<u32>(0x006fef00); }
    static void xlen() { Std_Xlen(); }
    static void xran() { Std_Xran(); }
    static const Unit *text(const Unit *p) { return p ? p : reinterpret_cast<const u16 *>((uintptr_t)0x006fd6c4); }
    static u32 lit(char c) { return CRT_btowc(static_cast<u32>(static_cast<s32>(c))); }
    // A unit from a string goes to the put in a register whose high half is stale (a call's answer, the iterator's address): only overflow sees it, and it takes a wint_t.
    static u32 unit(Unit c) { return c; }
    static void put(u32 *it, u32 c) { WOstreambufIter_Assign(reinterpret_cast<u8 *>(it), nullptr, c); }
    static void freeze(String *s) { StdWString_Freeze(s, nullptr); }
    static void tidy(String *s, u8 release) { CountedBuf_Reset(reinterpret_cast<u8 *>(s), nullptr, release); }
    static void assign(String *s, const String *x) { StdWString_AssignSub(s, nullptr, x, 0, npos()); }
    static bool grow(String *s, u32 n, u8 trim) { return StdWString_Grow(s, nullptr, n, trim) != 0; }
    static void copy(Unit *to, const Unit *from, u32 n) { WTraits_Copy(to, from, n); }
    static void eos(String *s, u32 n) { ImeWString_SetLength(reinterpret_cast<u8 *>(s), nullptr, n); }
    static void insert(String *s, u32 at, u32 c) {
        WTraits_Move(s->ptr + at + 1, s->ptr + at, s->len - at);
        WTraits_Assign(s->ptr + at, c, 1);
    }
    static u8 *facet(u8 *ios, u8 international) {
        if (international) {
            return use_facet(ios, (u32)(uintptr_t)&gLocalefacet_fb1de8, (u32)(uintptr_t)&gLocalefacet_fb1e2c, [] {
                u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x2c));
                return Globalfb1e34_Set(object ? static_cast<u8 *>(MoneyPunctIntlW_Ctor(object, nullptr, 0)) : nullptr);
            });
        }
        return use_facet(ios, (u32)(uintptr_t)&gLocalefacet_fb1e0c, (u32)(uintptr_t)&gLocalefacet_fb1e28, [] {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x2c));
            return Globalfb1e38_Set(object ? static_cast<u8 *>(MoneyPunctLocalW_Ctor(object, nullptr, 0)) : nullptr);
        });
    }
    static u8 *ctype(u8 *ios) {
        return use_facet(ios, (u32)(uintptr_t)&gLocalefacet_fb1df8, (u32)(uintptr_t)&gLocalefacet_7ced3c, [] {
            u8 *const object = static_cast<u8 *>(IosBase_Env().memAlloc(0x18));
            return Global7ced44_Set(object ? WCtype_Ctor(object, nullptr, 0) : nullptr);
        });
    }
    // ctype<wchar_t>::is(digit, c): its slot +8.
    static bool digit(u8 *ctype, Unit c) { return static_cast<u8>(reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32, u32)>((uintptr_t)vtable_of(ctype)[2])(ctype, nullptr, 4, c)) != 0; }
    static u32 *putmfld(void *self, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, const String &s) {
        return MoneyPutW_Putmfld(self, nullptr, out, failed, buffer, international, ios, fill, negative, s.allocator, s.ptr, s.len, s.res);
    }
};

template <typename C> void put_times(u32 *it, u32 c, u32 count) {
    for (; count; --count) C::put(it, c);
}

template <typename C> void put_text(u32 *it, const typename C::Unit *text, u32 count) {
    for (; count; --count) C::put(it, C::unit(*text++));
}

template <typename C> void append(typename C::String *s, u32 c) {
    if (C::npos() - s->len <= 1) C::xlen();
    const u32 grown = s->len + 1;
    if (C::grow(s, grown, 0)) {
        s->ptr[s->len] = static_cast<typename C::Unit>(c);
        s->len = grown;
        s->ptr[grown] = 0;
    }
}

// The separators into the integer part of value, from the right, by the group sizes.
template <typename C> void insert_groups(typename C::String *value, const char *groups, u32 separator, u32 fraction) {
    u32 at = value->len - fraction;
    while (*groups != 0x7f && static_cast<s8>(*groups) > 0) {
        const u32 size = static_cast<u32>(static_cast<s32>(static_cast<s8>(*groups)));
        if (size >= at) break;
        at -= size;
        if (value->len < at) C::xran();
        if (C::npos() - value->len <= 1) C::xlen();
        const u32 grown = value->len + 1;
        if (C::grow(value, grown, 0)) {
            C::insert(value, at, separator);
            C::eos(value, grown);
        }
        if (static_cast<s8>(groups[1]) > 0) ++groups;
    }
}

// The digits with the decimal point before the last `fraction`, with zeros in front when there are too few.
template <typename C> void put_value(u32 *it, typename C::String *value, u8 *facet, const u32 *vtable, u32 fraction) {
    const u32 len = value->len;
    if (!fraction) {
        C::freeze(value);
        put_text<C>(it, value->ptr, len);
    } else if (len <= fraction) {
        C::put(it, C::lit('0'));
        C::put(it, call_slot(vtable, 4, facet));
        put_times<C>(it, C::lit('0'), fraction - len);
        C::freeze(value);
        put_text<C>(it, value->ptr, len);
    } else {
        C::freeze(value);
        put_text<C>(it, value->ptr, len - fraction);
        C::put(it, call_slot(vtable, 4, facet));
        C::freeze(value);
        const typename C::Unit *const end = value->ptr ? value->ptr + value->len : nullptr;
        put_text<C>(it, end - fraction, fraction);
    }
}

// The original's x87 code, so that it rounds as the game's control word says: a value below zero (or a NaN) has its sign changed, written back, and sets negative;
// a value of 1e35 or more (or a NaN) goes down by 1e-10 until it is below, for at most 5000 zeros, and is written back. Returns the zeros.
u32 scale_value(double *value, u8 *negative) {
    u32 zeros = 0;
    u8 flag = 0;
    __asm {
        mov ecx, value
        fld qword ptr [ecx]
        fcom qword ptr [kZeroBits]
        fnstsw ax
        test ah, 1
        jz positive
        fchs
        fst qword ptr [ecx]
        mov flag, 1
    positive:
        fld qword ptr [kLimitBits]
        fcomp st(1)
        fnstsw ax
        test ah, 0x41
        jz below_limit
    again:
        cmp zeros, 0x1388
        jae write_back
        fmul qword ptr [kStepBits]
        fld qword ptr [kLimitBits]
        fcomp st(1)
        add zeros, 10
        fnstsw ax
        test ah, 0x41
        jnz again
    write_back:
        fstp qword ptr [ecx]
        jmp finished
    below_limit:
        fstp st(0)
    finished:
    }
    *negative = flag;
    return zeros;
}

template <typename C> u32 *do_put_number(void *self, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, double value) {
    u8 negative;
    const u32 zeros = scale_value(&value, &negative);
    typename C::String text;
    C::tidy(&text, 0);
    char digits[40];
    const s32 count = CRT_sprintf(digits, reinterpret_cast<const char *>((uintptr_t)0x00709dcc), value);
    for (s32 i = 0; i < count; ++i) append<C>(&text, C::lit(digits[i]));
    const auto zero = static_cast<typename C::Unit>(C::lit('0'));
    if (C::npos() - text.len <= zeros) C::xlen();
    if (zeros) {
        const u32 grown = text.len + zeros;
        if (C::grow(&text, grown, 0)) {
            for (u32 i = 0; i < zeros; ++i) text.ptr[text.len + i] = zero;
            text.len = grown;
            text.ptr[grown] = 0;
        }
    }
    typename C::String copy;
    copy.allocator = 0;
    copy.ptr = nullptr;
    copy.len = 0;
    copy.res = 0;
    C::assign(&copy, &text);
    C::putmfld(self, out, failed, buffer, international, ios, fill, negative, copy);
    C::tidy(&text, 1);
    return out;
}

template <typename C> u32 *do_put_string(void *self, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, const typename C::String *text) {
    u8 *const facet = C::ctype(ios);
    const typename C::Unit *from = C::text(text->ptr);
    u8 negative = 0;
    if (*from == static_cast<typename C::Unit>(C::lit('-'))) {
        negative = 1;
        ++from;
    }
    u32 count = 0;
    while (C::digit(facet, from[count])) ++count;
    typename C::String digits;
    C::tidy(&digits, 0);
    if (C::grow(&digits, count, 1)) {
        C::copy(digits.ptr, from, count);
        C::eos(&digits, count);
    }
    if (!count) append<C>(&digits, C::lit('0'));
    typename C::String copy;
    C::tidy(&copy, 0);
    C::assign(&copy, &digits);
    C::putmfld(self, out, failed, buffer, international, ios, fill, negative, copy);
    C::tidy(&digits, 1);
    return out;
}

template <typename C> u32 *putmfld(u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, typename C::String &value) {
    u8 *const facet = C::facet(ios, international);
    const u32 *const vtable = vtable_of(facet);
    const u32 fraction = call_slot(vtable, 0x1c, facet);
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    if (fraction < value.len && static_cast<s8>(*Narrow::text(grouping.ptr)) > 0) {
        const u32 separator = call_slot(vtable, 8, facet);
        insert_groups<C>(&value, Narrow::text(grouping.ptr), separator, fraction);
    }
    typename C::String sign;
    C::tidy(&sign, 0);
    u8 pattern[4];
    call_slot_out(vtable, negative ? 0x24 : 0x20, facet, pattern);
    typename C::String held;
    StdString *const narrow = reinterpret_cast<StdString *>(&held);   // (the slots write a string of the facet's units)
    if (negative) {
        MoneyPunct_NegativeSign(facet, nullptr, narrow);
    } else {
        MoneyPunct_PositiveSign(facet, nullptr, narrow);
    }
    C::assign(&sign, &held);
    C::tidy(&held, 1);
    typename C::String symbol;
    C::tidy(&symbol, 0);
    if (*reinterpret_cast<const u32 *>(ios + 0xc) & 8) {
        MoneyPunct_CurrSymbol(facet, nullptr, narrow);
        C::assign(&symbol, &held);
        C::tidy(&held, 1);
    }
    bool spaced = false;
    u32 total = 0;
    for (u32 i = 0; i < 4; ++i) {
        switch (static_cast<s8>(pattern[i])) {
            case ' ':
                spaced = true;
                break;
            case '$':
                total += symbol.len;
                break;
            case '+':
                total += sign.len;
                break;
            case 'v':
                total += (fraction ? 1 : 0) + value.len + (value.len <= fraction ? fraction - value.len + 1 : 0);
                break;
            default:
                break;
        }
    }
    const s32 width = *reinterpret_cast<const s32 *>(ios + 0x14);
    u32 padding = width > 0 && static_cast<u32>(width) > total ? static_cast<u32>(width) - total : 0;
    const u32 adjust = *reinterpret_cast<const u32 *>(ios + 0xc) & 0x1c0;
    u32 it[2] = {failed, static_cast<u32>(reinterpret_cast<uintptr_t>(buffer))};
    if (adjust != 0x40 && (adjust != 0x100 || !spaced)) {
        put_times<C>(it, fill, padding);
        padding = 0;
    }
    for (u32 i = 0; i < 4; ++i) {
        switch (static_cast<s8>(pattern[i])) {
            case ' ':
                if (adjust == 0x100) {
                    put_times<C>(it, fill, padding);
                    padding = 0;
                }
                break;
            case '$': {
                const u32 count = symbol.len;
                C::freeze(&symbol);
                put_text<C>(it, symbol.ptr, count);
                break;
            }
            case '+':
                if (sign.len) {
                    C::freeze(&sign);
                    put_text<C>(it, sign.ptr, 1);
                }
                break;
            case 'v':
                put_value<C>(it, &value, facet, vtable, fraction);
                break;
            default:
                break;
        }
    }
    if (sign.len > 1) {
        const u32 count = sign.len;
        C::freeze(&sign);
        put_text<C>(it, sign.ptr + 1, count - 1);
    }
    *reinterpret_cast<u32 *>(ios + 0x14) = 0;
    put_times<C>(it, fill, padding);
    out[0] = it[0];
    out[1] = it[1];
    C::tidy(&symbol, 1);
    C::tidy(&sign, 1);
    StdString_Tidy(&grouping, nullptr, 1);
    C::tidy(&value, 1);
    return out;
}

}  // namespace

u8 *MoneyPut_Facet(u8 *ios, u8 international) { return money_facet(ios, international); }

u8 *MoneyPut_Ctype(u8 *ios) { return ctype_facet(ios); }

extern "C" {

u32 __fastcall Streambuf_Sputc(u8 *buffer, void *, u32 c) {
    const u32 at = **reinterpret_cast<u32 **>(buffer + 0x20);
    if (at) {
        u32 *const count = *reinterpret_cast<u32 **>(buffer + 0x30);
        const u32 left = *count;
        if (at < at + left) {
            *count = left - 1;
            u32 *const pointer = *reinterpret_cast<u32 **>(buffer + 0x20);
            const u32 place = *pointer;
            *pointer = place + 1;
            *reinterpret_cast<u8 *>((uintptr_t)place) = static_cast<u8>(c);
            return c & 0xff;
        }
    }
    return reinterpret_cast<u32(__fastcall *)(u8 *, void *, u32)>((uintptr_t)vtable_of(buffer)[1])(buffer, nullptr, c & 0xff);
}

u32 *__fastcall OstreamIter_Put(u32 *it, void *, u32 c) {
    u8 *const buffer = reinterpret_cast<u8 *>((uintptr_t)it[1]);
    if (!buffer || Streambuf_Sputc(buffer, nullptr, c) == 0xffffffff) *reinterpret_cast<u8 *>(it) = 1;
    return it;
}

StdString *__fastcall MoneyPunct_Grouping(u8 *facet, void *, StdString *out) {
    call_slot_out(vtable_of(facet), 0xc, facet, out);
    return out;
}

StdString *__fastcall MoneyPunct_PositiveSign(u8 *facet, void *, StdString *out) {
    call_slot_out(vtable_of(facet), 0x14, facet, out);
    return out;
}

StdString *__fastcall MoneyPunct_CurrSymbol(u8 *facet, void *, StdString *out) {
    call_slot_out(vtable_of(facet), 0x10, facet, out);
    return out;
}

StdString *__fastcall MoneyPunct_NegativeSign(u8 *facet, void *, StdString *out) {
    call_slot_out(vtable_of(facet), 0x18, facet, out);
    return out;
}

u32 *__fastcall MoneyPunct_MakePattern(const u8 *facet, void *, u32 *out, u8 space, u8 precedes, u8 position) {
    const s8 place = static_cast<s8>(position);
    u32 at = 0x00709d10;
    if (!facet[0x28] && !(space & 0xfe) && !(precedes & 0xfe) && place >= 0 && place <= 4) {
        at = 0x00709d18 + (space == 1 ? 0x28 : 0) + (precedes == 1 ? 0x14 : 0) + static_cast<u32>(place);
    }
    *out = global<u32>(at);
    return out;
}

void *__fastcall MoneyPunctIntl_Ctor(u8 *facet, void *, u32 refs) {
    return moneypunct_ctor(facet, refs, 1, (u32)(uintptr_t)MoneypunctC_vtable, MoneypunctC_Init, (u32)(uintptr_t)Class709f2c_vtable);
}

void *__fastcall MoneyPunctLocal_Ctor(u8 *facet, void *, u32 refs) {
    return moneypunct_ctor(facet, refs, 0, (u32)(uintptr_t)MoneypunctC_vtable, MoneypunctC_Init, (u32)(uintptr_t)Class709f5c_vtable);
}

void *__fastcall MoneyPunctIntlW_Ctor(u8 *facet, void *, u32 refs) {
    return moneypunct_ctor(facet, refs, 1, (u32)(uintptr_t)MoneypunctW_vtable, MoneypunctW_Init, (u32)(uintptr_t)Class709da4_vtable);
}

void *__fastcall MoneyPunctLocalW_Ctor(u8 *facet, void *, u32 refs) {
    return moneypunct_ctor(facet, refs, 0, (u32)(uintptr_t)MoneypunctW_vtable, MoneypunctW_Init, (u32)(uintptr_t)Class709bbc_vtable);
}

u32 *__fastcall MoneyPut_DoPutNumber(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, double value) {
    return do_put_number<Narrow>(self, out, failed, buffer, international, ios, fill, value);
}

u32 *__fastcall MoneyPut_DoPutString(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, const StdString *text) {
    return do_put_string<Narrow>(self, out, failed, buffer, international, ios, fill, text);
}

u32 *__fastcall MoneyPut_Putmfld(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, u32 allocator, char *ptr, u32 len, u32 res) {
    StdString value;
    value.allocator = allocator;
    value.ptr = ptr;
    value.len = len;
    value.res = res;
    return putmfld<Narrow>(out, failed, buffer, international, ios, fill, negative, value);
}

u32 *__fastcall MoneyPutW_DoPutNumber(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, double value) {
    return do_put_number<Wide>(self, out, failed, buffer, international, ios, fill, value);
}

u32 *__fastcall MoneyPutW_DoPutString(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, const StdWString *text) {
    return do_put_string<Wide>(self, out, failed, buffer, international, ios, fill, text);
}

u32 *__fastcall MoneyPutW_Putmfld(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, u32 allocator, u16 *ptr, u32 len, u32 res) {
    StdWString value;
    value.allocator = allocator;
    value.ptr = ptr;
    value.len = len;
    value.res = res;
    return putmfld<Wide>(out, failed, buffer, international, ios, fill, negative, value);
}

}  // extern "C"
