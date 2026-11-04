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
#include "vobj3.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u32 kIntlId = 0x00fb1e80;
const u32 kIntlCache = 0x00fb1eb0;
const u32 kLocalId = 0x00fb1e90;
const u32 kLocalCache = 0x00fb1eac;
const u32 kCtypeId = 0x00fb1da4;
const u32 kCtypeCache = 0x00f827e0;

const unsigned long long kZeroBits = 0;
const unsigned long long kLimitBits = 0x4733426172c74d82;   // 1e35
const unsigned long long kStepBits = 0x3ddb7cdfd9d7bdbb;    // 1e-10

u32 npos() { return global<u32>(0x006fe5ac); }

const char *text_or_null_string(const char *text) { return text ? text : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0); }

u32 call_slot(const u32 *vtable, u32 offset, u8 *facet) { return reinterpret_cast<u32(__fastcall *)(u8 *)>((uintptr_t)vtable[offset / 4])(facet); }

void call_slot_out(const u32 *vtable, u32 offset, u8 *facet, void *out) { reinterpret_cast<void(__fastcall *)(u8 *, void *, void *)>((uintptr_t)vtable[offset / 4])(facet, nullptr, out); }

const u32 *vtable_of(const u8 *object) { return reinterpret_cast<const u32 *>((uintptr_t)*reinterpret_cast<const u32 *>(object)); }

void *moneypunct_ctor(u8 *facet, u32 refs, u8 international, u32 vtable) {
    u32 info[0x10];
    *reinterpret_cast<u32 *>(facet + 4) = refs;
    facet[0x28] = international;
    *reinterpret_cast<u32 *>(facet) = 0x00709fdc;
    Locinfo_Ctor(info, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c74c));
    MoneypunctC_Init(facet, nullptr, info);
    Locinfo_Dtor(info);
    *reinterpret_cast<u32 *>(facet) = vtable;
    return facet;
}

void throw_missing() {
    u32 bad[3];
    BadCast_Ctor(bad, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c734));
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

void put_times(u32 *it, u32 c, u32 count) {
    for (; count; --count) OstreamIter_Put(it, nullptr, c);
}

void put_text(u32 *it, const char *text, u32 count) {
    for (; count; --count) OstreamIter_Put(it, nullptr, static_cast<u8>(*text++));
}

void append_char(StdString *s, char c) {
    if (npos() - s->len <= 1) StdStrings_Xlen();
    const u32 grown = s->len + 1;
    if (StdString_Grow(s, nullptr, grown, 0)) {
        s->ptr[s->len] = c;
        s->len = grown;
        s->ptr[grown] = 0;
    }
}

// The separators into the integer part of value, from the right, by the group sizes.
void insert_groups(StdString *value, const char *groups, char separator, u32 fraction) {
    u32 at = value->len - fraction;
    while (*groups != 0x7f && static_cast<s8>(*groups) > 0) {
        const u32 size = static_cast<u32>(static_cast<s32>(static_cast<s8>(*groups)));
        if (size >= at) break;
        at -= size;
        if (value->len < at) StdStrings_Xran();
        if (npos() - value->len <= 1) StdStrings_Xlen();
        const u32 grown = value->len + 1;
        if (StdString_Grow(value, nullptr, grown, 0)) {
            CRT_memmove(value->ptr + at + 1, value->ptr + at, value->len - at);
            value->ptr[at] = separator;
            StdString_Eos(value, nullptr, grown);
        }
        if (static_cast<s8>(groups[1]) > 0) ++groups;
    }
}

// The digits with the decimal point before the last `fraction`, with zeros in front when there are too few.
void put_value(u32 *it, StdString *value, u8 *facet, const u32 *vtable, u32 fraction) {
    const u32 len = value->len;
    if (!fraction) {
        StdString_Freeze(value, nullptr);
        put_text(it, value->ptr, len);
    } else if (len <= fraction) {
        OstreamIter_Put(it, nullptr, '0');
        OstreamIter_Put(it, nullptr, static_cast<u8>(call_slot(vtable, 4, facet)));
        put_times(it, '0', fraction - len);
        StdString_Freeze(value, nullptr);
        put_text(it, value->ptr, len);
    } else {
        StdString_Freeze(value, nullptr);
        put_text(it, value->ptr, len - fraction);
        OstreamIter_Put(it, nullptr, static_cast<u8>(call_slot(vtable, 4, facet)));
        StdString_Freeze(value, nullptr);
        const char *const end = value->ptr ? value->ptr + value->len : nullptr;
        put_text(it, end - fraction, fraction);
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

}  // namespace

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

void *__fastcall MoneyPunctIntl_Ctor(u8 *facet, void *, u32 refs) { return moneypunct_ctor(facet, refs, 1, 0x00709f2c); }

void *__fastcall MoneyPunctLocal_Ctor(u8 *facet, void *, u32 refs) { return moneypunct_ctor(facet, refs, 0, 0x00709f5c); }

u32 *__fastcall MoneyPut_DoPutNumber(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, double value) {
    u8 negative;
    const u32 zeros = scale_value(&value, &negative);
    StdString text;
    StdString_Tidy(&text, nullptr, 0);
    char digits[40];
    const s32 count = CRT_sprintf(digits, reinterpret_cast<const char *>((uintptr_t)0x00709dcc), value);
    for (s32 i = 0; i < count; ++i) append_char(&text, digits[i]);
    if (npos() - text.len <= zeros) StdStrings_Xlen();
    if (zeros) {
        const u32 grown = text.len + zeros;
        if (StdString_Grow(&text, nullptr, grown, 0)) {
            std::memset(text.ptr + text.len, '0', zeros);
            text.len = grown;
            text.ptr[grown] = 0;
        }
    }
    StdString copy;
    copy.allocator = 0;
    copy.ptr = nullptr;
    copy.len = 0;
    copy.res = 0;
    StdString_AssignSub(&copy, nullptr, &text, 0, npos());
    MoneyPut_Putmfld(self, nullptr, out, failed, buffer, international, ios, fill, negative, copy.allocator, copy.ptr, copy.len, copy.res);
    StdString_Tidy(&text, nullptr, 1);
    return out;
}

u32 *__fastcall MoneyPut_DoPutString(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, const StdString *text) {
    u8 *const facet = use_facet(ios, kCtypeId, kCtypeCache, [] {
        void *const object = IosBase_Env().memAlloc(0x18);
        return Globalf827dc_Set(object ? static_cast<u8 *>(IosBase_Env().ctypeCtor(object, nullptr, 0, 0, 0)) : nullptr);
    });
    const char *from = text_or_null_string(text->ptr);
    u8 negative = 0;
    if (*from == '-') {
        negative = 1;
        ++from;
    }
    const u8 *const table = *reinterpret_cast<const u8 *const *>(facet + 0x10);
    u32 count = 0;
    while (table[static_cast<u32>(static_cast<u8>(from[count])) * 2] & 4) ++count;
    StdString digits;
    StdString_Tidy(&digits, nullptr, 0);
    if (StdString_Grow(&digits, nullptr, count, 1)) {
        std::memcpy(digits.ptr, from, count);
        StdString_Eos(&digits, nullptr, count);
    }
    if (!count) append_char(&digits, '0');
    StdString copy;
    StdString_Tidy(&copy, nullptr, 0);
    StdString_AssignSub(&copy, nullptr, &digits, 0, npos());
    MoneyPut_Putmfld(self, nullptr, out, failed, buffer, international, ios, fill, negative, copy.allocator, copy.ptr, copy.len, copy.res);
    StdString_Tidy(&digits, nullptr, 1);
    return out;
}

u32 *__fastcall MoneyPut_Putmfld(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 international, u8 *ios, u32 fill, u8 negative, u32 allocator, char *ptr, u32 len, u32 res) {
    StdString value;
    value.allocator = allocator;
    value.ptr = ptr;
    value.len = len;
    value.res = res;
    u8 *const facet = money_facet(ios, international);
    const u32 *const vtable = vtable_of(facet);
    const u32 fraction = call_slot(vtable, 0x1c, facet);
    StdString grouping;
    MoneyPunct_Grouping(facet, nullptr, &grouping);
    if (fraction < value.len && static_cast<s8>(*text_or_null_string(grouping.ptr)) > 0) {
        const char separator = static_cast<char>(call_slot(vtable, 8, facet));
        insert_groups(&value, text_or_null_string(grouping.ptr), separator, fraction);
    }
    StdString sign;
    StdString_Tidy(&sign, nullptr, 0);
    u8 pattern[4];
    call_slot_out(vtable, negative ? 0x24 : 0x20, facet, pattern);
    StdString held;
    if (negative) {
        MoneyPunct_NegativeSign(facet, nullptr, &held);
    } else {
        MoneyPunct_PositiveSign(facet, nullptr, &held);
    }
    StdString_AssignSub(&sign, nullptr, &held, 0, npos());
    StdString_Tidy(&held, nullptr, 1);
    StdString symbol;
    StdString_Tidy(&symbol, nullptr, 0);
    if (*reinterpret_cast<const u32 *>(ios + 0xc) & 8) {
        MoneyPunct_CurrSymbol(facet, nullptr, &held);
        StdString_AssignSub(&symbol, nullptr, &held, 0, npos());
        StdString_Tidy(&held, nullptr, 1);
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
        put_times(it, fill, padding);
        padding = 0;
    }
    for (u32 i = 0; i < 4; ++i) {
        switch (static_cast<s8>(pattern[i])) {
            case ' ':
                if (adjust == 0x100) {
                    put_times(it, fill, padding);
                    padding = 0;
                }
                break;
            case '$': {
                const u32 count = symbol.len;
                StdString_Freeze(&symbol, nullptr);
                put_text(it, symbol.ptr, count);
                break;
            }
            case '+':
                if (sign.len) {
                    StdString_Freeze(&sign, nullptr);
                    put_text(it, sign.ptr, 1);
                }
                break;
            case 'v':
                put_value(it, &value, facet, vtable, fraction);
                break;
            default:
                break;
        }
    }
    if (sign.len > 1) {
        const u32 count = sign.len;
        StdString_Freeze(&sign, nullptr);
        put_text(it, sign.ptr + 1, count - 1);
    }
    *reinterpret_cast<u32 *>(ios + 0x14) = 0;
    put_times(it, fill, padding);
    out[0] = it[0];
    out[1] = it[1];
    StdString_Tidy(&symbol, nullptr, 1);
    StdString_Tidy(&sign, nullptr, 1);
    StdString_Tidy(&grouping, nullptr, 1);
    StdString_Tidy(&value, nullptr, 1);
    return out;
}

}  // extern "C"
