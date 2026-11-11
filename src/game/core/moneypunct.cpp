#include "moneypunct.h"

#include <cstdint>
#include <cstring>

#include "buffers.h"
#include "crt.h"
#include "gameheap.h"
#include "globals.h"
#include "iosbase.h"
#include "wtraits.h"
#include "localefacet.h"

namespace {

u32 dword_at(uintptr_t at) {
    u32 value;
    std::memcpy(&value, reinterpret_cast<const void *>(at), 4);
    return value;
}

const char *lconv_text(const u8 *lconv, u32 at) { return *reinterpret_cast<const char *const *>(lconv + at); }

char *copy_narrow(const char *from) {
    const u32 size = static_cast<u32>(std::strlen(from)) + 1;
    char *const to = static_cast<char *>(GameHeap_Alloc(size));
    std::memcpy(to, from, size);
    return to;
}

u16 widen(char c) { return static_cast<u16>(CRT_btowc(static_cast<u32>(static_cast<s32>(static_cast<signed char>(c))))); }

u16 *copy_wide(const char *from) {
    const u32 size = static_cast<u32>(std::strlen(from)) + 1;
    u16 *const to = static_cast<u16 *>(GameHeap_Alloc(size * 2));
    for (u32 at = 0; at < size; ++at) to[at] = widen(from[at]);
    return to;
}

// The pattern from 0x709d18, whose entries overlap at a byte's steps: the precedes flag at `flags`, the space flag after it, the sign position.
u32 pattern(const u8 *lconv, u32 flags, s8 position, bool international) {
    const u8 precedes = lconv[flags];
    const u8 space = lconv[flags + 1];
    if (international || space & 0xfe || precedes & 0xfe || position < 0 || position > 4) return dword_at(0x00709d10);
    return dword_at(0x00709d18 + (precedes == 1 ? 0x14 : 0) + (space == 1 ? 0x28 : 0) + position);
}

const char *sign_text(const u8 *lconv, u32 at, s8 position, uintptr_t otherwise) {
    return position >= 0 && position <= 4 ? lconv_text(lconv, at) : reinterpret_cast<const char *>(otherwise);
}

// All but the two separators, and the texts through `copy`.
template <typename Copy> void init_rest(u8 *facet, const u8 *lconv, Copy copy) {
    *reinterpret_cast<char **>(facet + 8) = copy_narrow(lconv_text(lconv, 0x1c));
    const bool international = facet[0x28] != 0;
    *reinterpret_cast<void **>(facet + 0x10) = copy(lconv_text(lconv, international ? 0xc : 0x10));
    const s8 positive = static_cast<s8>(lconv[0x2e]);
    const s8 negative = static_cast<s8>(lconv[0x2f]);
    *reinterpret_cast<void **>(facet + 0x14) = copy(sign_text(lconv, 0x20, positive, 0x00786de4));
    *reinterpret_cast<void **>(facet + 0x18) = copy(sign_text(lconv, 0x24, negative, 0x007638f4));
    const s32 digits = static_cast<s8>(lconv[international ? 0x28 : 0x29]);
    *reinterpret_cast<s32 *>(facet + 0x1c) = digits < 0 || digits >= 0x7f ? 0 : digits;
    *reinterpret_cast<u32 *>(facet + 0x20) = pattern(lconv, 0x2a, positive, international);
    *reinterpret_cast<u32 *>(facet + 0x24) = pattern(lconv, 0x2c, negative, international);
}

const u8 *lconv_now() { return reinterpret_cast<const u8 *>((uintptr_t)Global781fc8_Get_691530()); }

void tidy(u32 *self, u32 vtable) {
    self[0] = vtable;
    const u32 slots[] = {2, 4, 5, 6};
    for (const u32 slot : slots) IosBase_Env().memFree(reinterpret_cast<void *>((uintptr_t)self[slot]));
    self[0] = (u32)(uintptr_t)LocaleFacet_vtable;
}

}  // namespace

extern "C" u32 __cdecl CRT_wcslen(const u16 *text);   // (online/chatlists)

static const char *text_at(const u8 *self, u32 at) { return *reinterpret_cast<const char *const *>(self + at); }

static const u16 *wide_at(const u8 *self, u32 at) { return *reinterpret_cast<const u16 *const *>(self + at); }

template <typename String> static void blank(String *out, u8 allocator) {
    *reinterpret_cast<u8 *>(out) = allocator;
    out->ptr = nullptr;
    out->len = 0;
    out->res = 0;
}

template <typename String> static u8 low_byte(const String *pointer) { return static_cast<u8>(reinterpret_cast<uintptr_t>(pointer)); }

static StdString *narrow_string(StdString *out, const char *text) {
    blank(out, low_byte(out));
    const u32 n = static_cast<u32>(strlen(text));
    if (n > 0xfffffffdu) StdStrings_Xlen();
    StdString_Tidy(out, nullptr, 1);
    if (!n) return out;
    StdString_Copy(out, nullptr, n);
    memcpy(out->ptr, text, n);
    out->len = n;
    out->ptr[n] = 0;
    return out;
}

static StdWString *wide_string(StdWString *out, const u16 *text) {
    blank(out, low_byte(out));
    const u32 n = static_cast<u32>(CRT_wcslen(text));
    if (n > 0x7ffffffdu) StdStrings_Xlen();
    CountedBuf_Reset(reinterpret_cast<u8 *>(out), nullptr, 1);
    if (!n) return out;
    StdWString_Copy(out, nullptr, n);
    for (u32 i = 0; i < n; ++i) out->ptr[i] = text[i];
    out->len = n;
    out->ptr[n] = 0;
    return out;
}

extern "C" {

u32 *__fastcall Moneypunct_Makepat(u8 *facet, void *, u32 *out, u32 space, u32 precedes, u32 position) {
    const u8 spaced = static_cast<u8>(space);
    const u8 before = static_cast<u8>(precedes);
    const s8 at = static_cast<s8>(position);
    if (facet[0x28] || spaced & 0xfe || before & 0xfe || at < 0 || at > 4)
        *out = dword_at(0x00709d10);
    else
        *out = dword_at(0x00709d18 + (before == 1 ? 0x14 : 0) + (spaced == 1 ? 0x28 : 0) + at);
    return out;
}

void __fastcall MoneypunctW_Tidy(u32 *self, void *) { tidy(self, (u32)(uintptr_t)MoneypunctW_vtable); }

void __fastcall MoneypunctC_Tidy(u32 *self, void *) { tidy(self, (u32)(uintptr_t)MoneypunctC_vtable); }

void __fastcall MoneypunctC_Init(u8 *facet, void *, const void *) {
    const u8 *const lconv = lconv_now();
    facet[0xc] = *lconv_text(lconv, 0x14);
    facet[0xd] = *lconv_text(lconv, 0x18);
    init_rest(facet, lconv, [](const char *from) -> void * { return copy_narrow(from); });
}

void __fastcall MoneypunctW_Init(u8 *facet, void *, const void *) {
    const u8 *const lconv = lconv_now();
    *reinterpret_cast<u16 *>(facet + 0xc) = widen(*lconv_text(lconv, 0x14));
    *reinterpret_cast<u16 *>(facet + 0xe) = widen(*lconv_text(lconv, 0x18));
    init_rest(facet, lconv, [](const char *from) -> void * { return copy_wide(from); });
}

void *__fastcall MoneypunctC_Ctor(u8 *facet, void *, const void *locinfo, u32 refs, u8 international) {
    *reinterpret_cast<u32 *>(facet + 4) = refs;
    facet[0x28] = international;
    *reinterpret_cast<u32 *>(facet) = (u32)(uintptr_t)MoneypunctC_vtable;
    MoneypunctC_Init(facet, nullptr, locinfo);
    return facet;
}

void *__fastcall MoneypunctW_Ctor(u8 *facet, void *, const void *locinfo, u32 refs, u8 international) {
    *reinterpret_cast<u32 *>(facet + 4) = refs;
    facet[0x28] = international;
    *reinterpret_cast<u32 *>(facet) = (u32)(uintptr_t)MoneypunctW_vtable;
    MoneypunctW_Init(facet, nullptr, locinfo);
    return facet;
}

void *__fastcall MoneypunctWIntl_Ctor(u8 *facet, void *, const void *locinfo, u32 refs) {
    MoneypunctW_Ctor(facet, nullptr, locinfo, refs, 1);
    *reinterpret_cast<u32 *>(facet) = (u32)(uintptr_t)Class709da4_vtable;
    return facet;
}

StdString *__fastcall Facet_Grouping(const u8 *self, void *, StdString *out) { return narrow_string(out, text_at(self, 8)); }
StdString *__fastcall MoneypunctC_CurrSymbol(const u8 *self, void *, StdString *out) { return narrow_string(out, text_at(self, 0x10)); }
StdString *__fastcall MoneypunctC_PositiveSign(const u8 *self, void *, StdString *out) { return narrow_string(out, text_at(self, 0x14)); }
StdString *__fastcall MoneypunctC_NegativeSign(const u8 *self, void *, StdString *out) { return narrow_string(out, text_at(self, 0x18)); }
StdWString *__fastcall MoneypunctW_CurrSymbol(const u8 *self, void *, StdWString *out) { return wide_string(out, wide_at(self, 0x10)); }
StdWString *__fastcall MoneypunctW_PositiveSign(const u8 *self, void *, StdWString *out) { return wide_string(out, wide_at(self, 0x14)); }
StdWString *__fastcall MoneypunctW_NegativeSign(const u8 *self, void *, StdWString *out) { return wide_string(out, wide_at(self, 0x18)); }

StdString *__fastcall Facet709fac_Get(const u8 *self, void *, StdString *out, u32, u32, u32 which, const StdString *fallback) {
    if (which > 1) {
        blank(out, *reinterpret_cast<const u8 *>(fallback));
        StdString_AssignSub(out, nullptr, fallback, 0, *reinterpret_cast<u32 *>((uintptr_t)0x006fe5ac));
        return out;
    }
    const char *const text = text_at(self, which ? 0xc : 8);
    blank(out, low_byte(out));
    const u32 n = static_cast<u32>(strlen(text));
    if (StdString_Grow(out, nullptr, n, 1)) {
        memcpy(out->ptr, text, n);
        out->len = n;
        out->ptr[n] = 0;
    }
    return out;
}

StdWString *__fastcall WFacet709c0c_Get(const u8 *self, void *, StdWString *out, u32, u32, u32 which, const StdWString *fallback) {
    if (which > 1) {
        blank(out, *reinterpret_cast<const u8 *>(fallback));
        StdWString_AssignSub(out, nullptr, fallback, 0, *reinterpret_cast<u32 *>((uintptr_t)0x006fef00));
        return out;
    }
    const u16 *const text = wide_at(self, which ? 0xc : 8);
    blank(out, low_byte(out));
    const u32 n = static_cast<u32>(CRT_wcslen(text));
    if (StdWString_Grow(out, nullptr, n, 1)) {
        WTraits_Copy(out->ptr, text, n);
        out->len = n;
        out->ptr[n] = 0;
    }
    return out;
}

}  // extern "C"
