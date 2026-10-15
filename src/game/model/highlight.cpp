#include "highlight.h"

#include <cstdint>

#include "../core/movs.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

u8 *&current() { return global<u8 *>(0x007a1ce0); }
u8 *&target() { return global<u8 *>(0x007afda0); }
u32 *saved() { return global<u32 *>(0x007afd94); }
u8 *model(const u8 *holder) { return at<u8 *>(at<const u8 *>(holder, 8), 4); }

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

void __cdecl Highlight_SetShown(u32 shown) { global<u32>(0x007a1c7c) = shown && current() ? 1 : 0; }

void __cdecl Highlight_Suspend(u32 suspend) {
    if (suspend) {
        if (current()) Highlight_Clear();
        global<u32>(0x007afda4) = 1;
    } else {
        global<u32>(0x007afda4) = 0;
    }
}
}
