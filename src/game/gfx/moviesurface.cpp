#include "moviesurface.h"

#include <cstdint>

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

u8 *settings(u8 *player) { return at<u8 *>(player, 0); }

// The COM method at a vtable offset (stdcall, the object first).
template <typename F> F method(void *object, u32 offset) { return *reinterpret_cast<F *>(*reinterpret_cast<u8 **>(object) + offset); }

using Method1 = s32(__stdcall *)(void *);
using Method2 = s32(__stdcall *)(void *, void *);
using Method3 = s32(__stdcall *)(void *, void *, void *);

const u32 kSurfaceLost = 0x887601c2;

// One surface's step in MovieSurface_Restore: true when its Restore failed.
bool restore_failed(u8 *player, u32 offset) {
    void *surface = at<void *>(settings(player), offset);
    if ((u32)method<Method1>(surface, 0x60)(surface) != kSurfaceLost) return false;
    surface = at<void *>(settings(player), offset);
    return method<Method1>(surface, 0x6c)(surface) != 0;
}

// Any COM method (its result as u32).
template <typename... A> u32 com(void *object, u32 offset, A... args) { return reinterpret_cast<u32(__stdcall *)(void *, A...)>(*reinterpret_cast<void **>(*reinterpret_cast<u8 **>(object) + offset))(object, args...); }

void describe(u32 *description, u32 flags, u32 width, u32 height, u32 fourcc, u32 caps) {
    for (u32 i = 0; i < 0x1b; ++i) description[i] = 0;
    description[0] = 0x6c;
    description[1] = flags;
    description[2] = height;
    description[3] = width;
    description[0x12] = 0x20;
    description[0x13] = 4;
    description[0x14] = fourcc;
    description[0x1a] = caps;
}

u32 queryLater(u8 *surface, u8 **later) { return com(surface, 0, 0x00707220u, later); }

void fillGrey(u8 *surface) {
    u32 effects[0x19] = {0x64};
    effects[0x14] = 0x80808080;
    com(surface, 0x14, 0u, 0u, 0u, 0x01000400u, effects);
}

}  // namespace

extern "C" {

void __fastcall MovieSurface_ReadSize(u8 *player, void *) {
    u32 desc[0x1b] = {};
    desc[0] = 0x6c;
    void *surface = at<void *>(settings(player), 0x14);
    if (method<Method2>(surface, 0x58)(surface, desc) == 0) {
        at<u32>(player, 4) = desc[3];
        at<u32>(player, 8) = desc[2];
    }
}

u32 __fastcall MovieSurface_PickMode(u8 *player, void *) {
    u8 *const s = settings(player);
    if (!(at<u32>(s, 4) & 0x200)) return 1;
    u32 driver[0x5f] = {}, emulated[0x5f] = {};
    driver[0] = 0x17c;
    emulated[0] = 0x17c;
    void *draw = at<void *>(s, 0x10);
    if (method<Method3>(draw, 0x2c)(draw, driver, emulated) != 0) return 1;
    const u32 flags = at<u32>(settings(player), 8), caps = driver[1];
    if ((flags & 4) && (caps & 0x100) && (caps & 0x200)) return 4;
    if ((flags & 2) && (caps & 0x2000) && (caps & 0x4000)) return 2;
    return 1;
}

u32 __fastcall MovieSurface_Restore(u8 *player, void *) {
    if (at<void *>(settings(player), 0x14) && (at<u32>(settings(player), 4) & 0x2000000) && restore_failed(player, 0x14)) return 1;
    if (at<void *>(settings(player), 0x18) && (at<u32>(settings(player), 4) & 0x8000000) && restore_failed(player, 0x18)) return 1;
    for (u32 offset = 0x1c; offset <= 0x20; offset += 4) {
        void *surface = at<void *>(settings(player), offset);
        if (!surface || (u32)method<Method1>(surface, 0x60)(surface) != kSurfaceLost) continue;
        surface = at<void *>(settings(player), offset);
        if (method<Method1>(surface, 0x6c)(surface) != 0) return 1;
        if (at<u32>(player, 0x4c) == 2) at<u32>(player, 0x50) = 1;
    }
    return 0;
}

u32 __fastcall MovieSurface_CreateOverlay(u8 *draw, u8 **out, u32 width, u32 height, u32 fourcc) {
    u32 description[0x1b];
    u8 *surface = nullptr, *later;
    describe(description, 0x1007, width, height, fourcc, 0x80);
    if (com(draw, 0x18, description, &surface, 0u)) return 1;
    const u32 failed = queryLater(surface, &later);
    com(surface, 8);
    if (failed) return 1;
    const u32 key[2] = {1, 1};
    if (com(later, 0x74, 4u, key)) return 1;
    fillGrey(later);
    *out = later;
    return 0;
}

u32 __fastcall MovieSurface_CreateOffscreen(u8 *draw, u8 **out, u32 width, u32 height, u32 fourcc) {
    u32 description[0x1b];
    u8 *surface, *later;
    describe(description, 0x1007, width, height, fourcc, 0x4040);
    if (com(draw, 0x18, description, &surface, 0u)) return 1;
    const u32 failed = queryLater(surface, &later);
    com(surface, 8);
    if (failed) return 1;
    fillGrey(later);
    *out = later;
    return 0;
}

u32 __fastcall MovieSurface_CreateFlipOverlay(u8 *draw, u8 **out, u8 **backOut, u32 width, u32 height, u32 fourcc) {
    u32 description[0x1b];
    u8 *surface = nullptr, *back = nullptr, *front, *later;
    describe(description, 0x1027, width, height, fourcc, 0x98);
    description[5] = 1;
    if (com(draw, 0x18, description, &surface, 0u)) return 1;
    u32 caps = 4;
    if (!com(surface, 0x30, &caps, &back) && !queryLater(surface, &front)) {
        com(surface, 8);
        surface = nullptr;
        if (!queryLater(back, &later)) {
            com(back, 8);
            back = nullptr;
            const u32 key[2] = {1, 1};
            if (!com(front, 0x74, 4u, key) && !com(later, 0x74, 4u, key)) {
                fillGrey(front);
                fillGrey(later);
                *out = front;
                *backOut = later;
                return 0;
            }
        }
    }
    if (surface) com(surface, 8);
    if (back) com(back, 8);
    return 1;
}

u32 __fastcall MovieSurface_UpdateOverlay(u8 *player, u32 show) {
    u32 effects[0xe] = {0x38};
    u32 flags = 0x200;
    if (show) {
        flags = 0x84000;
        effects[0xc] = 1;
    }
    u8 *const s = settings(player);
    const u32 left = at<u32>(player, 0x14), top = at<u32>(player, 0x18);
    const u32 destination[4] = {left, top, at<u32>(player, 0xc) + left, at<u32>(player, 0x10) + top};
    const u32 source[4] = {0, 0, at<u32>(s, 0x64), at<u32>(s, 0x60)};
    const u32 result = com(at<u8 *>(s, 0x1c), 0x84, source, at<u8 *>(s, 0x14), destination, flags, effects);
    if (!result) return 0;
    if (result == 0x80070057 || result == 0x88760168 || result == 0x80004001 || result == 0x80004005 || result == 0x88760082) at<u32>(player, 0x4c) = 1;
    if (result == kSurfaceLost) com(at<u8 *>(settings(player), 0x1c), 0x6c);
    return 1;
}
}
