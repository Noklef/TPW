#include "moviesurface.h"

#include <cstdint>

#include "dderrors.h"

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

// With the flag 0x1000 of the settings +4: the IDirectDraw's WaitForVerticalBlank (1, 0).
void waitForBlank(u8 *s) {
    if (at<u32>(s, 4) & 0x1000) com(at<u8 *>(s, 0x10), 0x58, 1u, 0u);
}

// The whole frame's rectangle and the player's place on the screen.
void presentRects(u8 *player, u32 *source, u32 *destination) {
    u8 *const s = settings(player);
    source[0] = 0;
    source[1] = 0;
    source[2] = at<u32>(s, 0x64);
    source[3] = at<u32>(s, 0x60);
    destination[0] = at<u32>(player, 0x14);
    destination[1] = at<u32>(player, 0x18);
    destination[2] = at<u32>(player, 0xc) + destination[0];
    destination[3] = at<u32>(player, 0x10) + destination[1];
}

// True when the surface was lost and its Restore failed.
bool restoreIfLost(u8 *surface) { return com(surface, 0x60) == kSurfaceLost && com(surface, 0x6c) != 0; }

// The frame's dwords (+0x648; half the width of the settings +0x64, in whole blocks of eight, for +0x60 lines) into the locked surface +0x1c.
void copyFrame(u8 *player) {
    u8 *const s = settings(player);
    u32 description[0x1b] = {0x6c};
    if (com(at<u8 *>(s, 0x1c), 0x64, 0u, description, 0x801u, 0u)) return;
    const s32 pitch = static_cast<s32>(description[4]) / 4;
    const u32 *from = at<const u32 *>(player, 0x648);
    u32 *to = reinterpret_cast<u32 *>((uintptr_t)description[9]);
    const u32 dwords = (at<u32>(s, 0x64) << 1) >> 2;
    for (u32 line = 0; line < at<u32>(s, 0x60); ++line) {
        const u32 count = (dwords + 7) & ~7u;
        for (u32 i = 0; i < count; ++i) to[i] = from[i];
        from += dwords;
        to += pitch;
    }
    com(at<u8 *>(s, 0x1c), 0x80, 0u);
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

void __fastcall MovieSurface_Present(u8 *player) {
    u8 *const s = settings(player);
    u8 *const palette = at<u8 *>(s, 0x28);
    if (at<u32>(s, 4) & 0x800) return;
    ++at<u32>(s, 0x7b4);
    const u32 mode = at<u32>(player, 0x4c);
    if (mode == 4) {
        waitForBlank(s);
        u32 source[4], destination[4];
        presentRects(player, source, destination);
        if (restoreIfLost(at<u8 *>(s, 0x1c))) return;
        if (com(at<u8 *>(s, 0x14), 0x14, destination, at<u8 *>(s, 0x1c), source, 0x1000000u, 0u)) at<u32>(player, 0x4c) = 1;
    } else if (mode == 2) {
        if (at<u8 *>(s, 0x1c)) {
            if (at<u32>(player, 0x54)) {
                u32 result;
                do {
                    result = com(at<u8 *>(s, 0x1c), 0x2c, 0u, 0u);
                } while (result == 0x887601ae || result == 0x8876021c);
            } else {
                waitForBlank(s);
                copyFrame(player);
            }
            if (at<u32>(player, 0x50)) at<u32>(player, 0x50) = MovieSurface_UpdateOverlay(player, 1);
        }
    } else if (mode == 1 && !(at<u32>(s, 4) & 0x400)) {
        u8 *const target = at<u8 *>(s, (at<u32>(s, 4) & 0x8000000) ? 0x18 : 0x14);
        if (at<u32>(s, 0x48) == 8) {
            for (u32 i = 0; i < 0x100; ++i) {
                palette[i * 4] = s[0xb4 + i * 3];
                palette[i * 4 + 1] = s[0xb5 + i * 3];
                palette[i * 4 + 2] = s[0xb6 + i * 3];
                palette[i * 4 + 3] = 0;
            }
        }
        u32 source[4], destination[4];
        presentRects(player, source, destination);
        if (restoreIfLost(target)) return;
        const u32 flags = at<u32>(s, 4);
        if (flags & 0x30) {
            // Field by field: each second line of the target from one line of the frame; with 0x10 and not 0x20 the next line too.
            for (u32 line = 0; line < at<u32>(s, 0x60); line += 2) {
                destination[1] = at<u32>(player, 0x18) + line;
                destination[3] = destination[1] + 1;
                source[1] = line >> 1;
                source[3] = source[1] + 1;
                com(target, 0x14, destination, at<u8 *>(s, 0x1c), source, 0x1000000u, 0u);
                if (flags & 0x20) continue;
                destination[1] = at<u32>(player, 0x18) + line + 1;
                destination[3] = destination[1] + 1;
                com(target, 0x14, destination, at<u8 *>(s, 0x1c), source, 0x1000000u, 0u);
            }
        } else {
            com(target, 0x14, destination, at<u8 *>(s, 0x1c), source, 0x1000000u, 0u);
        }
    }
    if (!(at<u32>(s, 4) & 0x8000000) || at<u32>(player, 0x4c) != 1) return;
    u8 *const primary = at<u8 *>(s, 0x14);
    const u32 result = com(primary, 0x2c, 0u, 1u);
    if (at<u32>(s, 0x48) == 8) {
        com(at<u8 *>(s, 0x24), 0x18, 0u, 0u, 0x100u, palette);
    } else if (result) {
        (*reinterpret_cast<void(__stdcall *const *)(const char *)>((uintptr_t)0x006fd1e0))(DdErrors_Name(result));
    }
}
}
