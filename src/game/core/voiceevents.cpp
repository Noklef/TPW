#include "voiceevents.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }
using OnHandle = s32(__stdcall *)(u32);

}  // namespace

extern "C" {

u8 *__fastcall VoiceEvents_Ctor(u8 *self, void *, u8 *voices, u32 count) {
    at<u8 *>(self, 4) = voices;
    at<u32>(self, 0xc) = count;
    at<u32>(self, 8) = count + 1;
    at<void *>(self, 0x10) = GameHeap_Alloc((count + 1) * 4);
    at<u32>(self, 0x14) = 0;
    at<u32>(self, 0) = 0x0070ab50;
    return self;
}

void __fastcall VoiceEvents_Dtor(u8 *self) {
    at<u32>(self, 0) = 0x0070ab50;
    for (u32 i = 0; i < at<u32>(self, 8); ++i) Import<OnHandle>(0x006fd13c)(at<u32 *>(self, 0x10)[i]);   // CloseHandle
    GameHeap_Free(at<void *>(self, 0x10));
}

u8 *__fastcall VoiceEvents_DeletingDtor(u8 *self, void *, u32 flags) {
    VoiceEvents_Dtor(self);
    if (flags & 1) GameHeap_Free(self);
    return self;
}

u32 __fastcall VoiceEvents_Stop(u8 *self) {
    at<u32>(self, 0x14) |= 1;
    return static_cast<u32>(Import<OnHandle>(0x006fd124)(at<u32 *>(self, 0x10)[at<u32>(self, 0xc)]));   // SetEvent
}

}  // extern "C"
