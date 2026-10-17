// The surface format helpers: see surface.h.
#include "surface.h"

#include <cstdint>
#include <cstring>

#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../files/file.h"

namespace {

u32 at(const u8 *p, u32 offset) {
    u32 v;
    std::memcpy(&v, p + offset, 4);
    return v;
}
void put(u8 *p, u32 offset, u32 v) { std::memcpy(p + offset, &v, 4); }

template <typename T>
T addr(u32 address) {
    return reinterpret_cast<T>(static_cast<std::uintptr_t>(address));
}
const SurfaceEnv kGameEnv = {addr<void **>(0x00fa76b0), addr<const u8 *>(0x00fa76b4), addr<const void *>(0x00fa71f8)};
const SurfaceEnv *g_env = &kGameEnv;

const s32 kSurfaceLost = static_cast<s32>(0x887601c2);  // DDERR_SURFACELOST
const u32 kDescSize = 0x6c;                             // sizeof(DDSURFACEDESC)

// The colour key in info's pixel format: rgb (0xAARRGGBB) through each channel's shifts.
u32 packKey(const SurfaceInfo *info, u32 rgb) {
    PixelChannel c[4];
    PixelChannel_Set(&c[0], nullptr, info->redMask, 0x17);
    PixelChannel_Set(&c[1], nullptr, info->greenMask, 0xf);
    PixelChannel_Set(&c[2], nullptr, info->blueMask, 7);
    PixelChannel_Set(&c[3], nullptr, info->alphaMask, 0x1f);
    u32 key = 0;
    for (const PixelChannel &p : c) key |= ((rgb >> (p.up & 31)) << (p.down & 31)) & p.mask;
    return key;
}

void setSourceKey(Surface *s, u32 key) {
    u32 ck[2] = {key, key};                           // DDCOLORKEY: low, high
    comcall<s32>(s->surface, 0x74, 8u, ck);           // SetColorKey(DDCKEY_SRCBLT)
}

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

void Surface_SetEnv(const SurfaceEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __fastcall PixelChannel_Set(PixelChannel *c, void *, u32 mask, u32 top) {
    c->mask = mask;
    u32 bit = 31;
    if (mask) {
        for (u32 b = 0x80000000u; !(mask & b); b >>= 1) --bit;
    }
    if (bit >= top) {
        c->up = 0;
        c->down = static_cast<u16>(bit - top);
    } else {
        c->down = 0;
        c->up = static_cast<u16>(top - bit);
    }
}

void __fastcall PixelShifts_Set(PixelShifts *s, void *, const u32 *masks) {
    PixelChannel_Set(&s->alpha, nullptr, masks[3], 0x1f);
    PixelChannel_Set(&s->red, nullptr, masks[0], 0x17);
    PixelChannel_Set(&s->green, nullptr, masks[1], 0xf);
    PixelChannel_Set(&s->blue, nullptr, masks[2], 7);
}

void __fastcall Surface_SetInfo(Surface *s, void *, const u8 *desc) {
    SurfaceInfo &i = s->info;
    i.bits = reinterpret_cast<void *>(static_cast<std::uintptr_t>(at(desc, 0x24)));
    i.width = at(desc, 0xc);
    i.height = at(desc, 0x8);
    i.pitch = static_cast<s32>(at(desc, 0x10));
    i.bitCount = at(desc, 0x54);
    if (i.bitCount == 8) {
        i.paletted = 1;
        return;
    }
    i.paletted = 0;
    i.redMask = at(desc, 0x58);
    i.greenMask = at(desc, 0x5c);
    i.blueMask = at(desc, 0x60);
    i.alphaMask = at(desc, 0x64);
}

SurfaceInfo *__fastcall Surface_Refresh(Surface *s) {
    u8 desc[0x6c];
    const u32 size = sizeof desc;
    std::memcpy(desc, &size, 4);
    if (comcall<s32>(s->surface, 0x58, desc) >= 0) {
        if (!at(desc, 0x24)) std::memcpy(desc + 0x24, &s->info.bits, 4);
        Surface_SetInfo(s, nullptr, desc);
    }
    return &s->info;
}

SurfaceInfo *__fastcall SurfaceInfo_Ctor(SurfaceInfo *i) {
    i->bitCount = 0;
    i->paletted = 0;
    i->bits = nullptr;
    i->width = 0;
    i->height = 0;
    i->pitch = 0;
    return i;
}

SurfaceInfo *__fastcall SurfaceInfo_Init(SurfaceInfo *i, void *, const u32 *size, const u32 *format, void *bits,
                                         s32 pitch) {
    std::memcpy(i, format, 0x18);
    i->bits = bits;
    i->width = size[0];
    i->height = size[1];
    i->pitch = pitch ? pitch : static_cast<s32>(((i->bitCount + 7) >> 3) * i->width);
    return i;
}

s32 *__fastcall SurfaceInfo_Alloc(SurfaceInfo *i, void *, s32 *out) {
    if (!i->bits) {
        i->pitch = static_cast<s32>(i->width * ((i->bitCount + 7) >> 3));
        i->bits = Mem_GlobalAlloc(SurfaceInfo_Size(i));
        if (i->bits) {
            *out = 0;
            return out;
        }
    }
    *out = -1;
    return out;
}

void __fastcall SurfaceInfo_Free(SurfaceInfo *i) {
    if (i->bits) {
        Mem_GlobalFree(i->bits);
        i->bits = nullptr;
    }
}

s32 *__fastcall SurfaceInfo_SubRectAt(const SurfaceInfo *i, void *, s32 *out, SurfaceInfo *dst, const s32 *at,
                                      const s32 *size) {
    const Rect r = {at[0], at[1], at[0] + size[0], at[1] + size[1]};
    return SurfaceInfo_SubRect(i, nullptr, out, dst, &r);
}

s32 *__fastcall SurfaceInfo_SubRect(const SurfaceInfo *i, void *, s32 *out, SurfaceInfo *dst, const Rect *r) {
    Rect part = {0, 0, static_cast<s32>(i->width), static_cast<s32>(i->height)};
    if (!Rect_Intersect(&part, r)) Rect_Collapse(&part);
    *dst = *i;
    dst->width = static_cast<u32>(part.right - part.left);
    dst->height = static_cast<u32>(part.bottom - part.top);
    const u32 offset = static_cast<u32>(part.top) * static_cast<u32>(i->pitch) +
                       ((dst->bitCount + 7) >> 3) * static_cast<u32>(part.left);
    dst->bits = reinterpret_cast<void *>(reinterpret_cast<std::uintptr_t>(dst->bits) + offset);
    *out = dst->width && dst->height ? 0 : -1;
    return out;
}

u32 __fastcall SurfaceInfo_Size(const SurfaceInfo *i) { return i->height * static_cast<u32>(i->pitch); }

void __cdecl Surface_SetDirectDraw(void *directDraw) { *g_env->directDraw = directDraw; }

s32 *__cdecl Surface_Attach(s32 *out, void *surface, Surface *s) {
    std::memset(s, 0, sizeof *s);
    u8 desc[kDescSize] = {};
    put(desc, 0, kDescSize);
    put(desc, 4, 0x100f);  // DDSD_CAPS | HEIGHT | WIDTH | PITCH | PIXELFORMAT
    comcall<s32>(surface, 0x58, desc);
    s->attached = 1;
    s->surface = surface;
    Surface_InitHeader(s, at(desc, 0xc), at(desc, 0x8), at(desc, 0x54), 0);
    *out = 0;
    return out;
}

Surface *__cdecl Surface_InitHeader(Surface *s, u32, u32, u32 bitCount, u32 unknown1c) {
    s->drawFlags = 0;
    s->ghost = nullptr;
    s->unknown08 = 0;
    s->palette = bitCount == 8 ? g_env->palette : nullptr;
    s->unknown1c = unknown1c;
    return s;
}

u32 __cdecl Surface_Restore(Surface *s) {
    if (comcall<s32>(s->surface, 0x60) == kSurfaceLost && comcall<s32>(s->surface, 0x6c) != 0) return 0;
    return 1;
}

s32 *__cdecl Surface_Create(s32 *out, u32 width, u32 height, u32 caps, Surface *s) {
    u8 desc[kDescSize] = {};
    put(desc, 0, kDescSize);
    put(desc, 4, 7);  // DDSD_CAPS | HEIGHT | WIDTH
    put(desc, 8, height);
    put(desc, 0xc, width);
    put(desc, 0x68, caps | 0x40);  // DDSCAPS_OFFSCREENPLAIN
    if (comcall<s32>(*g_env->directDraw, 0x18, desc, &s->surface, static_cast<void *>(nullptr))) {
        *out = -1;
        return out;
    }
    std::memset(desc, 0, sizeof desc);
    put(desc, 0, kDescSize);
    put(desc, 4, 0x1009);  // DDSD_CAPS | PITCH | PIXELFORMAT
    comcall<s32>(s->surface, 0x58, desc);
    Surface_InitHeader(s, width, height, at(desc, 0x54), 0);
    *out = 0;
    return out;
}

s32 *__cdecl Surface_Release(s32 *out, Surface *s) {
    comcall<u32>(s->surface, 8);
    s->surface = nullptr;
    *out = 0;
    return out;
}

s32 *__cdecl Surface_BltFast(s32 *out, Surface *dst, u32 x, u32 y, Surface *src, const Rect *srcRect, u32 flags) {
    const SurfaceInfo *info = Surface_Refresh(src);
    Surface_Refresh(dst);
    Rect all = {0, 0, 0, 0};
    if (!srcRect) {
        all.right = static_cast<s32>(info->width);
        all.bottom = static_cast<s32>(info->height);
        srcRect = &all;
    }
    if (flags & 1) setSourceKey(src, info->bitCount == 8 ? src->keyIndex : packKey(info, src->keyRgb));
    const s32 hr = comcall<s32>(dst->surface, 0x1c, x, y, src->surface, const_cast<Rect *>(srcRect), flags ^ 0x10);
    if (hr == kSurfaceLost) {
        Surface_Restore(dst);
        Surface_Restore(src);
    }
    *out = hr ? -1 : 0;
    return out;
}

s32 *__cdecl Surface_Blt(s32 *out, Surface *dst, const Rect *dstRect, Surface *src, const Rect *srcRect, u32 flags) {
    const SurfaceInfo *srcInfo = Surface_Refresh(src);
    const SurfaceInfo *dstInfo = Surface_Refresh(dst);
    Rect dstAll = {0, 0, static_cast<s32>(dstInfo->width), static_cast<s32>(dstInfo->height)};
    Rect srcAll = {0, 0, static_cast<s32>(srcInfo->width), static_cast<s32>(srcInfo->height)};
    if (!srcRect) srcRect = &srcAll;
    if (!dstRect) dstRect = &dstAll;
    u32 fx[0x19] = {};  // DDBLTFX
    fx[0] = sizeof fx;
    u32 bltFlags = flags ^ 0x1000000;  // DDBLT_WAIT
    if (flags & 0x800) {               // DDBLT_DDFX
        bltFlags |= 0x800;
        fx[1] |= 8;                    // DDBLTFX_NOTEARING
    }
    if (flags & 0x8000) {              // DDBLT_KEYSRC
        setSourceKey(src, srcInfo->bitCount == 8 ? src->keyIndex : packKey(srcInfo, src->keyRgb));
        bltFlags |= 0x8000;
    }
    const s32 hr = comcall<s32>(dst->surface, 0x14, const_cast<Rect *>(dstRect), src->surface,
                                const_cast<Rect *>(srcRect), bltFlags, fx);
    if (hr == kSurfaceLost) {
        Surface_Restore(dst);
        Surface_Restore(src);
    }
    // The original passes a failure to Debug_Printf (0x5da3c0), empty in this build.
    *out = hr ? -1 : 0;
    return out;
}

s32 *__cdecl Surface_Lock(s32 *out, Surface *s, void **bits) {
    u8 desc[kDescSize] = {};
    put(desc, 0, kDescSize);
    const u32 lockFlags = *g_env->noSysLock ? 0x801u : 1u;  // DDLOCK_WAIT (| DDLOCK_NOSYSLOCK)
    s32 hr = comcall<s32>(s->surface, 0x64, static_cast<void *>(nullptr), desc, lockFlags, static_cast<void *>(nullptr));
    if (hr) {
        if (hr == kSurfaceLost && Surface_Restore(s))
            hr = comcall<s32>(s->surface, 0x64, static_cast<void *>(nullptr), desc, lockFlags,
                              static_cast<void *>(nullptr));
        if (hr < 0) {
            *out = hr;
            return out;
        }
    }
    Surface_SetInfo(s, nullptr, desc);
    if (bits) *bits = reinterpret_cast<void *>(static_cast<std::uintptr_t>(at(desc, 0x24)));
    *out = 0;
    return out;
}

s32 __cdecl Surface_Unlock(Surface *s) { return comcall<s32>(s->surface, 0x80, static_cast<void *>(nullptr)); }

void *__cdecl Surface_GetSurface(Surface *s, s32 addRef) {
    if (addRef && s->surface) comcall<u32>(s->surface, 4);
    return s->surface;
}

void __fastcall Surface_SetMasks(u8 *surface, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f, u32 g, u32 h, u32 i, u32 j, u32 k, u32 l) {
    at<u32>(surface, 0x80) = a;
    at<u32>(surface, 0x84) = b;
    at<u32>(surface, 0x88) = c;
    at<u32>(surface, 0x8c) = d;
    at<u32>(surface, 0x90) = e;
    at<u32>(surface, 0x94) = f;
    at<u32>(surface, 0x98) = g;
    at<u32>(surface, 0x9c) = h;
    at<u32>(surface, 0xa0) = i;
    at<u32>(surface, 0xa4) = j;
    at<u32>(surface, 0xa8) = k;
    at<u32>(surface, 0xac) = l;
    if (at<s32>(surface, 0x1c) != 0x10) return;
    const u8 low = (u8)f;      // (the third word of the second group)
    const u8 high = (u8)l;     // (the last word)
    for (u32 n = 0; n < 0x100; ++n) {
        at<u32>(surface, 0xb0 + 4 * n) = ((n >> ((8 - low) & 31)) << ((0x18 - low) & 31)) | ((n >> (low & 31)) << ((low + 8) & 31));
        const u32 mask = (1u << (high & 31)) - 1u;
        at<u32>(surface, 0x4b0 + 4 * n) = ((n & mask) << ((8 - high) & 31)) | ((n >> (high & 31)) << ((0x10 - (s8)i) & 31));
    }
}
}  // extern "C"
