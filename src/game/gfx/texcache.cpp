#include "texcache.h"

#include "animtex.h"
#include "surfacepool.h"
#include "texture.h"

#include <cstdint>
#include <initializer_list>

#include "../core/addrcalls.h"
#include "../core/crt.h"
#include "../core/gameheap.h"
#include "../core/globals.h"
#include "../core/recordsort.h"
#include "../core/vcall.h"
#include "../core/vobj3.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"
#include "texcache.h"
#include "../render/state.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

const u8 *Next(const u8 *item) { return at<const u8 *>(item, 4); }

// Through the item's virtual base (the offset at +4 of the table its +0 points to): the base's byte at the frame slot (0x879d48) taken
// off its +0x10 and cleared, its s32 +0x14 halved (an arithmetic shift).
void DecaySlot(u8 *item) {
    u8 *const base = item + at<s32>(at<u8 *>(item, 0), 4);
    at<u32>(base, 0x10) -= at<u8>(base, global<u32>(0x00879d48));
    at<u8>(base, global<u32>(0x00879d48)) = 0;
    at<s32>(base, 0x14) >>= 1;
}

// TexCache_ReleaseItems' reset: flags 0x7000 and 0x30 cleared and bits 6 and 7 copied to 4 and 5, the sizes (+0x10, +0x12) rounded
// up by the cache's mask (+0x18e4) to +0x14 / +0x16, the key set; with clear, the virtual base's first six dwords zeroed.
void ResetItem(const u8 *cache, u8 *item, bool clear) {
    const u32 masked = at<u32>(item, 8) & ~0x7030u;
    at<u32>(item, 8) = ((u32)((s32)masked >> 2) & 0x30) | masked;
    const u32 mask = at<u32>(cache, 0x18e4);
    at<u16>(item, 0x14) = (u16)((at<u16>(item, 0x10) + mask) & ~mask);
    at<u16>(item, 0x16) = (u16)((at<u16>(item, 0x12) + mask) & ~mask);
    TexCacheItem_SetKey(item);   // (inlined in the second loop, with 0x70232c's copies of its constants)
    if (clear) {
        u8 *const base = item + at<s32>(at<u8 *>(item, 0), 4);
        for (u32 offset = 0; offset < 0x18; offset += 4)
            at<u32>(base, offset) = 0;
    }
}

// Puts the item in the output and adds its area.
void Put(u32 *&out, u32 &area, const u8 *item) {
    *out++ = (u32)(uintptr_t)item;
    area += (u32)((s32)at<s16>(item, 0x14) * (s32)at<s16>(item, 0x16));
}

AddrCalls g_destroyCalls;
AddrCalls g_setupCalls;

u32 address(const void *p) { return (u32)(uintptr_t)p; }

}  // namespace

void TexCache_SetDestroyCalls(const AddrCall *table, u32 count) { g_destroyCalls.Set(table, count); }
void TexCache_SetSetupCalls(const AddrCall *table, u32 count) { g_setupCalls.Set(table, count); }

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

u32 __fastcall TexCache_PackItems(u8 *cache, void *, u8 **items, u8 *out, u32 *outCount, const s32 *area, u32) {
    struct Rect {
        u32 x, y, w, h, key;
    };
    Rect rects[0x80];
    u8 temp[0x14];
    s32 count = 1, placed = 0;
    u32 whole = 1;
    rects[0].x = rects[0].y = 0;
    rects[0].w = (u32)area[2];
    rects[0].h = (u32)area[3];
    rects[0].key = (rects[0].w << 16) + rects[0].h;
    at<u32>(cache, 0x1924) = 0;
    at<u32>(cache, 0x191c) = 0;
    at<u32>(cache, 0x1918) = 0;
    global<u32>(0x008c4b6c) = 0;
    u8 *item = items[0];
    u8 **next = items + 1;
    if (item) {
        while (count != 0 && placed < 0x7f) {
            const s32 mask = at<s32>(cache, 0x18e4);
            s32 i = count - 1;
            for (; i >= 0; i--) {
                s32 width = at<s16>(item, 0x14), height = at<s16>(item, 0x16);
                if (at<u32>(cache, 0x18e8)) {
                    width = (s32)(((u32)width + mask) & ~(u32)mask);
                    height = (s32)(((u32)height + mask) & ~(u32)mask);
                }
                if (width <= (s32)rects[i].w && height <= (s32)rects[i].h)
                    break;
            }
            const bool halve = placed == 0 && i == -1 && (at<u8>(cache, 0x18cc) & 1);
            if (halve) {
                const bool filter = Texture_UsesDriverFilter(item) != 0;
                const bool can = (at<u8>(item, 8) & 0x30) && (!filter || at<s16>(item, 0x14) > global<s32>(0x00702434));
                if (can) {
                    if (!Texture_UsesDriverFilter(item) || at<s16>(item, 0x14) != global<s32>(0x00702434)) {
                        at<u32>(item, 8) -= 0x10;
                        const s32 height = at<s16>(item, 0x16);
                        at<u16>(item, 0x14) = (u16)(((s32)at<s16>(item, 0x14) + 1) >> 1);
                        at<u16>(item, 0x16) = (u16)((height + 1) >> 1);
                        TexCacheItem_SetKey(item);
                    }
                    const s32 key = at<s32>(item, 0x38);
                    u8 **end = next;
                    while (*end && at<s32>(*end, 0x38) > key)
                        end++;
                    for (u8 **p = next; p != end; p++)
                        p[-1] = *p;
                    end[-1] = item;
                    at<u32>(cache, 0x1924) = 1;
                    item = next[-1];
                    if (!item)
                        break;
                    continue;
                }
            }
            const s32 width = at<s16>(item, 0x14), height = at<s16>(item, 0x16);
            if (at<s32>(cache, 0x1918) < width)
                at<s32>(cache, 0x1918) = width;
            if (at<s32>(cache, 0x191c) < height)
                at<s32>(cache, 0x191c) = height;
            if (i < 0) {
                whole = 0;
            } else {
                Rect &rect = rects[i];
                u32 *const row = reinterpret_cast<u32 *>(out + placed * 0x14);
                u32 usedW, usedH;
                if (at<u8>(cache, 0x18cc) & 2) {
                    const u32 padW = ((width + (u32)mask) & ~(u32)mask) - width;
                    const u32 padH = ((height + (u32)mask) & ~(u32)mask) - height;
                    row[0] = rect.x + (u32)((s32)padW >> 1);
                    row[1] = rect.y + (u32)((s32)padH >> 1);
                    usedW = padW + width;
                    usedH = padH + height;
                } else {
                    row[0] = rect.x;
                    row[1] = rect.y;
                    usedW = (u32)width;
                    usedH = (u32)height;
                }
                row[2] = usedW;
                row[3] = usedH;
                row[4] = (u32)(uintptr_t)item;
                placed++;
                const bool high = (s32)(rect.h - usedH) > 8;
                const bool wide = (s32)(rect.w - usedW) > 8;
                if (!wide && !high) {
                    count--;
                    rect = rects[count];
                } else if (wide && !high) {
                    rect.x += usedW;
                    rect.w -= usedW;
                    rect.key = (rect.w << 16) + rect.h;
                } else if (!wide) {
                    rect.y += usedH;
                    rect.h -= usedH;
                    rect.key = (rect.w << 16) + rect.h;
                } else {
                    Rect &top = rects[count];
                    top = rect;
                    top.y += usedH;
                    top.h -= usedH;
                    top.w = usedW;
                    top.key = (top.w << 16) + top.h;
                    count++;
                    rect.x += usedW;
                    rect.w -= usedW;
                    rect.key = (rect.w << 16) + rect.h;
                }
                if (count - i > 1)
                    RecordSort_Run(reinterpret_cast<RecordSort *>(0x008bcb98), nullptr, reinterpret_cast<u8 *>(rects), temp, 0x14, 0x10, i, count - 1);
                global<u32>(0x008c4b6c)++;
                global<u32>(0x008c4b68)++;
            }
            next++;
            item = next[-1];
            if (!item)
                break;
        }
    }
    at<u32>(out + placed * 0x14, 0x10) = 0;
    *outCount = (u32)placed;
    return whole && !item ? 1 : 0;
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
    // The original's frame: keys[16] at ESP+0x38, entries[16] right after them at ESP+0x78, then (+0xb8) the return address. A 17th
    // qualifying entry with 16 kept writes one past each (sic): keys[16] is entries[0] and entries[16] the return address.
    u32 frame[0x21];
    s32 *const keys = reinterpret_cast<s32 *>(frame);
    u32 *const entries = frame + 0x10;
    bool returnClobbered = false;
    for (u32 size = 0; size < 6; ++size) {
        const s32 side = at<s32>(reinterpret_cast<const void *>((uintptr_t)0x00702420), 4 * size);
        const s32 area = side * side;
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
                keys[j + 1] = keys[j];   // (keys[16]: entries[0], sic)
            if (kept == 0x10 && place < kept)
                returnClobbered = true;   // (entries[16], sic)
            if (kept < limit)
                ++kept;
            entries[place] = (u32)(uintptr_t)entry;
            keys[place] = key;
            if (place == 0x10)
                returnClobbered = true;
        }
        for (s32 i = kept - 1; i >= 0; --i) {
            u8 *const entry = reinterpret_cast<u8 *>((uintptr_t)entries[i]);
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
    if (returnClobbered)   // the original's RET goes to the entry pointer written over its return address (sic)
        reinterpret_cast<void(__cdecl *)()>((uintptr_t)entries[0x10])();
}

void __fastcall TexCache_RequeueItems(u8 *, void *, u8 **list, u8 *item) {
    while (item) {
        Texture_ClearCacheFlags(item);
        at<u32>(item, 0x1c) = 0;
        const u32 flags = at<u32>(item, 8);
        if (flags & 0x40000000) {
            at<u32>(item, 8) = flags | 0x2000;
            u8 *prev = nullptr;
            u8 *cur = *list;
            if (cur) {
                const s32 key = at<s32>(item, 0x38);
                while (at<s32>(cur, 0x38) > key) {
                    prev = cur;
                    cur = at<u8 *>(cur, 4);
                    if (!cur)
                        break;
                }
            }
            u8 *const next = at<u8 *>(item, 4);
            if (cur) {
                if (!prev) {
                    at<u8 *>(item, 4) = cur;
                    *list = item;
                } else {
                    at<u8 *>(prev, 4) = item;
                    at<u8 *>(item, 4) = cur;
                }
            } else {
                at<u8 *>(item, 4) = nullptr;
                if (prev)
                    at<u8 *>(prev, 4) = item;
                else
                    *list = item;
            }
            item = next;
            continue;
        }
        const u32 kept = flags & 0xffff0fcf;
        at<u32>(item, 8) = kept;
        const u16 w = (u16)((at<u16>(item, 0x10) + 7) & ~7);
        const u16 h = (u16)((at<u16>(item, 0x12) + 7) & ~7);
        at<u32>(item, 8) = kept | ((u32)((s32)kept >> 2) & 0x30);   // (bits 6 and 7 copied down to 4 and 5)
        at<u16>(item, 0x14) = w;
        at<u16>(item, 0x16) = h;
        TexCacheItem_SetKey(item);   // (inlined; its constants are 0x702460's copies)
        DecaySlot(item);
        item = at<u8 *>(item, 4);
    }
}

void __fastcall TexCache_ReleaseItems(u8 *cache, void *, u32 mode) {
    for (u32 size = 0; size < 6; size++) {
        u8 **const entries = reinterpret_cast<u8 **>(cache + 0x400 * size);
        const s32 *const count = &at<s32>(cache, 0x1800 + 4 * size);
        for (s32 i = 0; i < *count; i++) {
            u8 *item = at<u8 *>(entries[i], 4);
            at<u8 *>(entries[i], 4) = nullptr;
            if (!(mode & 2)) {
                for (; item; item = at<u8 *>(item, 4))
                    ResetItem(cache, item, (mode & 1) != 0);
                continue;
            }
            TexCacheItem_SetFlags(at<u8 *>(entries[i], 0), nullptr, 0);
            while (item) {
                if (at<u32>(item, 8) & 0x200000) {
                    ResetItem(cache, item, (mode & 1) != 0);
                    item = at<u8 *>(item, 4);
                } else {
                    u8 *const next = at<u8 *>(item, 4);
                    at<u8 *>(item, 4) = at<u8 *>(entries[i], 4);
                    at<u8 *>(entries[i], 4) = item;
                    item = next;
                }
            }
        }
    }
}

void __fastcall TexCache_ShrinkItems(u8 *cache, void *, u8 **items, s32 count, s32 tier, s32 limit) {
    const s32 *const tiers = reinterpret_cast<const s32 *>((uintptr_t)0x00702360);
    const s32 smallest = global<s32>(0x0070235c);
    for (s32 i = 0; i < count; i = (s32)((u32)i + 1)) {
        u8 *const item = items[i];
        if ((s32)(at<u32>(item, 8) & 0xc0) < tiers[tier * 2])
            continue;
        const s32 band = tiers[tier * 2 + 1];
        const float lod = at<float>(item, 0x30);
        if (!(lod > at<float>(cache, 0x1904 + band * 4)) || lod > at<float>(cache, 0x1908 + band * 4))
            continue;
        const bool filter = Texture_UsesDriverFilter(item) != 0;
        if (!(at<u8>(item, 8) & 0x30) || (filter && !(at<s16>(item, 0x14) > smallest)))
            continue;
        const s32 before = (s32)at<s16>(item, 0x16) * at<s16>(item, 0x14);
        if (at<u32>(item, 8) & 0x80000)
            continue;
        if (Texture_UsesDriverFilter(item) && at<s16>(item, 0x14) == smallest)
            continue;
        const s32 width = ((s32)at<s16>(item, 0x14) + 1) >> 1;
        const s32 height = ((s32)at<s16>(item, 0x16) + 1) >> 1;
        at<u32>(item, 8) -= 0x10;
        at<u16>(item, 0x14) = (u16)width;
        at<u16>(item, 0x16) = (u16)height;
        TexCacheItem_SetKey(item);   // (inlined; its constants are 0x7023f0's copies)
        const s32 after = (s32)at<s16>(items[i], 0x16) * at<s16>(items[i], 0x14);
        at<u32>(cache, 0x18dc) += (u32)(after - before);
        if (at<s32>(cache, 0x18dc) < limit)
            i = count;
    }
}

void __fastcall TexCache_BeginFrame(u8 *cache) {
    if (global<u32>(0x008bd508) & 0xc0000000)
        at<u32>(cache, 0x18cc) |= 1;
    else
        at<u32>(cache, 0x18cc) &= ~1u;
    Table879d08_FillFrom();
    for (u32 size = 0; size < 6; ++size) {
        for (s32 i = 0; i < at<s32>(cache, 0x1800 + 4 * size); ++i) {
            u8 *const entry = at<u8 *>(cache, 0x400 * size + 4 * i);
            for (u8 *item = at<u8 *>(entry, 4); item; item = at<u8 *>(item, 4)) {
                at<u32>(item, 8) &= ~0x2000u;
                DecaySlot(item);
                at<u32>(item, 8) &= ~0x40000000u;
                at<u32>(item, 0x30) = 0x3f800000;   // 1.0f
                if (at<u32>(cache, 0x18cc) & 1)
                    Texture_ClearKeep(item);
            }
            if (at<u32>(cache, 0x18cc) & 1)
                TexCacheItem_ClearFlag1(at<u8 *>(entry, 0));
        }
    }
    for (u8 *item = at<u8 *>(cache, 0x18c4); item; item = at<u8 *>(item, 4))
        at<u32>(item, 8) &= ~0x2000u;
    at<u8 *>(cache, 0x18c4) = nullptr;
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

}  // extern "C"

namespace {

// The side index's last entry given up: its record's held object deleted, its block freed, the count down. False with none.
bool giveUpLast(u8 *cache, s32 index) {
    s32 &count = at<s32>(cache, 0x1800 + 4 * index);
    if (count == 0) return false;
    u8 **const slot = &at<u8 *>(cache, 4 * ((index << 8) + count) - 4);
    if (u8 *const held = at<u8 *>(*slot, 0)) vcall<u32>(held, 0x24, 1u);
    GameHeap_Free(*slot);
    --count;
    return true;
}

// The debug log's text, with the engine debug flag.
template <typename... A> void debugLog(const char *format, A... args) {
    if (!(at<u32>(nullptr, 0x008bd508) & 0x1000000)) return;
    char text[100];
    Crt().sprintf(text, format, args...);
    EngineErrors_GetEnv()->log(text);
}

}  // namespace

extern "C" {

u32 __fastcall TexCache_CreateSurfaces(u8 *cache) {
    const s32 *const sides = reinterpret_cast<const s32 *>(0x007022e8);
    const auto add = [&](s32 i) {
        s32 made;
        return TexCache_AddSurfaces(cache, nullptr, sides[i], &made, at<u32>(nullptr, 0x008bd554), at<s32>(cache, 0x18e8));
    };
    for (s32 pass = 0; pass < 2; ++pass) {
        for (s32 i = 0; i < 6; ++i) {
            s32 &budget = at<s32>(cache, 0x1860 + 4 * i);
            if (budget == 0) continue;
            if (!add(i) && pass == 0) {
                const bool room = giveUpLast(cache, SurfacePool_SideIndex(0)) || giveUpLast(cache, SurfacePool_SideIndex(1));   // (indexes -1 and 8, sic)
                if (!room || !add(i)) {
                    debugLog("Failed to create surfaces in tcache, size %d, count %d", sides[i], i);
                    Object_FreeSixPointerArrays(cache);
                    return 0;
                }
            }
            if (i < 5) {
                at<s32>(cache, 0x1864 + 4 * i) += budget;
                budget = 0;
            }
        }
        at<s32>(cache, 0x1860) = at<s32>(cache, 0x1874);
        at<s32>(cache, 0x1874) = 0;
    }
    for (s32 i = 0; i < 6; ++i)
        debugLog("%d Textures created of size %d", at<s32>(cache, 0x1800 + 4 * i), sides[i]);
    s32 first = 0;
    while (first < 6 && at<s32>(cache, 0x1800 + 4 * first) == 0) ++first;
    at<s32>(cache, 0x18ec) = sides[first];   // (with none, the dword after the table: sic)
    return 1;
}

u32 __fastcall TexCache_ChooseBudget(u8 *cache) {
    const s32 *const sizes = reinterpret_cast<const s32 *>(0x007022d8);
    const s32 *const limits = reinterpret_cast<const s32 *>(0x00702300);
    s32 memory = (s32)vcall<u32>(at<u8 *>(nullptr, 0x008bd52c), 8);
    u32 added = 1;
    s32 count;
    if (memory == 0) {
        const s32 budget = at<s32>(nullptr, (u32)&gState_8c1620);
        at<s32>(cache, 0x1860) = budget ? budget : 0x2000000;
        added = TexCache_AddSurfaces(cache, nullptr, 0x100, &count, at<u32>(nullptr, 0x008bd554), 0);
        memory = (s32)((u32)sizes[at<u32>(nullptr, 0x008bd554)] * (u32)count << 16);
        debugLog("Amount of texture space safely allocated %x", memory);
        Object_FreeSixPointerArrays(cache);
    } else {
        count = (memory >> 16) / sizes[at<u32>(nullptr, 0x008bd554)];
        if (count > 0x2000000) count = 0x2000000;
    }
    at<s32>(nullptr, 0x008c1624) = memory;
    if (const s32 fixed = at<s32>(cache, 0x1920)) {
        at<s32>(cache, 0x18f4) = fixed;
    } else {
        s32 &level = at<s32>(cache, 0x18f4);
        level = 0;
        while (count > limits[2 * level] && ++level < 3) {
        }
        at<s32>(cache, 0x18f8) = limits[2 * level + 1];
    }
    if (!added || count < 0x40 / sizes[at<u32>(nullptr, 0x008bd554)]) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb075400f);
        return 0;
    }
    u32 &flags = at<u32>(cache, 0x18cc);
    if ((flags & 2) && count > 0x40 && at<u32>(nullptr, 0x008bd524) == 1) {
        debugLog("Mipmaps per texture %d", at<u32>(cache, 0x18e8));
        flags |= 2;   // (and a count of 3 / 4 kept, never read: sic)
    } else {
        if (flags & 2) debugLog("NOTICE not enough space for mipmaps, creating surfaces without");
        flags &= ~2u;
        at<u32>(cache, 0x18e8) = 0;
        at<u32>(cache, 0x18e4) = 0;
    }
    const X87Reg total = (double)memory;
    for (u32 i = 0; i < 6; ++i) at<u32>(cache, 0x1894 + 4 * i) = 0;
    for (s32 i = 5; i >= 0; --i)
        at<s32>(cache, 0x1860 + 4 * i) = (s32)(long long)(X87Reg(at<float>(cache, 0x187c + 4 * i)) * total).v;
    return 1;
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

s32 __fastcall TexCacheEntry_Score(u8 *entry, void *, u32) {
    u8 *const surface = at<u8 *>(entry, 0);
    const s32 area = reinterpret_cast<s32(__fastcall *)(u8 *)>(at<void **>(surface, 0)[3])(surface);
    at<s32>(entry, 8) = area;
    at<s32>(entry, 0xc) = 0;
    at<s32>(entry, 0x1c) = 0;
    const u8 *item = at<const u8 *>(entry, 4);
    bool keep = false, filtered = false, whole = item != nullptr;
    s32 tagged = 0, count = 0, sum = 0, weighted = 0;
    for (; item; item = Next(item)) {
        if (at<u32>(item, 8) & 0x4000)
            keep = true;
        const bool filter = Texture_UsesDriverFilter(item);
        const u8 low = at<u8>(item, 8);
        if (filter ? (low & 0x30) && at<s16>(item, 0x14) > at<s32>(nullptr, 0x00702564) : (low & 0x30) != 0)
            whole = false;
        if (Texture_UsesDriverFilter(item))
            filtered = true;
        const s32 itemArea = (s32)at<s16>(item, 0x14) * (s32)at<s16>(item, 0x16);
        if (at<u32>(item, 8) & 0x40000000)
            at<s32>(entry, 0xc) += itemArea;
        const u32 flags = at<u32>(item, 8);
        if (flags & 0x10000000)
            ++tagged;
        sum += itemArea;
        weighted += at<s32>(item, at<u32>(at<const u8 *>(item, 0), 4) + 0x10) * itemArea;
        if (flags & 0x4000)
            at<s32>(entry, 0x1c) += itemArea;
        ++count;
    }
    TexCacheItem_SetFlags(surface, nullptr, (keep ? 1 : 0) | (filtered ? 2 : 0) | (whole ? 4 : 0));
    at<s32>(entry, 8) -= sum;
    const s32 age = at<s32>(nullptr, 0x008bcc40) - at<s32>(entry, 0x14);
    const s32 fresh = age < 0x10 ? 0x10 - age : 0;
    const s32 score = weighted + area * fresh + 2 * (count + 2 * (at<s32>(entry, 0xc) + 2 * (sum + 2 * tagged)));
    at<s32>(entry, 0x10) = score;
    return score;
}

u32 __fastcall TexCache_PlaceTexture(u8 *cache, void *, u8 *entry, const s32 *place, u8 *texture) {
    if (!texture)
        texture = reinterpret_cast<u8 *>((uintptr_t)(u32)place[4]);
    u8 *const surface = at<u8 *>(entry, 0);
    void **const slots = at<void **>(surface, 0);
    reinterpret_cast<u32(__fastcall *)(u8 *, void *, u8 *, s32, s32)>(slots[7])(surface, nullptr, texture, place[0], place[1]);
    s32 x = place[0], y = place[1], w = at<s16>(texture, 0x14), h = at<s16>(texture, 0x16);
    const u32 flags0 = at<u32>(texture, 8);
    if (flags0 & 0x80000000) {
        const s32 maxW = at<s16>(texture, 0x10);
        if (w > maxW) {
            x += (w - maxW) >> 1;
            w = maxW;
        }
        const s32 maxH = at<s16>(texture, 0x12);
        if (h > maxH) {
            y += (h - maxH) >> 1;
            h = maxH;
        }
        at<u32>(texture, 8) = flags0 & 0x7fffffff;
    }
    at<u32>(texture, 0xc) = at<u32>(at<const u8 *>(nullptr, 0x0087a248), 4);
    at<u32>(texture, 8) = (at<u32>(texture, 8) & ~0x2000u) | 0x1000;
    at<u8 *>(texture, 0x18) = entry;
    const u32 flags = at<u32>(texture, 8);
    const auto value = [](u32 address) { return X87Reg(at<float>(nullptr, address)); };
    if (at<u32>(nullptr, 0x008bd524) == 4) {
        const bool filtered = (flags & 0x30000) != 0, alt = (flags & 0x4000000) != 0;
        const X87Reg b = X87_Through(at<float>(nullptr, filtered ? (flags & 0x10000 ? 0x00702474 : 0x008c1628) : alt ? 0x008c1630 : 0x008c1628));
        const X87Reg a = value(filtered ? (flags & 0x20000 ? 0x00702474 : 0x008c1628) : alt ? 0x008c1634 : 0x008c1628);
        at<float>(texture, 0x28) = (float)b;
        at<float>(texture, 0x2c) = (float)a;
        at<float>(texture, 0x20) = (float)(X87Reg((double)w) - (b + b));
        at<float>(texture, 0x24) = (float)(X87Reg((double)h) - (a + a));
        Texture_UpdateWrapMask(texture);
        const u32 offset = ((u32)at<s32>(surface, 0x10) * (u32)place[1] + (u32)place[0]) << 2;
        at<u32>(texture, 0x1c) = offset + reinterpret_cast<u32(__fastcall *)(u8 *)>(at<void **>(surface, 0)[2])(surface);
    } else {
        const X87Reg scale = at<float>(cache, 0x1900);
        const X87Reg width = (double)at<s32>(surface, 8), height = (double)at<s32>(surface, 0xc);
        if ((flags & 0x30000) && (!(flags & 0x400) || at<u32>(texture, 0x3c))) {
            const X87Reg a = value(flags & 0x10000 ? 0x00702474 : 0x008c1628);
            const X87Reg b = value(flags & 0x20000 ? 0x00702474 : 0x008c1628);
            const X87Reg p = X87_DivR(width, scale) * a;
            const X87Reg q = X87_DivR(height, scale) * b;
            at<float>(texture, 0x28) = (float)p;
            at<float>(texture, 0x2c) = (float)q;
            at<float>(texture, 0x20) = (float)X87_SubR(p + p, X87Reg(1.0));
            at<float>(texture, 0x24) = (float)X87_SubR(q + q, X87Reg(1.0));
        } else {
            const bool alt = (flags & 0x4000000) != 0;
            const X87Reg c = value(alt ? 0x008c1634 : 0x008c1628), d = value(alt ? 0x008c1638 : 0x00702474);
            const X87Reg e = alt ? value(0x008c163c) : X87Reg(0.0), f = value(alt ? 0x008c1630 : 0x008c1628);
            const X87Reg down = (e + c) + X87Reg((double)y), across = (d + f) + X87Reg((double)x);
            at<float>(texture, 0x2c) = (float)(down / height * scale);
            at<float>(texture, 0x28) = (float)(across / width * scale);
            at<float>(texture, 0x24) = (float)((X87Reg((double)h) - (c + c)) / height * scale);
            at<float>(texture, 0x20) = (float)((X87Reg((double)w) - (f + f)) / width * scale);
        }
        at<u32>(texture, 0x1c) = reinterpret_cast<u32(__fastcall *)(u8 *)>(at<void **>(surface, 0)[2])(surface);
    }
    at<u32>(texture, 0xc) = at<u32>(nullptr, 0x008bcc40);
    return 1;
}
}

namespace {

const TexCacheTraceEnv kGameTraceEnv = {reinterpret_cast<void *(__cdecl *)(const char *, const char *)>((uintptr_t)0x0067b8a0),
                                        reinterpret_cast<u32(__cdecl *)(const void *, u32, u32, void *)>((uintptr_t)0x0067b6d0),
                                        reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067d070),
                                        reinterpret_cast<s32(__cdecl *)(void *)>((uintptr_t)0x0067b620)};
const TexCacheTraceEnv *g_traceEnv = &kGameTraceEnv;

#pragma pack(push, 1)
struct TraceRecord {
    u8 tag;
    const u8 *item;
    u32 flags;
    u16 width, height;
    u32 weight;
};
#pragma pack(pop)

}  // namespace

void TexCache_SetTraceEnv(const TexCacheTraceEnv *e) { g_traceEnv = e ? e : &kGameTraceEnv; }

extern "C" {

void __fastcall TexCache_TraceItem(u8 *cache, void *, const u8 *item) {
    void *file;
    if (at<u32>(cache, 0x18cc) & 0x40000000) {
        file = g_traceEnv->fopen("c:\\tcache.log", "a");
    } else {
        at<u32>(cache, 0x18cc) |= 0x40000000;
        file = g_traceEnv->fopen("c:\\tcache.log", "w+b");
        if (!file)
            return;
        g_traceEnv->fwrite(&global<u8>(0x008bd550), 1, 0x40f0, file);
        g_traceEnv->fwrite(&global<u32>(0x008bd508), 1, 4, file);
        g_traceEnv->fwrite(&global<u32>(0x008bcbc8), 1, 4, file);
    }
    if (!file)
        return;
    TraceRecord record;
    record.tag = 1;
    record.item = item;
    record.flags = at<u32>(item, 8);
    record.width = at<u16>(item, 0x10);
    record.height = at<u16>(item, 0x12);
    record.weight = X87_FloatThrough(at<u32>(item, 0x30));
    g_traceEnv->fwrite(&record.tag, 1, 1, file);
    g_traceEnv->fwrite(&record.item, 1, 0x10, file);
    g_traceEnv->fflush(file);
    g_traceEnv->fclose(file);
}

void __fastcall TexCache_TouchItem(u8 *cache, void *, u8 *item, float weight) {
    if (!item || at<u32>(item, 8) & 0x2000000)
        return;
    const u32 before = at<u32>(item, 8);
    if (before & 0x400 && at<u32>(item, 0x3c) && at<u32>(global<u8 *>(0x0087a248), 4) != at<u32>(item, 0xc) && before & 0x1000)
        TexCache_UnlinkItem(cache, nullptr, item);
    at<u32>(item, 8) |= 0x40000000;
    u8 *const base = item + at<s32>(at<u8 *>(item, 0), 4);
    u8 &slot = base[global<s32>(0x00879d48)];
    if (slot < 0xff) {
        slot++;
        at<u32>(base, 0x10)++;
    }
    at<u32>(base, 0x14) |= 0x8000;
    const u32 flags = at<u32>(item, 8);
    if (!(flags & 0x3000)) {
        at<float>(item, 0x30) = weight;
        const u32 cleared = (flags | 0x2000) & ~0x30u;
        at<u32>(item, 8) = ((u32)((s32)cleared >> 2) & 0x30) | cleared;
        const u16 mask = (u16)at<u32>(cache, 0x18e4);
        at<u16>(item, 0x14) = (u16)((at<u16>(item, 0x10) + mask) & ~mask);
        at<u16>(item, 0x16) = (u16)((at<u16>(item, 0x12) + mask) & ~mask);
        TexCacheItem_SetKey(item);
        at<u8 *>(item, 4) = at<u8 *>(cache, 0x18c4);
        at<u8 *>(cache, 0x18c4) = item;
    } else {
        if (at<float>(item, 0x30) > weight)
            at<float>(item, 0x30) = weight;
        if (global<u32>(0x008bd508) & 0xc0000000 && flags & 0x1000 && flags & 0x4000)
            TexCacheItem_SetFlag1(at<u8 *>(at<u8 *>(item, 0x18), 0));
    }
    if (at<u32>(item, 8) & 0x800000)
        TexCache_TouchItem(cache, nullptr, item + 0x60, weight);
    if (global<u8>(0x008bd550) & 0x80)
        TexCache_TraceItem(cache, nullptr, item);
}
}

namespace {

// The entries' pointers by a signed key at this offset, the smallest first.
template <u32 Key>
void sortEntries(u8 **entries, s32 first, s32 last) {
    const auto key = [](const u8 *entry) { return *reinterpret_cast<const s32 *>((uintptr_t)entry + Key); };
    for (;;) {
        s32 left = first, right = last;
        const s32 pivot = key(entries[(first + last) >> 1]);
        do {
            while (key(entries[left]) < pivot) ++left;
            while (key(entries[right]) > pivot) --right;
            if (left > right) break;
            u8 *const moved = entries[left];
            entries[left] = entries[right];
            entries[right] = moved;
            ++left;
            --right;
        } while (left <= right);
        if (first < right) sortEntries<Key>(entries, first, right);
        if (last <= left) return;
        first = left;
    }
}

// The chain's first 0x1000 items into the array, the rest losing flags 0xf030; their number.
s32 listToArray(u8 *const *list, u8 **items) {
    u8 *item = *list;
    s32 count = 0;
    while (item && count < 0x1000) {
        items[count++] = item;
        item = *reinterpret_cast<u8 **>(item + 4);
    }
    if (count == 0x1000) {
        for (; item; item = *reinterpret_cast<u8 **>(item + 4)) *reinterpret_cast<u32 *>(item + 8) &= 0xffff0fcf;
    }
    return count;
}

}  // namespace

void __fastcall TexCache_SortEntriesByKept(u8 *, void *, u8 **entries, s32 first, s32 last) { sortEntries<0xc>(entries, first, last); }

void __fastcall TexCache_SortEntriesByScore(u8 *, void *, u8 **entries, s32 first, s32 last) { sortEntries<0x10>(entries, first, last); }

void __fastcall TexCache_SortItemsByKey(u8 *, void *, u8 **items, s32 first, s32 last) {
    if (first < last) RecordSort_RunPointersUp(reinterpret_cast<RecordSort *>(0x008bcb98), nullptr, items, reinterpret_cast<u8 *>(&first), 0x60, 0x38, first, last);
}

void __fastcall TexCache_SortListByKey(u8 *cache, void *, u8 **list) {
    u8 *items[0x1000];
    const s32 count = listToArray(list, items);
    TexCache_SortItemsByKey(cache, nullptr, items, 0, count - 1);
    u8 *head = nullptr;
    for (s32 i = 0; i < count; ++i) {
        *reinterpret_cast<u8 **>(items[i] + 4) = head;
        head = items[i];
    }
    *list = head;
}

void __fastcall TexCache_SortListByAddress(u8 *, void *, u8 **list) {
    u8 *items[0x1000];
    u32 temp;
    const s32 count = listToArray(list, items);
    if (count > 1) RecordSort_RunUp(reinterpret_cast<RecordSort *>(0x008bcb98), nullptr, reinterpret_cast<u8 *>(items), reinterpret_cast<u8 *>(&temp), 4, 0, 0, count - 1);
    u8 *head = nullptr;
    for (s32 i = count - 1; i >= 0; --i) {
        *reinterpret_cast<u8 **>(items[i] + 4) = head;
        head = items[i];
    }
    *list = head;
}

namespace {

s32 &used(const u8 *cache, s32 size) { return at<s32>(cache, 0x1830 + size * 4); }
s32 &split(const u8 *cache, s32 size) { return at<s32>(cache, 0x1818 + size * 4); }
s32 &entryCount(const u8 *cache, s32 size) { return at<s32>(cache, 0x1800 + size * 4); }
u8 **entriesOf(u8 *cache, s32 size) { return reinterpret_cast<u8 **>(cache + 0x400 * size); }

// The item can come down a level: bits 4-5 of its flags set and, with the driver filter, wider than the smallest (0x702434).
bool canHalve(const u8 *item) {
    if (Texture_UsesDriverFilter(item))
        return (at<u8>(item, 8) & 0x30) && at<s16>(item, 0x14) > global<s32>(0x00702434);
    return (at<u8>(item, 8) & 0x30) != 0;
}

// One level down: flags - 0x10, the current sizes halved upward, the key again (inlined in places). With the driver filter at the smallest width: nothing.
void halveItem(u8 *item) {
    if (Texture_UsesDriverFilter(item) && at<s16>(item, 0x14) == global<s32>(0x00702434))
        return;
    at<u32>(item, 8) -= 0x10;
    const s32 height = at<s16>(item, 0x16);
    at<u16>(item, 0x14) = (u16)(((s32)at<s16>(item, 0x14) + 1) >> 1);
    at<u16>(item, 0x16) = (u16)((height + 1) >> 1);
    TexCacheItem_SetKey(item);
}

// Back to the wanted size: flags 0xf030 cleared, bits 6-7 copied to 4-5, the current sizes the wanted ones rounded up to 8, the key again (inlined).
void resetItem(u8 *item) {
    const u32 kept = at<u32>(item, 8) & 0xffff0fcf;
    at<u32>(item, 8) = kept | ((u32)((s32)kept >> 2) & 0x30);
    at<u16>(item, 0x14) = (u16)((at<u16>(item, 0x10) + 7) & ~7);
    at<u16>(item, 0x16) = (u16)((at<u16>(item, 0x12) + 7) & ~7);
    TexCacheItem_SetKey(item);
}

// The entry's items off it and those with flag 0x40000000 on the pending list (flag 0x2000, 0x4000 kept); its surface's flags 0.
void releaseEntry(u8 *cache, u8 *entry) {
    u8 *item = at<u8 *>(entry, 4);
    u8 *const surface = at<u8 *>(entry, 0);
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
    TexCacheItem_SetFlags(surface, nullptr, 0);
}

// A place in the short list of 16 entries (slots 0-15) and their keys (slots 16-31); slot 32 is the next local. The move up is a forward
// copy, so the old value at the place fills all up to cap, and at cap 16 it runs on into the keys and past them (sic).
void shortListPut(u32 *slots, s32 &count, s32 cap, s32 place, const u8 *entry, s32 key) {
    for (s32 i = 0; i != cap - place; ++i)
        slots[place + 1 + i] = slots[place + i];
    for (s32 i = 0; i != cap - place; ++i)
        slots[16 + place + 1 + i] = slots[16 + place + i];
    if (count < cap)
        ++count;
    slots[place] = (u32)(uintptr_t)entry;
    slots[16 + place] = (u32)key;
}

u8 *slotEntry(u32 *slots, s32 i) { return reinterpret_cast<u8 *>((uintptr_t)slots[i]); }

}  // namespace

void __fastcall TexCache_TraceUpdate(u8 *) {
    const u32 tag = 0x10;
    void *const file = g_traceEnv->fopen("c:\\tcache.log", "a");
    if (!file)
        return;
    g_traceEnv->fwrite(&tag, 1, 1, file);
    g_traceEnv->fflush(file);
    g_traceEnv->fclose(file);
}

void __fastcall TexCache_RankEntries(u8 *cache) {
    at<u32>(cache, 0x18d0) = 0;
    at<u32>(cache, 0x18d4) = 0;
    at<u32>(cache, 0x18d8) = 0;
    for (s32 size = 0; size < 6; ++size) {
        u8 **const entries = entriesOf(cache, size);
        for (s32 i = 0; i < entryCount(cache, size); ++i) {
            if ((at<u8>(cache, 0x18cc) & 1) || !TexCacheItem_Flag1(at<u8 *>(entries[i], 0)))
                TexCacheEntry_Score(entries[i], nullptr, at<u32>(cache, 0x18cc));
        }
        s32 ranked;
        if (at<u8>(cache, 0x18cc) & 1) {
            if (entryCount(cache, size))
                TexCache_SortEntriesByKept(cache, nullptr, entries, 0, entryCount(cache, size) - 1);
            ranked = entryCount(cache, size);
        } else {
            const s32 count = entryCount(cache, size);
            s32 lo = 0, hi = count - 1;
            if (hi > 0) {
                do {
                    while (lo < hi && !TexCacheItem_Flag1(at<u8 *>(entries[lo], 0)))
                        ++lo;
                    while (lo < hi && TexCacheItem_Flag1(at<u8 *>(entries[hi], 0)))
                        --hi;
                    u8 *const moved = entries[lo];
                    entries[lo] = entries[hi];
                    entries[hi] = moved;
                    ++lo;
                    --hi;
                } while (lo < hi);
                if (hi > 0)
                    TexCache_SortEntriesByKept(cache, nullptr, entries, 0, hi);
            }
            ranked = hi < count ? hi + 1 : hi;
        }
        split(cache, size) = ranked;
        s32 last = ranked - 1;
        if (last > 0) {
            while (last > 0) {
                const u8 *const entry = entries[last];
                const u32 kept = at<u32>(entry, 0xc);
                if ((s32)(kept * 2) <= vcall<s32>(at<u8 *>(entry, 0), 0xc))
                    break;
                --last;
            }
            if (last > 0)
                TexCache_SortEntriesByScore(cache, nullptr, entries, 0, last);
        }
        split(cache, size) = last;
    }
    at<u32>(cache, 0x18d0) = 0;
    at<u32>(cache, 0x18d4) = 0;
    at<u32>(cache, 0x18d8) = 0;
    for (s32 size = 0; size < 6; ++size) {
        u8 **const entries = entriesOf(cache, size);
        for (s32 i = 0; i < entryCount(cache, size); ++i) {
            u8 *const surface = at<u8 *>(entries[i], 0);
            if (!(at<u8>(cache, 0x18cc) & 1) && TexCacheItem_Flag1(surface))
                continue;
            TexCacheEntry_Score(entries[i], nullptr, at<u32>(cache, 0x18cc));
            at<u32>(cache, 0x18d0) += at<u32>(entries[i], 8);
            at<u32>(cache, 0x18d4) += at<u32>(entries[i], 0xc);
            at<u32>(cache, 0x18d8) += vcall<u32>(surface, 0xc);
        }
    }
}

void __fastcall TexCache_ArrangePending(u8 *cache) {
    u8 *items[0x1000];
    u32 temp;
    RecordSort *const sort = reinterpret_cast<RecordSort *>(0x008bcb98);
    double top = 0.0f;   // 0x7023f8
    s32 count = 0;
    u8 *item = at<u8 *>(cache, 0x18c4);
    while (item && count < 0x1000) {
        items[count++] = item;
        const double lod = at<float>(item, 0x30);
        if (!(top >= lod))   // (unordered too)
            top = lod;
        item = at<u8 *>(item, 4);
    }
    for (; item; item = at<u8 *>(item, 4))
        resetItem(item);
    const X87Reg high = X87Reg(top) - -0.001f;   // 0x7023fc
    const X87Reg span = high - -0.001f;
    at<u32>(cache, 0x1904) = 0xba83126f;   // (-0.001)
    at<float>(cache, 0x1914) = (float)high;
    at<float>(cache, 0x1908) = (float)X87_SubR(span * -0.125f, -0.001f);
    at<float>(cache, 0x190c) = (float)X87_SubR(span * -0.25f, -0.001f);
    at<float>(cache, 0x1910) = (float)X87_SubR(span * -0.5f, -0.001f);
    if (count > 1)
        RecordSort_RunPointersByFloat(sort, nullptr, items, reinterpret_cast<u8 *>(&temp), 0x60, 0x30, 0, count - 1);
    at<u32>(cache, 0x18dc) = 0;
    for (s32 i = 0; i < count; ++i)
        at<s32>(cache, 0x18dc) += (s32)at<s16>(items[i], 0x12) * at<s16>(items[i], 0x10);
    for (s32 tier = 0; tier < at<s32>(cache, 0x18f8); ++tier)
        TexCache_ShrinkItems(cache, nullptr, items, count, tier, (s32)0x80000000);
    const s32 room = at<s32>(cache, 0x18d8) - at<s32>(cache, 0x18d4);
    const s32 area = at<s32>(cache, 0x18dc);
    const bool tight = (s32)(long long)(X87Reg((double)area) * 1.5f).v > room;   // 0x7023c0
    if (((at<u8>(cache, 0x18cc) & 4) && area > at<s32>(cache, 0x18fc)) || tight) {
        s32 tier = at<s32>(cache, 0x18f8);
        if (tier != 0xc) {
            for (;;) {
                TexCache_ShrinkItems(cache, nullptr, items, count, tier, room);
                const s32 now = at<s32>(cache, 0x18dc);
                if (now <= room && (!(at<u8>(cache, 0x18cc) & 4) || now <= at<s32>(cache, 0x18fc)))
                    break;
                if (++tier >= 0xc)
                    break;
            }
        }
        if (at<u8>(cache, 0x18cc) & 4) {
            for (s32 i = count - 1; i >= 0 && at<s32>(cache, 0x18dc) > at<s32>(cache, 0x18fc); --i) {
                if (at<u32>(items[i], 8) & 0x80000)
                    continue;
                at<u32>(items[i], 8) |= 0x8000;
                at<s32>(cache, 0x18dc) -= (s32)at<s16>(items[i], 0x16) * at<s16>(items[i], 0x14);
            }
        }
    }
    if (count - 1 > 0)
        RecordSort_RunPointersUp(sort, nullptr, items, reinterpret_cast<u8 *>(&temp), 0x60, 0x38, 0, count - 1);
    at<u8 *>(cache, 0x18c4) = nullptr;
    for (s32 i = 0; i < count; ++i) {
        if (at<u32>(items[i], 8) & 0x8000) {
            resetItem(items[i]);
        } else {
            at<u8 *>(items[i], 4) = at<u8 *>(cache, 0x18c4);
            at<u8 *>(cache, 0x18c4) = items[i];
        }
    }
}

u32 __fastcall TexCache_Rebalance(u8 *cache) {
    if (global<u32>(0x008bd524) != 1)
        return 1;
    bool changed = false;
    float shares[6];
    for (s32 size = 0; size < 6; ++size)
        shares[size] = (float)(X87Reg((double)split(cache, size)) / X87Reg((double)entryCount(cache, size)));
    for (s32 size = 0; size < 6; ++size) {
        const float share = shares[size];
        if (share >= 0.67f)   // 0x70246c (a NaN goes on)
            continue;
        const s32 side = reinterpret_cast<const s32 *>((uintptr_t)0x00702420)[size];
        const X87Reg more = X87Reg(share) * 1.5f;   // 0x702460
        const bool belowSmaller = size > 0 && !(more.v >= shares[size - 1]);
        const bool belowLarger = size < 5 && !(more.v >= shares[size + 1]);
        u32 slots[33];
        if (belowSmaller) {
            s32 cap = 1 - (s32)(long long)((X87Reg(share) - shares[size - 1]) * -0.25f).v;   // 0x702470
            if (cap > 16)
                cap = 16;
            s32 listed = 0;
            const s32 start = split(cache, size) < 0 ? 0 : split(cache, size);
            u8 **const entries = entriesOf(cache, size);
            for (s32 i = entryCount(cache, size) - 1; i >= start; --i) {
                const u8 *const entry = entries[i];
                const u8 *const surface = at<u8 *>(entry, 0);
                if (TexCacheItem_Flag1(surface) && !(at<u8>(cache, 0x18cc) & 1))
                    continue;
                if (TexCacheItem_Flag2(surface))
                    continue;
                s32 wanted = 0;
                for (const u8 *item = at<u8 *>(entry, 4); item; item = at<u8 *>(item, 4)) {
                    if (at<u32>(item, 8) & 0x40000000)
                        ++wanted;
                }
                s32 place = listed;
                while (place > 0 && wanted < (s32)slots[16 + place - 1])
                    --place;
                shortListPut(slots, listed, cap, place, entry, wanted);
            }
            for (s32 i = listed - 1; i >= 0; --i) {
                releaseEntry(cache, slotEntry(slots, i));
                changed = true;
            }
        }
        if (!belowLarger)
            continue;
        s32 cap = 1 - (s32)(long long)((X87Reg(share) - shares[size + 1]) * -0.25f).v;
        if (cap > 16)
            cap = 16;
        s32 listed = 0;
        const s32 start = split(cache, size) < 0 ? 0 : split(cache, size);
        u8 **const entries = entriesOf(cache, size);
        for (s32 i = entryCount(cache, size) - 1; i >= start; --i) {
            const u8 *const entry = entries[i];
            const u8 *const surface = at<u8 *>(entry, 0);
            if (TexCacheItem_Flag1(surface) && !(at<u8>(cache, 0x18cc) & 1))
                continue;
            if (TexCacheItem_Flag2(surface)) {
                if (TexCacheItem_Flag4(surface))
                    continue;
            } else if ((s32)(at<u32>(entry, 0xc) * 2) >= side * side) {
                continue;
            }
            const u8 *const item = at<u8 *>(entry, 4);
            s32 key = 0;
            if (item)
                key = (s32)(long long)((1.5f - X87Reg(at<float>(item, 0x30))) * 2047.0f).v + (s32)((at<u32>(item, 8) & 0x30) << 8);
            s32 place = listed;
            while (place > 0 && key > (s32)slots[16 + place - 1])
                --place;
            shortListPut(slots, listed, cap, place, entry, key);
        }
        for (s32 i = listed - 1; i >= 0; --i) {
            u8 *const entry = slotEntry(slots, i);
            for (u8 *item = at<u8 *>(entry, 4); item; item = at<u8 *>(item, 4)) {
                if (canHalve(item))
                    halveItem(item);
            }
            releaseEntry(cache, entry);
            changed = true;
        }
    }
    if (changed) {
        TexCache_RankEntries(cache);
        TexCache_ArrangePending(cache);
    }
    return 1;
}

s32 __fastcall TexCache_PlaceNext(u8 *cache, void *, u8 **list) {
    struct Row {
        u32 x, y, w, h, item;
    };
    Row rows[0x80], spare[0x80];
    u8 *merged[0x1000];
    u8 *const head = *list;
    s32 size = Texture_UsesDriverFilter(head) ? (s32)Texture_Level(head) : 0;
    if (used(cache, size) >= split(cache, size)) {
        while (size < 6) {
            const s32 taken = used(cache, size + 1), last = split(cache, size + 1);
            ++size;
            if (taken < last)
                break;
        }
    }
    if (size == 6)
        return -1;
    const s32 first = size;
    u8 *entry = entriesOf(cache, size)[used(cache, size)];
    u8 *surface = at<u8 *>(entry, 0);
    if (Texture_UsesDriverFilter(head)) {
        const s32 width = at<s32>(surface, 8);
        while (canHalve(head) && width < at<s16>(head, 0x14)) {
            halveItem(head);
            ++global<u32>(0x008c4b70);
        }
        if (width < at<s16>(head, 0x14))
            return -1;
        if (canHalve(head) && size < 5) {
            const X87Reg here = X87Reg((double)(split(cache, size) - used(cache, size))) / X87Reg((double)entryCount(cache, size));
            const X87Reg there = X87Reg((double)(split(cache, size + 1) - used(cache, size + 1))) / X87Reg((double)entryCount(cache, size + 1));
            if (there.v >= here.v) {
                halveItem(head);
                ++size;
                entry = entriesOf(cache, size)[used(cache, size)];
                surface = at<u8 *>(entry, 0);
            }
        }
        *list = at<u8 *>(head, 4);
        TexCache_RequeueItems(cache, nullptr, list, at<u8 *>(entry, 4));
        at<u8 *>(entry, 4) = nullptr;
        ++used(cache, size);
        TexCacheItem_SetFlags(surface, nullptr, 0);
        if (!vcall<u32>(surface, 0x10))
            return 0;
        const s32 place[5] = {0, 0, at<s32>(surface, 8), at<s32>(surface, 0xc), (s32)(uintptr_t)head};
        if (!TexCache_PlaceTexture(cache, nullptr, entry, place, nullptr))
            return 0;
        at<u8 *>(entry, 4) = head;
        at<u8 *>(head, 4) = nullptr;
        u32 flags = ((at<u32>(head, 8) & 0x4000) | 0x8000) >> 0xe;
        if (!canHalve(head))
            flags |= 4;
        TexCacheItem_SetFlags(surface, nullptr, flags);
        return (s32)Texture_Reload(entry);
    }
    const u32 areaSum = TexCache_MergeItems(reinterpret_cast<u32 *>(merged), head, entry);
    u32 placed;
    u32 whole = TexCache_PackItems(cache, nullptr, merged, reinterpret_cast<u8 *>(rows), &placed, reinterpret_cast<const s32 *>(surface), 1);
    if (!placed) {
        TexCache_SortListByKey(cache, nullptr, list);
        return -1;
    }
    s32 chosen = size;
    while (whole) {
        whole = 0;
        if ((at<s32>(surface, 8) >> 1) < at<s32>(cache, 0x1918) || (at<s32>(surface, 0xc) >> 1) < at<s32>(cache, 0x191c))
            break;
        ++size;
        ++global<u32>(0x008c4b5c);
        if (used(cache, size) >= split(cache, size)) {
            while (size < 6) {
                const s32 taken = used(cache, size + 1), last = split(cache, size + 1);
                ++size;
                if (taken < last)
                    break;
            }
        }
        if (size >= 6)
            break;
        u8 *const smaller = entriesOf(cache, size)[used(cache, size)];
        u8 *const smallerSurface = at<u8 *>(smaller, 0);
        TexCache_MergeItems(reinterpret_cast<u32 *>(merged), head, smaller);
        const u32 counted = global<u32>(0x008c4b6c);
        u32 count;
        whole = TexCache_PackItems(cache, nullptr, merged, reinterpret_cast<u8 *>(spare), &count, reinterpret_cast<const s32 *>(smallerSurface), 0);
        if (!whole) {
            global<u32>(0x008c4b6c) = counted;
            break;
        }
        for (u32 i = 0; i != count + 1; ++i)
            rows[i] = spare[i];
        placed = count;
        chosen = size;
        entry = smaller;
        surface = smallerSurface;
    }
    if (at<u32>(cache, 0x1924))
        TexCache_SortListByKey(cache, nullptr, list);
    if (first != chosen) {
        chosen = TexCache_ChooseSize(cache, nullptr, chosen, first, (s32)areaSum);
        entry = entriesOf(cache, chosen)[used(cache, chosen)];
        surface = at<u8 *>(entry, 0);
    }
    TexCache_RequeueItems(cache, nullptr, list, at<u8 *>(entry, 4));
    at<u8 *>(entry, 4) = nullptr;
    const s32 rowCount = (s32)placed;
    for (s32 a = 0; a < rowCount - 1; ++a) {
        for (s32 b = a + 1; b < rowCount; ++b) {
            if (rows[a].item > rows[b].item) {
                const Row moved = rows[a];
                rows[a] = rows[b];
                rows[b] = moved;
            }
        }
    }
    ++used(cache, chosen);
    TexCacheItem_SetFlags(surface, nullptr, 0);
    if (!vcall<u32>(surface, 0x10))
        return 0;
    TexCache_SortListByAddress(cache, nullptr, list);
    const Row *row = rows;
    u8 *before = nullptr, *tail = nullptr;
    bool none = true;
    for (u8 *item = *list; item && row->item;) {
        u8 *const next = at<u8 *>(item, 4);
        if ((u32)(uintptr_t)item == row->item) {
            if (canHalve(item))
                none = false;
            if (at<u32>(item, 8) & 0x4000)
                TexCacheItem_SetFlag1(surface);
            if (!TexCache_PlaceTexture(cache, nullptr, entry, reinterpret_cast<const s32 *>(row), nullptr)) {
                vcall<void>(surface, 0x14);
                return 0;
            }
            if (before)
                at<u8 *>(before, 4) = next;
            else
                *list = at<u8 *>(*list, 4);
            if (tail)
                at<u8 *>(tail, 4) = item;
            else
                at<u8 *>(entry, 4) = item;
            at<u8 *>(item, 4) = nullptr;
            --global<u32>(0x008c4b6c);
            ++row;
            tail = item;
        } else {
            before = item;
        }
        item = next;
    }
    if (none)
        TexCacheItem_SetFlags(surface, nullptr, at<u32>(surface, 4) | 4);
    if (*list)
        TexCache_SortListByKey(cache, nullptr, list);
    u8 *kept = at<u8 *>(entry, 4);
    TexCache_SortListByKey(cache, nullptr, &kept);
    at<u8 *>(entry, 4) = kept;
    return (s32)Texture_Reload(entry);
}

s32 __fastcall TexCache_PlacePending(u8 *cache) {
    for (s32 size = 0; size < 6; ++size)
        used(cache, size) = 0;
    u8 **const list = &at<u8 *>(cache, 0x18c4);
    s32 result = 1;
    while (*list && result) {
        result = TexCache_PlaceNext(cache, nullptr, list);
        if (result == -1) {
            u8 *const head = *list;
            resetItem(head);
            result = 1;
            *list = at<u8 *>(head, 4);
        }
    }
    for (u8 *item = *list; item; item = at<u8 *>(item, 4))
        resetItem(item);
    *list = nullptr;
    return result;
}

u32 __fastcall TexCache_Update(u8 *cache) {
    if (global<u8>(0x008bd550) & 0x80)
        TexCache_TraceUpdate(cache);
    if (!at<u8 *>(cache, 0x18c4))
        return 1;
    if (global<u32>(0x008bd508) & 0xc0000000)
        TexCache_Evict(cache);
    TexCache_RankEntries(cache);
    TexCache_ArrangePending(cache);
    TexCache_Rebalance(cache);
    if (TexCache_PlacePending(cache))
        return 1;
    EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 2, 0xb0754011);
    return 0;
}

extern "C" u32 __cdecl TexCache_Destroy() {
    u8 *const cache = reinterpret_cast<u8 * &>(gTexcache_8c4b44);
    if (!cache) return 0;
    g_destroyCalls.Th(0x0058dd60, address(cache), 1u);   // TexCache_ReleaseItems
    g_destroyCalls.Th(0x0058d380, address(cache));       // Object_FreeSixPointerArrays
    g_destroyCalls.Th(0x0058b280, 0x008c47d8u);          // SurfacePool_ReleaseAll
    at<u32>(cache, 0x18cc) &= ~0x20000000u;
    g_destroyCalls.Cd(0x0067acb0, gTexcache_8c4b44);
    gTexcache_8c4b44 = 0;
    return 1;
}

extern "C" u32 __fastcall TexCache_ReleaseGraphics(u8 *self) {
    g_destroyCalls.Th(0x0058dd60, gTexcache_8c4b44, 1u);
    g_destroyCalls.Th(0x0058d380, address(self));
    g_destroyCalls.Th(0x0058b280, 0x008c47d8u);
    at<u32>(self, 0x18cc) &= ~0x20000000u;
    return 1;
}

extern "C" u32 __fastcall TexCache_Setup(u8 *self, void *, u32 format) {
    debugLog("\n\n ***** Texture Cache Initialization");
    debugLog("Surfaceformat chosen %d", format);
    if (global<u8>(0x8bd550) & 2) {
        at<u32>(self, 0x18e8) = 4;
        at<u32>(self, 0x18cc) |= 2;
    }
    if (global<u32>(0x8bd524) == 4) {
        global<u32>(0x8bd554) = 3;
    } else {
        global<u32>(0x8bd554) = format;
        if (!g_setupCalls.Th(0x0058b220, 0x008c47d8u, global<u32>(0x7022e8), at<u32>(self, 0x18e8), format)) return 0;   // SurfacePool_MakeLevels
    }
    if (!g_setupCalls.Th(0x0058d400, (u32)(uintptr_t)self)) return 0;   // TexCache_ChooseBudget
    if (!g_setupCalls.Th(0x0058d670, (u32)(uintptr_t)self)) return 0;   // TexCache_CreateSurfaces
    at<u32>(self, 0x18f0) = 0x10;   // (the original first sets 0x40, 0x80 or 0x100 here by +0x1804 and +0x1808: overwritten)
    at<u32>(self, 0x18cc) |= 0x20000000;
    return 1;
}

extern "C" u32 __cdecl TexCache_Create(u32 format) {
    g_setupCalls.Cd(0x0056b630);   // Table879d08_Fill
    if (gTexcache_8c4b44) return 0;
    void *const block = GameHeap_Alloc(0x1928);
    const u32 cache = block ? g_setupCalls.Th(0x0058d0f0, (u32)(uintptr_t)block) : 0;   // TexCache_InitDefaults
    gTexcache_8c4b44 = cache;
    if (cache) return TexCache_Setup(reinterpret_cast<u8 *>((uintptr_t)cache), nullptr, format);
    g_setupCalls.Th(0x0056af20, 0x00879ad0u, 3u, 0xb0754000u);   // EngineErrors_Push
    return 0;
}

extern "C" u32 __fastcall TexCache_ChangeFormat(u8 *self, void *, u32 format) {
    if (at<u32>(self, 0x18cc) & 0x20000000) {
        g_setupCalls.Th(0x0058dd60, gTexcache_8c4b44, 1u);   // TexCache_ReleaseItems
        g_setupCalls.Th(0x0058d380, (u32)(uintptr_t)self);   // Object_FreeSixPointerArrays
        g_setupCalls.Th(0x0058b280, 0x008c47d8u);   // SurfacePool_ReleaseAll
        at<u32>(self, 0x18cc) &= 0xdfffffff;
    }
    if (format != 0xffffffff) {
        global<u32>(0x8bd554) = format;
        if (global<u32>(0x8bd524) == 1 && !g_setupCalls.Th(0x0058b220, 0x008c47d8u, global<u32>(0x7022e8), at<u32>(self, 0x18e8), format)) return 0;
    }
    return g_setupCalls.Th(0x0058d670, (u32)(uintptr_t)self);
}
