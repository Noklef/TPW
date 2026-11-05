#include "numput.h"

#include <cstdint>

#include "buffers.h"
#include "crt.h"
#include "crtlocale.h"
#include "crtoutput.h"
#include "globals.h"
#include "iosbase.h"
#include "moneyput.h"
#include "sharedglobals.h"
#include "stdlocale.h"
#include "stdstring.h"
#include "vobj3.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

// The put area's pointer (through +0x20) and its count (through +0x30), as streambuf::sputc reads them. The vtable's slot 1 is overflow.
u32 overflow(u8 *buffer, u32 c) {
    using Overflow = u32(__fastcall *)(u8 *, void *, u32);
    return reinterpret_cast<const Overflow *>((uintptr_t)at<u32>(buffer, 0))[1](buffer, nullptr, c);
}

void put_narrow(u32 &failed, u8 *buffer, u32 c) {
    if (buffer) {
        u32 *const pointer = at<u32 *>(buffer, 0x20);
        const u32 p = *pointer;
        u32 result;
        if (p) {
            s32 *const count = at<s32 *>(buffer, 0x30);
            const u32 left = static_cast<u32>(*count);
            if (p < p + left) {
                *count = static_cast<s32>(left - 1);
                *at<u32 *>(buffer, 0x20) = p + 1;
                *reinterpret_cast<u8 *>((uintptr_t)p) = static_cast<u8>(c);
                result = c & 0xff;
            } else {
                result = overflow(buffer, c & 0xff);
            }
        } else {
            result = overflow(buffer, c & 0xff);
        }
        if (result != 0xffffffff) return;
    }
    failed = (failed & 0xffffff00) | 1;
}

void put_wide(u32 &failed, u8 *buffer, u32 c) {
    if (buffer) {
        u32 *const pointer = at<u32 *>(buffer, 0x20);
        const u32 p = *pointer;
        u32 result;
        if (p) {
            s32 *const count = at<s32 *>(buffer, 0x30);
            const u32 left = static_cast<u32>(*count);
            if (p < p + left * 2) {
                *count = static_cast<s32>(left - 1);
                *at<u32 *>(buffer, 0x20) = p + 2;
                *reinterpret_cast<u16 *>((uintptr_t)p) = static_cast<u16>(c);
                result = c;
            } else {
                result = overflow(buffer, c);
            }
        } else {
            result = overflow(buffer, c);
        }
        if (static_cast<u16>(result) != 0xffff) return;
    }
    failed = (failed & 0xffffff00) | 1;
}

template <typename Pad, typename Digit> u32 *do_put(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 value, Pad pad, Digit digit) {
    char text[0x44];
    const s32 length = CRT_sprintf(text, "%p", value);
    const s32 width = at<s32>(ios, 0x14);
    u32 padding = width > 0 && width > length ? static_cast<u32>(width - length) : 0;
    if ((at<u32>(ios, 0xc) & 0x1c0) != 0x40) {
        for (; padding; --padding) pad(failed, buffer);
    }
    for (s32 i = 0; static_cast<u32>(i) < static_cast<u32>(length); ++i) digit(failed, buffer, text[i]);
    at<u32>(ios, 0x14) = 0;
    for (; padding; --padding) pad(failed, buffer);
    out[0] = failed;
    out[1] = static_cast<u32>(reinterpret_cast<uintptr_t>(buffer));
    return out;
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

struct NarrowChars {
    static const u32 kId = 0x00fb1da8, kCache = 0x00fb1db8;
    static void put(u32 *it, u32 c) { put_narrow(it[0], reinterpret_cast<u8 *>((uintptr_t)it[1]), c); }
    static u32 widen(char c) { return static_cast<u8>(c); }
    static bool any(u32 c) { return static_cast<u8>(c) != 0; }
    static u8 *make(void *object) { return Globalfb1dbc_Set(object ? static_cast<u8 *>(IosBase_Env().numpunctCtor(object, nullptr, 0)) : nullptr); }
};

struct WideChars {
    static const u32 kId = 0x00fb1ddc, kCache = 0x00fb1e1c;
    static void put(u32 *it, u32 c) { put_wide(it[0], reinterpret_cast<u8 *>((uintptr_t)it[1]), c); }
    static u32 widen(char c) { return CRT_btowc(static_cast<u32>(static_cast<s32>(c))); }
    static bool any(u32 c) { return static_cast<u16>(c) != 0; }
    static u8 *make(void *object) { return Globalfb1e30_Set(object ? static_cast<u8 *>(IosBase_Env().wnumpunctCtor(object, nullptr, 0)) : nullptr); }
};

// use_facet<numpunct> on the stream's locale. A missing one comes from the classic locale, made once; other locales throw.
template <typename C> u8 *numpunct_facet(u8 *ios) {
    u8 *locale;
    Obj_ShareMember20(ios, nullptr, &locale);
    u32 lock[2];
    IosBase_Env().lockit(lock);
    u8 *facet = Locale_Getfacet(&locale, nullptr, Id_Get(&global<u32>(C::kId)), 1);
    if (!facet) {
        if (Locale_Iscloc(&locale)) {
            facet = global<u8 *>(C::kCache);
            if (!facet) {
                facet = C::make(IosBase_Env().memAlloc(0x18));
                global<u8 *>(C::kCache) = facet;
            }
        } else {
            u32 bad[3];
            BadCast_Ctor(bad, nullptr, reinterpret_cast<const char *>((uintptr_t)0x0075c734));
            IosBase_Env().cxxThrow(bad, reinterpret_cast<const void *>((uintptr_t)0x007193a8));
        }
    }
    IosBase_Env().unlockit(lock);
    CountedRef_Release(&locale);
    return facet;
}

u32 facet_slot(u8 *facet, u32 offset) {
    using Slot = u32(__fastcall *)(u8 *);
    return reinterpret_cast<const Slot *>((uintptr_t)at<u32>(facet, 0))[offset / 4](facet);
}

template <typename C> void put_times(u32 *it, u32 c, u32 count) {
    for (; count; --count) C::put(it, c);
}

template <typename C> void put_text(u32 *it, const char *text, u32 count) {
    for (; count; --count) C::put(it, C::widen(*text++));
}

template <typename C> u32 *finish(u32 *out, u32 *it, u8 *ios, u32 fill, u32 padding) {
    at<u32>(ios, 0x14) = 0;
    put_times<C>(it, fill, padding);
    out[0] = it[0];
    out[1] = it[1];
    return out;
}

template <typename C> u32 *fput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, const char *text, u32 zeros, u32 count) {
    u32 it[2] = {failed, static_cast<u32>(reinterpret_cast<uintptr_t>(buffer))};
    const s32 width = at<s32>(ios, 0x14);
    u32 padding = width > 0 && static_cast<u32>(width) > zeros + count ? static_cast<u32>(width) - count - zeros : 0;
    const u32 adjust = at<u32>(ios, 0xc) & 0x1c0;
    if (adjust != 0x40) {
        if (adjust == 0x100 && count && (*text == '+' || *text == '-')) {
            C::put(it, C::widen(*text));
            ++text;
            --count;
        }
        put_times<C>(it, fill, padding);
        padding = 0;
    }
    const char *const point = static_cast<const char *>(CRT_memchr(text, **reinterpret_cast<const signed char **>((uintptr_t)Global781fc8_Get_691530()), count));
    if (point) {
        u8 *const facet = numpunct_facet<C>(ios);
        const u32 length = static_cast<u32>(point - text) + 1;
        put_text<C>(it, text, length - 1);
        C::put(it, facet_slot(facet, 4));
        text += length;
        count -= length;
    }
    const char *const exponent = static_cast<const char *>(CRT_memchr(text, 'e', count));
    if (exponent) {
        const u32 length = static_cast<u32>(exponent - text) + 1;
        put_text<C>(it, text, length - 1);
        put_times<C>(it, C::widen('0'), zeros);
        zeros = 0;
        C::put(it, C::widen(at<u32>(ios, 0xc) & 4 ? 'E' : 'e'));
        text += length;
        count -= length;
    }
    put_text<C>(it, text, count);
    put_times<C>(it, C::widen('0'), zeros);
    return finish<C>(out, it, ios, fill, padding);
}

template <typename C> u32 *iput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, char *text, u32 count) {
    u32 it[2] = {failed, static_cast<u32>(reinterpret_cast<uintptr_t>(buffer))};
    const u32 prefix = text[0] == '+' || text[0] == '-' ? 1 : text[0] == '0' && (text[1] == 'x' || text[1] == 'X') ? 2 : 0;
    u8 *const facet = numpunct_facet<C>(ios);
    StdString groups;
    MoneyPunct_Grouping(facet, nullptr, &groups);
    const u32 separator = facet_slot(facet, 8);
    const signed char *group = reinterpret_cast<const signed char *>(groups.ptr ? groups.ptr : reinterpret_cast<const char *>((uintptr_t)0x006fe5b0));
    bool commas = false;
    if (*group > 0) {
        u32 place = count;
        while (*group != 0x7f && *group > 0 && static_cast<u32>(*group) < place - prefix) {
            place -= static_cast<u32>(*group);
            CRT_memmove(text + place + 1, text + place, count - place + 1);
            text[place] = ',';
            ++count;
            if (group[1] > 0) ++group;
            commas = true;
        }
    }
    const s32 width = at<s32>(ios, 0x14);
    u32 padding = width > 0 && static_cast<u32>(width) > count ? static_cast<u32>(width) - count : 0;
    const u32 adjust = at<u32>(ios, 0xc) & 0x1c0;
    if (adjust != 0x40) {
        if (adjust == 0x100) {
            put_text<C>(it, text, prefix);
            text += prefix;
            count -= prefix;
        }
        put_times<C>(it, fill, padding);
        padding = 0;
    }
    if (commas) {
        for (;;) {
            const u32 length = CRT_strcspn(text, ",");
            put_text<C>(it, text, length);
            text += length;
            count -= length;
            if (!count) break;
            if (C::any(separator)) C::put(it, separator);
            ++text;
            --count;
        }
    } else {
        put_text<C>(it, text, count);
    }
    finish<C>(out, it, ios, fill, padding);
    StdString_Tidy(&groups, nullptr, 1);
    return out;
}

template <typename C> u32 *put_integer(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value, char decimal) {
    char format[8];
    char text[0x40];
    const u32 flags = at<u32>(ios, 0xc);
    char *f = format;
    *f++ = '%';
    if (flags & 0x20) *f++ = '+';
    if (flags & 8) *f++ = '#';
    *f++ = 'l';
    const u32 base = flags & 0xe00;
    *f++ = base == 0x400 ? 'o' : base != 0x800 ? decimal : flags & 4 ? 'X' : 'x';
    *f = 0;
    const s32 length = CRT_sprintf(text, format, value);
    return iput<C>(out, failed, buffer, ios, fill, text, static_cast<u32>(length));
}

template <typename C> u32 *put_float(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high, bool isLong) {
    char format[8];
    char text[0x3c];   // (the original's room: %f of a large value overruns it there too)
    const u32 flags = at<u32>(ios, 0xc);
    s32 precision = at<s32>(ios, 0x10);
    if (precision <= 0 && !(flags & 0x2000)) precision = 6;
    const s32 used = precision > 0x24 ? 0x24 : precision;
    char *f = format;
    *f++ = '%';
    if (flags & 0x20) *f++ = '+';
    if (flags & 0x10) *f++ = '#';
    *f++ = '.';
    *f++ = '*';
    if (isLong) *f++ = 'L';
    const u32 notation = flags & 0x3000;
    *f++ = notation == 0x2000 ? 'f' : notation == 0x1000 ? 'e' : 'g';
    *f = 0;
    const s32 length = CRT_sprintf(text, format, used, low, high);
    return fput<C>(out, failed, buffer, ios, fill, text, static_cast<u32>(precision - used), static_cast<u32>(length));
}

}  // namespace

extern "C" {

u32 *__cdecl NumPut_Fput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, const char *text, u32 zeros, u32 count) { return fput<NarrowChars>(out, failed, buffer, ios, fill, text, zeros, count); }
u32 *__cdecl NumPutW_Fput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, const char *text, u32 zeros, u32 count) { return fput<WideChars>(out, failed, buffer, ios, fill, text, zeros, count); }
u32 *__cdecl NumPut_Iput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, char *text, u32 count) { return iput<NarrowChars>(out, failed, buffer, ios, fill, text, count); }
u32 *__cdecl NumPutW_Iput(u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, char *text, u32 count) { return iput<WideChars>(out, failed, buffer, ios, fill, text, count); }

u32 *__fastcall NumPut_DoPutLong(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) { return put_integer<NarrowChars>(out, failed, buffer, ios, fill, value, 'd'); }
u32 *__fastcall NumPut_DoPutUlong(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) { return put_integer<NarrowChars>(out, failed, buffer, ios, fill, value, 'u'); }
u32 *__fastcall NumPutW_DoPutLong(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) { return put_integer<WideChars>(out, failed, buffer, ios, fill, value, 'd'); }
u32 *__fastcall NumPutW_DoPutUlong(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) { return put_integer<WideChars>(out, failed, buffer, ios, fill, value, 'u'); }

u32 *__fastcall NumPut_DoPutDouble(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high) { return put_float<NarrowChars>(out, failed, buffer, ios, fill, low, high, false); }
u32 *__fastcall NumPut_DoPutLongDouble(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high) { return put_float<NarrowChars>(out, failed, buffer, ios, fill, low, high, true); }
u32 *__fastcall NumPutW_DoPutDouble(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high) { return put_float<WideChars>(out, failed, buffer, ios, fill, low, high, false); }
u32 *__fastcall NumPutW_DoPutLongDouble(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 low, u32 high) { return put_float<WideChars>(out, failed, buffer, ios, fill, low, high, true); }

u32 *__fastcall NumPut_DoPutPointer(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) {
    return do_put(out, failed, buffer, ios, value, [fill](u32 &f, u8 *b) { put_narrow(f, b, fill); }, [](u32 &f, u8 *b, char c) { put_narrow(f, b, static_cast<u8>(c)); });
}

u32 *__fastcall NumPutW_DoPutPointer(void *, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value) {
    return do_put(out, failed, buffer, ios, value, [fill](u32 &f, u8 *b) { put_wide(f, b, fill); },
                  [](u32 &f, u8 *b, char c) { put_wide(f, b, CRT_btowc(static_cast<u32>(static_cast<s32>(c)))); });
}

}  // extern "C"
