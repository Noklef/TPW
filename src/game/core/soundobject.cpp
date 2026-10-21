#include "soundobject.h"

#include <cstdint>

#include "soundmixer.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 *__fastcall SoundObject_Broadcast(u8 *object, void *, u32 *out) {
    if (u8 *const mixer = at<u8 *>(object, 0x20)) SoundMixer_ForEachActive(mixer, nullptr, 0);
    u8 *const channel = at<u8 *>(object, 0x24);
    if (!channel) {
        *out = 0xffffffff;
        return out;
    }
    u32 functor[2];   // (the second dword left unset, sic)
    functor[0] = 0x0070a5b0;
    vcall<void>(channel, 0x10, out, functor);
    return out;
}

u32 *__fastcall SoundObject_BroadcastValue(u8 *object, void *, u32 *out, u32 value) {
    u8 *const channel = at<u8 *>(object, 0x24);
    if (channel && (at<u32>(channel, 0x28) & 0x400)) {
        if (u8 *const mixer = at<u8 *>(object, 0x20)) SoundMixer_ForEachActive(mixer, nullptr, value);
    } else {
        SoundObject_Broadcast(object, nullptr, &value);
    }
    *out = 0;
    return out;
}

u32 *__fastcall SoundObject_VoiceInfo(u8 *object, void *, u32 *out, u32 id, u32 *result) {
    if (u8 *const mixer = at<u8 *>(object, 0x20)) {
        u8 *const voice = SoundMixer_FindVoice(mixer, nullptr, id);
        if (voice) {
            if (u8 *const info = at<u8 *>(voice, 0xc)) {
                *result = at<u32>(info, 8);
                *out = 0;
                return out;
            }
        }
    }
    *out = 0xffffffff;
    return out;
}

}  // extern "C"
