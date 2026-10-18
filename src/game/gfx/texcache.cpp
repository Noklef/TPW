#include "texcache.h"

#include "animtex.h"
#include "surfacepool.h"
#include "texture.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const u8 *Next(const u8 *item) { return at<const u8 *>(item, 4); }

// Puts the item in the output and adds its area.
void Put(u32 *&out, u32 &area, const u8 *item) {
    *out++ = (u32)(uintptr_t)item;
    area += (u32)((s32)at<s16>(item, 0x14) * (s32)at<s16>(item, 0x16));
}

}  // namespace

extern "C" {

void __fastcall TexCache_Halve(u8 *cache, void *, u8 *dest, const u8 *source) {
    const u32 format = at<u32>(cache, 0x30);
    if (format != 2 && format != 3)
        return;
    const u32 width = at<u32>(dest, 0xc), height = at<u32>(dest, 8);
    const u32 destPitch = at<u32>(dest, 0x10), sourcePitch = at<u32>(source, 0x10);
    u8 *out = reinterpret_cast<u8 *>(at<uintptr_t>(dest, 0x24));
    const u8 *in = reinterpret_cast<const u8 *>(at<uintptr_t>(source, 0x24));
    const u32 bytes = format == 3 ? 4 : 2;
    for (u32 y = height; y; y--) {
        const u8 *below = in + sourcePitch;
        for (u32 x = width; x; x--) {
            if (format == 3) {
                const u32 p00 = at<u32>(in, 0), p01 = at<u32>(in, 4), p10 = at<u32>(below, 0), p11 = at<u32>(below, 4);
                u32 high = ((p01 >> 2) & 0x3fc03fc0) + ((p10 >> 2) & 0x3fc03fc0) + ((p11 >> 2) & 0x3fc03fc0) +
                           ((p00 >> 2) & 0x3fc03fc0) + 0x800080;
                u32 low = (p10 & 0xff00ff) + (p11 & 0xff00ff) + (p01 & 0xff00ff) + (p00 & 0xff00ff) + 0x20002;
                at<u32>(out, 0) = (high & 0xff00ff00) | ((low >> 2) & 0xff00ff);
            } else {
                const u32 p00 = at<u16>(in, 0), p01 = at<u16>(in, 2), p10 = at<u16>(below, 0), p11 = at<u16>(below, 2);
                const u32 high = ((p10 & 0xf0f0) + (p11 & 0xf0f0) + (p01 & 0xf0f0) + (p00 & 0xf0f0) + 0x2020) >> 2;
                const u32 low = ((p00 & 0xf0f) + (p01 & 0xf0f) + (p11 & 0xf0f) + (p10 & 0xf0f) + 0x202) >> 2;
                at<u16>(out, 0) = (u16)(((low ^ high) & 0xf0f) ^ high);
            }
            out += bytes;
            in += 2 * bytes;
            below += 2 * bytes;
        }
        in += 2 * (sourcePitch - bytes * width);
        out += destPitch - bytes * width;
    }
}

u32 __stdcall TexCache_MergeItems(u32 *out, const u8 *listB, const u8 *owner) {
    u32 area = 0;
    const u8 *a = Next(owner);
    const u8 *b = listB;
    if (a && Texture_UsesDriverFilter(a))
        a = Next(a);
    while (a) {
        if (!b)
            break;
        while (a && !(at<u32>(a, 8) & 0x40000000))
            a = Next(a);
        if (!a)
            break;
        while (Texture_UsesDriverFilter(b)) {
            b = Next(b);
            if (!b)
                break;
        }
        if (!b)
            continue;
        const s16 by = at<s16>(b, 0x12), ay = at<s16>(a, 0x12);
        if (by > ay || (by == ay && at<s16>(b, 0x10) > at<s16>(a, 0x10))) {
            Put(out, area, b);
            b = Next(b);
        } else {
            Put(out, area, a);
            a = Next(a);
        }
    }
    for (; b; b = Next(b)) {
        if (!Texture_UsesDriverFilter(b))
            Put(out, area, b);
    }
    for (; a; a = Next(a)) {
        if (at<u32>(a, 8) & 0x40000000)
            Put(out, area, a);
    }
    *out = 0;
    return area;
}

u8 *__fastcall TexCache_InitDefaults(u8 *cache) {
    static const struct { u32 offset, value; } kDefaults[] = {
        {0x18c0, 0x80}, {0x18bc, 0x80}, {0x18b8, 0x80}, {0x18b4, 0x80}, {0x18b0, 0x80},
        {0x187c, 0x3f000000}, {0x1880, 0x3e800000}, {0x1884, 0x3e2e147b}, {0x1888, 0x3d851eb8},
        {0x188c, 0x3c449ba6}, {0x1890, 0x3b449ba6}, {0x18ac, 0x100},
        {0x18f4, 0}, {0x1920, 0}, {0x18f8, 0}, {0x1900, 0x3f800000}, {0x18ec, 0}, {0x1878, 4},
        {0x18c4, 0}, {0x18c8, 0}, {0x18cc, 0}, {0x18d4, 0}, {0x18d0, 0}, {0x18e4, 7}, {0x18e8, 0},
    };
    for (const auto &d : kDefaults)
        at<u32>(cache, d.offset) = d.value;
    for (u32 i = 0; i < 6; i++) {
        at<u32>(cache, 0x1800 + 4 * i) = 0;
        at<u32>(cache, 0x1830 + 4 * i) = 0;
    }
    return cache;
}

u8 *__fastcall TexCacheEntry_Ctor(u8 *entry) {
    for (u32 offset : {0u, 4u, 0xcu, 8u})
        at<u32>(entry, offset) = 0;
    at<u32>(entry, 0x14) = 0xffffffff;
    return entry;
}

u8 *__fastcall TexCacheItem_Ctor(u8 *item) {
    at<u32>(item, 0) = 0x00702198;
    at<u32>(item, 4) = 0;
    return item;
}

void __fastcall TexCacheItem_SetFlags(u8 *item, void *, u32 flags) {
    at<u32>(item, 4) = (at<u32>(item, 4) & 0x10000000) | flags;
}

u32 __fastcall TexCacheItem_Flag1(const u8 *item) { return at<u32>(item, 4) & 1; }

u32 __fastcall TexCacheItem_Flag2(const u8 *item) { return at<u32>(item, 4) & 2; }

u32 __fastcall TexCacheItem_Flag4(const u8 *item) { return at<u32>(item, 4) & 4; }

void __fastcall TexCacheItem_SetFlag1(u8 *item) { at<u32>(item, 4) |= 1; }

void __fastcall TexCacheItem_ClearFlag1(u8 *item) { at<u32>(item, 4) &= ~1u; }

u32 __fastcall TexCache_UnlinkItem(u8 *cache, void *, u8 *item) {
    if (!(at<u32>(cache, 0x18cc) & 0x20000000))
        return 1;
    if (!item)
        return 0;
    if (!(at<u32>(item, 8) & 0x1000))
        return 1;
    u8 *const owner = at<u8 *>(item, 0x18);
    u8 *prev = nullptr;
    u8 *node = at<u8 *>(owner, 4);
    for (;;) {
        if (!node)
            return 0;
        if (node == item)
            break;
        prev = node;
        node = at<u8 *>(node, 4);
    }
    if (prev)
        at<u8 *>(prev, 4) = at<u8 *>(item, 4);
    else
        at<u8 *>(owner, 4) = at<u8 *>(item, 4);
    const u32 masked = at<u32>(item, 8) & 0xffff0fcf;
    const u16 width = (u16)((at<u16>(item, 0x10) + 7) & 0xfff8);
    const X87Reg lod = (X87Reg(1.5f) - at<float>(item, 0x30)) * 2047.0f;  // 0x70232c, 0x702330
    const u16 height = (u16)((at<u16>(item, 0x12) + 7) & 0xfff8);
    const u32 flags = ((u32)((s32)masked >> 2) & 0x30) | masked;
    at<u32>(item, 8) = flags;
    at<u16>(item, 0x14) = width;
    at<u16>(item, 0x16) = height;
    const u32 key = ((((u32)(long long)lod.v & 0xfff) << 8) + (u32)(s32)(s16)width << 8) + (u32)(s32)(s16)height;
    at<u32>(item, 0x38) = key;
    if (flags & 0x4000)
        at<u32>(item, 0x38) = key | 0x20000000;  // never: the mask above clears 0x4000 (sic)
    if (flags & 0x80000)
        at<u32>(item, 0x38) |= 0x40000000;
    return 1;
}

void __fastcall TexCacheItem_SetKey(u8 *item) {
    const X87Reg lod = (X87Reg(1.5f) - at<float>(item, 0x30)) * 2047.0f;  // 0x7022d0, 0x7022d4
    const u32 key = ((((u32)(long long)lod.v & 0xfff) << 8) + (u32)(s32)at<s16>(item, 0x14) << 8) + (u32)(s32)at<s16>(item, 0x16);
    const u32 flags = at<u32>(item, 8);
    at<u32>(item, 0x38) = key;
    if (flags & 0x4000)
        at<u32>(item, 0x38) = key | 0x20000000;   // (the flags as they are)
    if (flags & 0x80000)
        at<u32>(item, 0x38) |= 0x40000000;   // (in memory)
}

void __fastcall TexCache_Evict(u8 *cache) {
    s32 limit = at<s32>(cache, 0x1878);
    if (limit > 0x10)
        limit = 0x10;
    for (u32 size = 0; size < 6; ++size) {
        const s32 side = at<s32>(reinterpret_cast<const void *>((uintptr_t)0x00702420), 4 * size);
        const s32 area = side * side;
        s32 keys[0x11];   // (one past for the full insertions)
        u8 *entries[0x11];
        s32 kept = 0;
        const s32 count = at<s32>(cache, 0x1800 + 4 * size);
        for (s32 i = 0; i < count; ++i) {
            u8 *const entry = at<u8 *>(cache, 0x400 * size + 4 * i);
            if (!at<u32>(entry, 4))
                continue;
            const s32 used = at<s32>(entry, 0x1c);
            const s32 key = at<s32>(entry, 0xc);
            if (used <= 0 || (s32)((u32)used << 2) >= area || (s32)((u32)key * 2) >= area)
                continue;
            s32 place = kept;
            while (place && key < keys[place - 1])
                --place;
            for (s32 j = place; j < kept; ++j)   // forward over the overlap (sic)
                entries[j + 1] = entries[j];
            for (s32 j = place; j < kept; ++j)
                keys[j + 1] = keys[j];
            if (kept < limit)
                ++kept;
            entries[place] = entry;
            keys[place] = key;
        }
        for (s32 i = kept - 1; i >= 0; --i) {
            u8 *const entry = entries[i];
            u8 *const owner = at<u8 *>(entry, 0);
            u8 *item = at<u8 *>(entry, 4);
            at<u8 *>(entry, 4) = nullptr;
            while (item) {
                const u32 had = at<u32>(item, 8) & 0x4000;
                u8 *const next = at<u8 *>(item, 4);
                Texture_ClearCacheFlags(item);
                at<u32>(item, 0x1c) = 0;
                const u32 flags = at<u32>(item, 8);
                if (flags & 0x40000000) {
                    at<u32>(item, 8) = flags | (had ? 0x6000 : 0x2000);
                    at<u8 *>(item, 4) = at<u8 *>(cache, 0x18c4);
                    at<u8 *>(cache, 0x18c4) = item;
                }
                item = next;
            }
            TexCacheItem_SetFlags(owner, nullptr, 0);
        }
    }
}

u32 __fastcall TexCache_AddSurfaces(u8 *cache, void *, s32 side, s32 *made, u32 format, s32 levels) {
    s32 count = 0;
    bool failed = false;
    const s32 index = SurfacePool_SideIndex(side);
    s32 area = (s32)(at<u32>(nullptr, 0x007022d8 + 4 * at<u32>(nullptr, 0x008bd554)) * (u32)side * (u32)side);
    if (levels > 1)
        area = (s32)((u32)area * 4) / 3;
    s32 &step = at<s32>(nullptr, 0x008c4898);
    step = area;
    s32 &budget = at<s32>(cache, 0x1860 + 4 * index);
    s32 &entries = at<s32>(cache, 0x1800 + 4 * index);
    if (budget >= area) {
        do {
            if (failed || entries >= 0x100)
                break;
            u8 *const device = at<u8 *>(nullptr, 0x008bd52c);
            u8 *const surface = reinterpret_cast<u8 *(__fastcall *)(u8 *)>(at<void **>(device, 0)[4])(device);
            if (surface) {
                void **const slots = at<void **>(surface, 0);
                if (!reinterpret_cast<s32(__fastcall *)(u8 *, void *, s32, s32, u32, s32)>(slots[0])(surface, nullptr, side, side, format, levels)) {
                    reinterpret_cast<void(__fastcall *)(u8 *, void *, u32)>(slots[9])(surface, nullptr, 1);
                    failed = true;
                } else {
                    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x20));
                    at<u8 *>(cache, 4 * (entries + (index << 8))) = block ? TexCacheEntry_Ctor(block) : nullptr;
                    at<u8 *>(at<u8 *>(cache, 4 * (entries + (index << 8))), 0) = surface;
                    ++entries;
                    ++count;
                }
            }
            budget -= step ? step : area;
        } while (budget >= step);
    }
    *made = count;
    AnimTextures_Set(reinterpret_cast<const u32 *>(cache), reinterpret_cast<const u32 *>(cache + 0x1800));
    return count > 0;
}

s32 __fastcall TexCache_ChooseSize(u8 *cache, void *, s32 a, s32 b, s32 limit) {
    const s32 freeA = (s32)(at<u32>(cache, 0x1818 + 4 * a) - at<u32>(cache, 0x1830 + 4 * a));
    const float shareA = (float)(X87Reg((double)freeA) / X87Reg((double)at<s32>(cache, 0x1800 + 4 * a)));
    const s32 freeB = (s32)(at<u32>(cache, 0x1818 + 4 * b) - at<u32>(cache, 0x1830 + 4 * b));
    const float shareB = (float)(X87Reg((double)freeB) / X87Reg((double)at<s32>(cache, 0x1800 + 4 * b)));
    u8 *const surface = at<u8 *>(at<u8 *>(cache, (u32)a << 10), 0);
    const s32 used = reinterpret_cast<s32(__fastcall *)(u8 *)>(at<void **>(surface, 0)[3])(surface);
    if (!((X87Reg(shareB) + X87Reg(shareB)).v >= shareA) && at<s32>(cache, 0x1818 + 4 * b) + 10 < at<s32>(cache, 0x1818 + 4 * a) &&
        (s32)((u32)used * 3) < (s32)((u32)limit * 4))
        return b;
    return a;
}
}
