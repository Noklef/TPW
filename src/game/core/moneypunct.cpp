#include "moneypunct.h"

#include <cstdint>
#include <cstring>

#include "crt.h"
#include "gameheap.h"
#include "globals.h"
#include "iosbase.h"

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
    self[0] = 0x00709b44;
}

}  // namespace

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

void __fastcall MoneypunctW_Tidy(u32 *self, void *) { tidy(self, 0x00709d74); }

void __fastcall MoneypunctC_Tidy(u32 *self, void *) { tidy(self, 0x00709fdc); }

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
    *reinterpret_cast<u32 *>(facet) = 0x00709fdc;
    MoneypunctC_Init(facet, nullptr, locinfo);
    return facet;
}

void *__fastcall MoneypunctW_Ctor(u8 *facet, void *, const void *locinfo, u32 refs, u8 international) {
    *reinterpret_cast<u32 *>(facet + 4) = refs;
    facet[0x28] = international;
    *reinterpret_cast<u32 *>(facet) = 0x00709d74;
    MoneypunctW_Init(facet, nullptr, locinfo);
    return facet;
}

}  // extern "C"
