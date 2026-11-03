#include "moviesound.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

u8 *settings(u8 *player) { return at<u8 *>(player, 0); }

// The COM method at a vtable offset (stdcall, the object first).
template <typename F> F method(void *object, u32 offset) { return *reinterpret_cast<F *>(*reinterpret_cast<u8 **>(object) + offset); }

using Method1 = s32(__stdcall *)(void *);
using Method2 = s32(__stdcall *)(void *, void *);
using Method3 = s32(__stdcall *)(void *, void *, void *);

// Any COM method.
template <typename... A> s32 com(void *object, u32 offset, A... args) { return method<s32(__stdcall *)(void *, A...)>(object, offset)(object, args...); }

}  // namespace

extern "C" {

u32 __fastcall MovieSound_CreateStream(u8 *player, void *) {
    struct Format {
        u16 tag, channels;
        u32 rate, bytes;
        u16 align, bits, extra;
    };
    u8 *const s = settings(player);
    bool made = false;
    if (void *const device = at<void *>(s, 0x54)) {
        const u32 rate = at<u32>(player, 0x5c);
        Format format = {1, 2, rate, rate << 2, 4, 0x10, 0};
        u32 description[5] = {0x14, 0x100e0, at<u32>(player, 0x3d4), 0, (u32)(uintptr_t)&format};
        if (!com(device, 0xc, description, &at<void *>(s, 0x5c), 0u)) {
            void *const buffer = at<void *>(s, 0x5c);
            com(buffer, 0x44, at<u32>(player, 0x5c));
            u32 caps[5] = {0x14, 0, 0, 0, 0};
            if (at<u32>(s, 4) & 0x2000) com(buffer, 0x3c, at<u32>(s, 0x8c));
            if (!com(buffer, 0xc, caps)) {
                at<u32>(player, 0x36c) = caps[2];
                at<u32>(player, 0x370) = 0;
                at<u32>(player, 0x3cc) = 0;
                u8 *first, *second;
                u32 firstBytes, secondBytes;
                if (!com(buffer, 0x2c, at<u32>(player, 0x370), at<u32>(player, 0x36c), &first, &firstBytes, &second, &secondBytes, 0u)) {
                    std::memset(first, 0, firstBytes);
                    if (secondBytes) std::memset(second, 0, secondBytes);
                    com(buffer, 0x4c, first, firstBytes, second, secondBytes);
                }
                made = !com(buffer, 0x30, 0u, 0u, 1u);
            }
            if (!made) com(buffer, 8);
        }
    }
    if (!made) {
        at<void *>(s, 0x5c) = nullptr;
        at<u32>(player, 0x3c8) = 0;
        at<u32>(player, 0x36c) = 0x8000;
        at<u32>(player, 0x3cc) = 1;
        at<u32>(player, 0x370) = 0;
    }
    at<u32>(player, 0x368) = 1;
    return 0;
}

u32 __fastcall MovieSound_BufferBytes(void *buffer, void *) {
    u32 caps[5] = {0x14, 0, 0, 0, 0};
    method<Method2>(buffer, 0xc)(buffer, caps);
    return caps[2];
}

u32 __fastcall MovieSound_Release(void **place, void *) {
    if (void *object = *place) {
        method<Method1>(object, 8)(object);
        *place = nullptr;
    }
    return 0;
}

u32 __fastcall MovieSound_ReleaseAll(u8 *player, void *) {
    if (!(at<u32>(settings(player), 4) & 0x800000)) return 0;
    if (at<u32>(player, 0x3d8)) {
        if (void *primary = at<void *>(settings(player), 0x58)) {
            method<Method1>(primary, 8)(primary);
            at<void *>(settings(player), 0x58) = nullptr;
        }
    }
    if (at<u32>(player, 0x3dc)) MovieSound_Release(reinterpret_cast<void **>(settings(player) + 0x54), nullptr);
    return 0;
}

u32 __fastcall MovieSound_ReleaseStream(u8 *player, void *) {
    if (void *stream = at<void *>(settings(player), 0x5c)) {
        method<Method1>(stream, 8)(stream);
        at<void *>(settings(player), 0x5c) = nullptr;
    }
    return 0;
}

s32 __fastcall MovieSound_PlayDistance(u8 *player, void *) {
    s32 position;
    if (at<u32>(player, 0x3cc)) {
        at<s32>(player, 0x3c8) = (s32)(at<u32>(player, 0x3c8) + 0x372);
        if (at<s32>(player, 0x3c8) >= at<s32>(player, 0x36c)) at<s32>(player, 0x3c8) = (s32)(at<u32>(player, 0x3c8) - at<u32>(player, 0x36c));
        position = at<s32>(player, 0x3c8);
    } else {
        void *stream = at<void *>(settings(player), 0x5c);
        s32 play = 0, write = 0;   // (the original's are uninitialised)
        method<Method3>(stream, 0x10)(stream, &play, &write);
        position = play;
    }
    if (at<s32>(player, 0x370) < position) return (s32)((u32)position - at<u32>(player, 0x370));
    return (s32)((u32)position - at<u32>(player, 0x370) + at<u32>(player, 0x36c));
}
}
