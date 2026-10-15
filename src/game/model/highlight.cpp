#include "highlight.h"

#include <cstdint>

#include "../core/globalmem.h"
#include "../core/movs.h"
#include "../math/x87.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

u8 *&current() { return global<u8 *>(0x007a1ce0); }
u8 *&target() { return global<u8 *>(0x007afda0); }
u32 *saved() { return global<u32 *>(0x007afd94); }
u8 *model(const u8 *holder) { return at<u8 *>(at<const u8 *>(holder, 8), 4); }
u8 *&flashing() { return global<u8 *>(0x007afe14); }

// The release both 0x469180 and 0x469470 inline, for the current holder h.
void release(u8 *h) {
    const u8 *m = model(h);
    u8 *mesh = at<u8 *>(m, 0x70);
    for (u32 i = 0; i < at<u16>(m, 0x44); ++i, mesh += 0xa0) {
        const u32 flags = at<u32>(mesh, 0);
        if (flags & 0x4000) continue;
        if (!(at<u32>(h, 4) & 0x4000000)) at<u32>(mesh, 0) = flags & ~0x1000000u;
        at<u32>(mesh, 0x64) = saved()[i];
        h = current();
    }
    at<u32>(h, 4) &= ~0x20000u;
    at<u32>(model(current()), 0x30) &= ~0x400000u;
    current() = nullptr;
}

}  // namespace

extern "C" {

void __cdecl Highlight_Update() {
    u8 *h = current();
    if (h && target() && h != target() && (at<u32>(h, 4) & 0x20000)) release(h);
    u8 *t = target();
    if (!t || (at<u32>(t, 4) & 0x20000) || global<u32>(0x007afda4)) return;
    const u8 *m = model(t);
    u8 *buffer = global<u8 *>(0x007afd00);
    u8 *mesh = at<u8 *>(m, 0x70);
    for (u32 i = 0; i < at<u16>(m, 0x44); ++i, mesh += 0xa0) {
        const u32 flags = at<u32>(mesh, 0);
        if (flags & 0x4000) continue;
        at<u32>(mesh, 0) = flags | 0x1000000;
        const u32 skip = at<u16>(mesh, 0x5e) * 12u;
        const uintptr_t from = (uintptr_t)at<u32>(mesh, 0x64) + skip, to = (uintptr_t)buffer + skip;
        if (to && from && at<u16>(mesh, 0x5c))
            Rep_Movs(reinterpret_cast<void *>(to), reinterpret_cast<const void *>(from), at<u16>(mesh, 0x5c) * 12u);
        saved()[i] = at<u32>(mesh, 0x64);
        at<u8 *>(mesh, 0x64) = buffer;
        buffer += (at<u16>(mesh, 0x5c) + at<u16>(mesh, 0x5e)) * 12u;
    }
    at<u32>(target(), 4) |= 0x20000;
    at<u32>(model(target()), 0x30) |= 0x400000;
    current() = target();
}

void __cdecl Highlight_SetTarget(u8 *holder) { target() = holder; }

void __cdecl Highlight_ResetTints() {
    for (u32 i = 0; i < 6; ++i)
        for (u32 c = 0; c < 3; ++c) global<u32>(0x007afdb0 + 0x10 * i + 4 * c) = 0x3f800000;  // 1.0f
}

void __cdecl Highlight_Clear() {
    u8 *h = current();
    if (h && (at<u32>(h, 4) & 0x20000)) release(h);
}

u8 *__cdecl Highlight_Current() { return current(); }

float *__cdecl Highlight_Point() { return reinterpret_cast<float *>(0x007a1c90); }

void __cdecl Highlight_SetShown(u32 shown) { global<u32>(0x007a1c7c) = shown && current() ? 1 : 0; }

void __cdecl Highlight_Suspend(u32 suspend) {
    if (suspend) {
        if (current()) Highlight_Clear();
        global<u32>(0x007afda4) = 1;
    } else {
        global<u32>(0x007afda4) = 0;
    }
}

void __cdecl Highlight_AllocBuffers() {
    global<void *>(0x007afd00) = GlobalMem_Alloc((global<u32>(0x007afd90) + global<u32>(0x007afd9c)) * 12, 1);
    global<void *>(0x007afd94) = GlobalMem_Alloc(global<u32>(0x007afe10) * 4, 1);
}

void __cdecl Highlight_FreeBuffers() {
    GlobalMem_Free(global<void *>(0x007afd00));
    GlobalMem_Free(global<void *>(0x007afd94));
    global<void *>(0x007afd00) = nullptr;
    global<void *>(0x007afd94) = nullptr;
}

void __cdecl Highlight_Flash(u8 *holder) {
    u8 *m = model(holder);
    at<u32>(holder, 4) |= 0x4000000;
    at<u32>(m, 0x30) |= 0x800000;
    at<u8 *>(holder, 0xd0) = flashing();
    flashing() = holder;
    if (at<u32>(m, 0x5c) != at<u32>(at<const u8 *>(m, 0x9c), 0x5c)) {
        u8 *mesh = at<u8 *>(m, 0x70);
        for (u32 i = at<u16>(m, 0x44); i; --i, mesh += 0xa0) at<u32>(mesh, 0) |= 0x1000000;
        return;
    }
    at<u32>(m, 0x30) |= 0x100000;
    u8 *block = static_cast<u8 *>(GlobalMem_Alloc(at<u16>(m, 0x4a) * 12u, 1));
    at<u8 *>(m, 0x5c) = block;
    u8 *mesh = at<u8 *>(m, 0x70);
    for (u32 i = at<u16>(m, 0x44); i; --i, mesh += 0xa0) {
        at<u32>(mesh, 0) |= 0x1000000;
        at<u8 *>(mesh, 0x64) = block;
        block += (at<u16>(mesh, 0x58) + at<u16>(mesh, 0x5c)) * 12u;
    }
}

void __cdecl Highlight_Unflash(u8 *holder) {
    u8 *m = model(holder);
    const u8 *base = at<const u8 *>(m, 0x9c);
    at<u32>(holder, 4) &= ~0x4000000u;
    at<u32>(m, 0x30) &= ~0x800000u;
    if (flashing() == holder) {
        flashing() = at<u8 *>(holder, 0xd0);
    } else {
        u8 *prev = flashing();
        while (at<u8 *>(prev, 0xd0) != holder) prev = at<u8 *>(prev, 0xd0);  // sic: never ends unless listed
        at<u8 *>(prev, 0xd0) = at<u8 *>(holder, 0xd0);
    }
    at<u8 *>(holder, 0xd0) = nullptr;
    bool again = false;
    if (at<u32>(holder, 4) & 0x20000) {
        u8 *h = current();
        if (h && (at<u32>(h, 4) & 0x20000)) release(h);
        again = true;
    }
    u8 *mesh = at<u8 *>(m, 0x70);
    const u8 *from = at<const u8 *>(base, 0x70);
    if (at<u32>(m, 0x30) & 0x100000) {
        at<u32>(m, 0x30) &= ~0x100000u;
        GlobalMem_Free(at<void *>(m, 0x5c));
        at<u32>(m, 0x5c) = at<u32>(base, 0x5c);
        for (u32 i = at<u16>(m, 0x44); i; --i, mesh += 0xa0, from += 0xa0) {
            at<u32>(mesh, 0x64) = at<u32>(from, 0x64);
            at<u32>(mesh, 0) &= ~0x1000000u;
        }
    } else {
        for (u32 i = at<u16>(m, 0x44); i; --i, mesh += 0xa0, from += 0xa0) {
            at<u32>(mesh, 0) &= ~0x1000000u;
            if (at<u32>(mesh, 0x64) && at<u32>(from, 0x64) && at<u16>(mesh, 0x5e))
                Rep_Movs(at<u8 *>(mesh, 0x64), at<const u8 *>(from, 0x64), at<u16>(mesh, 0x5e) * 12u);
        }
    }
    if (again) Highlight_Update();
}

void __cdecl Highlight_PulseFlashing() {
    const float phase = global<float>(0x007afd04);
    const X87Reg level = phase > 1.0f ? X87Reg(2.0f) - phase : X87Reg(phase);
    const u32 v = (u32)(long long)(level * 255.0f).v;  // __ftol
    const u32 colour = v | (v | 0xffffff00) << 8;
    const X87Reg step = X87Reg(global<float>(0x007b497c)) * -2.0f;  // gMotionTimeStep
    global<float>(0x007afd04) = (float)std::fmod((X87Reg(phase) - step).v, 2.0);  // _CIfmod (0x67b24a)
    for (const u8 *h = flashing(); h; h = at<const u8 *>(h, 0xd0)) {
        const u8 *m = model(h);
        const u8 *mesh = at<const u8 *>(m, 0x70);
        for (u32 i = at<u16>(m, 0x44); i; --i, mesh += 0xa0) {
            u32 *to = at<u32 *>(mesh, 0x64);
            for (u32 j = at<u16>(mesh, 0x5e); j; --j) *to++ = colour;  // rep stosd
        }
    }
}
}
