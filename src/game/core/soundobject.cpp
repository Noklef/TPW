#include "soundobject.h"

#include <cstdint>
#include <cstring>

#include "../math/vector.h"

#include "fieldops.h"
#include "initializers.h"
#include "soundcategory.h"
#include "soundchannel.h"
#include "soundmixer.h"
#include "string.h"
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

u32 *__fastcall SoundObject_SendVoice(u8 *object, void *, u32 *out, u32 id, u32 value) {
    if (!id) {
        *out = 0xffffffff;
        return out;
    }
    u8 *const channel = at<u8 *>(object, 0x24);
    if (channel && (at<u32>(channel, 0x28) & 0x400)) return SoundMixer_SendVoice(at<u8 *>(object, 0x20), nullptr, out, id, value);
    vcall<void>(object, 0x18, out, id);
    return out;
}

u32 *__fastcall SoundObject_SetVoices(u8 *object, void *, u32 *out, u32 count) {
    if (at<u32>(object, 0x1c) == count) {
        *out = 0;
        return out;
    }
    at<u32>(object, 0x1c) = count;
    if (u8 *const mixer = at<u8 *>(object, 0x20)) Obj_Set4cMin1e(mixer, nullptr, count);
    u8 record[0x70];
    Init_6b9c50(record);
    SetThenFlag40_4c_6b9cf0(record, nullptr, at<u32>(object, 0x1c));
    u32 functor[2] = {0x0070a278, (u32)(uintptr_t)record};
    SoundChannel_Apply(at<u8 *>(object, 0x24), nullptr, out, reinterpret_cast<u8 *>(functor));
    return out;
}

u32 *__fastcall SoundObject_SetPlacement(u8 *object, void *, u32 *out, const u32 *position, const u32 *direction, const u32 *value) {
    if (!at<u8 *>(object, 0x24)) {
        *out = 0xffffffff;
        return out;
    }
    u32 scaled[3] = {position[0], position[1], position[2]};
    Vec3_Scale(reinterpret_cast<float *>(scaled), nullptr, at<float>(at<u8 *>(object, 0x24), 0x2c));
    if (std::memcmp(scaled, object + 0x28, 12) || std::memcmp(direction, object + 4, 12) || std::memcmp(value, object + 0x10, 12)) {
        std::memcpy(object + 0x28, scaled, 12);
        std::memcpy(object + 4, direction, 12);
        std::memcpy(object + 0x10, value, 12);
        u32 local;   // (the original's is the direction argument's own slot)
        SoundChannel_SetPlacement(at<u8 *>(object, 0x24), nullptr, &local, (u32)(uintptr_t)(object + 0x28), (u32)(uintptr_t)(object + 4),
                                  (u32)(uintptr_t)(object + 0x10));
    }
    *out = 0;
    return out;
}

s32 *__fastcall SoundObject_LoadCategory(u8 *object, void *, s32 *out, const char *path, const char *name, u32 *index) {
    if (!at<u32>(object, 0x24)) {
        *out = -1;
        return out;
    }
    String full;
    String_Ctor(&full);
    SoundObject_ResolvePath(object, nullptr, path, reinterpret_cast<u8 *>(&full));
    SoundCategory_Load(SoundCategory_Get(), nullptr, out, full.data, name, index);
    String_Dtor(&full);
    return out;
}

u32 __fastcall SoundObject_ResolvePath(u8 *object, void *, const char *path, u8 *string) {
    const auto slash = [](char c) { return c == '\\' || c == '/'; };
    if ((path[1] == ':' && slash(path[2])) || (slash(path[0]) && slash(path[1]))) return vcall<u32>(string, 0x14, path);
    vcall<u32>(string, 0x18, *reinterpret_cast<u8 **>(object + 0x24) + 0x30);
    return vcall<u32>(string, 0x20, path);
}

}  // extern "C"
