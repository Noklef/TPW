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
}
