// The surface format helpers: see surface.h.
#include "surface.h"

#include <cstdint>
#include <cstring>

#include "../core/vcall.h"

namespace {

u32 at(const u8 *p, u32 offset) {
    u32 v;
    std::memcpy(&v, p + offset, 4);
    return v;
}

}  // namespace

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

}  // extern "C"
