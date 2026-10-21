#include "moviesound.h"

#include <cstdint>

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }

u8 *settings(u8 *player) { return at<u8 *>(player, 0); }

// The COM method at a vtable offset (stdcall, the object first).
template <typename F> F method(void *object, u32 offset) { return *reinterpret_cast<F *>(*reinterpret_cast<u8 **>(object) + offset); }

using Method1 = s32(__stdcall *)(void *);
using Method2 = s32(__stdcall *)(void *, void *);
using Method3 = s32(__stdcall *)(void *, void *, void *);

}  // namespace

extern "C" {

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
