#include "soundobject.h"

#include <cstdint>
#include <cstring>

#include "../math/vector.h"
#include "../files/file.h"

#include "fieldops.h"
#include "gameheap.h"
#include "initializers.h"
#include "selectors.h"
#include "soundcategory.h"
#include "soundchannel.h"
#include "soundmixer.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

// 0x6b89b0 / 0x6b8a10 / 0x6b8a70's body: the functor of that vtable to the id's voice.
u32 *SendVoiceFunctor(u8 *object, u32 *out, u32 id, u32 vtable) {
    if (id && at<u8 *>(object, 0x20)) {
        if (u8 *const voice = SoundMixer_FindVoice(at<u8 *>(object, 0x20), nullptr, id)) {
            u32 functor[2];
            functor[1] = at<u32>(voice, 0x24);
            functor[0] = vtable;
            vcall<void>(voice, 0x18, out, functor);
            return out;
        }
    }
    *out = 0xffffffff;
    return out;
}

}  // namespace

extern "C" {

u32 *__fastcall SoundObject_Open(u8 *object, void *, u32 *out, u32 value) {
    vcall<void>(object, 0x24);
    if (static_cast<s32>(*vcall<u32 *>(at<u8 *>(object, 0x24), 0x1c, &value, value, at<u32>(object, 0x1c))) < 0) {
        *out = 0xffffffff;
        return out;
    }
    u8 *const channel = at<u8 *>(object, 0x24);
    SoundCategory_Get();
    *reinterpret_cast<u8 **>(static_cast<uintptr_t>(0xfb1ffc)) = channel;
    at<u8 *>(at<u8 *>(object, 0x24), 0x60) = object + 0x28;
    *reinterpret_cast<u8 **>(static_cast<uintptr_t>(0xfb1f24)) = at<u8 *>(object, 0x24);
    vcall<void>(object, 0x1c, &value, 1u);
    *out = 0;
    return out;
}

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

void __fastcall SoundObject_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x0070a588;
    if (u8 *const channel = at<u8 *>(object, 0x24)) vcall<void>(channel, 0);
    if (u8 *const mixer = at<u8 *>(object, 0x20)) vcall<void>(mixer, 0, 1u);
}

u32 *__fastcall SoundObject_SendVoiceFunctor1(u8 *object, void *, u32 *out, u32 id) { return SendVoiceFunctor(object, out, id, 0x0070a2d0); }
u32 *__fastcall SoundObject_SendVoiceFunctor2(u8 *object, void *, u32 *out, u32 id) { return SendVoiceFunctor(object, out, id, 0x0070a5b8); }
u32 *__fastcall SoundObject_SendVoiceFunctor3(u8 *object, void *, u32 *out, u32 id) { return SendVoiceFunctor(object, out, id, 0x0070a5c0); }

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

u32 *__fastcall SoundObject_ApplyEaxRecord(u8 *object, void *, u32 *out, u32 index) {
    if (!Obj_Flags28_Bit1IfBit0(at<u8 *>(object, 0x24))) {
        *out = 0xffffffff;
        return out;
    }
    SoundChannel_ApplyEaxRecord(at<u8 *>(object, 0x24), nullptr, out, index);
    return out;
}

u32 *__fastcall SoundObject_AddOccluder(u8 *object, void *, u32 *out, u32 x, u32 y, u32 z, u32 radius, u8 **node) {
    u8 *const channel = *reinterpret_cast<u8 **>(object + 0x24);
    if (!channel) {
        *out = 0xffffffff;
        return out;
    }
    SoundChannel_TryAddOccluder(channel, nullptr, out, x, y, z, radius, node);
    return out;
}

u32 *__fastcall SoundObject_RemoveOccluder(u8 *object, void *, u32 *out, u8 **node) {
    u8 *const channel = *reinterpret_cast<u8 **>(object + 0x24);
    if (!channel) {
        *out = 0xffffffff;
        return out;
    }
    SoundChannel_RemoveOccluder(channel, nullptr, out, node);
    return out;
}

u32 *__fastcall SoundObject_ReleaseRecordBanks(u8 *object, void *, u32 *out, const u8 *record) {
    if (!*reinterpret_cast<u8 **>(object + 0x24) || !record) {
        *out = 0xffffffff;
        return out;
    }
    SoundCategory_ReleaseRecordBanks(out, record);
    return out;
}

u32 *__fastcall SoundObject_ReleaseRecordAt(u8 *, void *, u32 *out, u32 index) {
    if (!index) {
        *out = 0xffffffff;
        return out;
    }
    SoundCategory_ReleaseRecordAt(SoundCategory_Get(), nullptr, out, index);
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

u8 *__fastcall SoundObject_DeletingDtor(u8 *object, void *, u32 flags) {
    SoundObject_Dtor(object);
    if (flags & 1) GameHeap_Free(object);
    return object;
}

u32 __fastcall SoundObject_CallVoice10(u8 *object, void *, u32 id, u32 a, u32 b) {
    if (!id || !at<u8 *>(object, 0x20)) return 0;
    return SoundMixer_CallVoice10(at<u8 *>(object, 0x20), nullptr, id, a, b);
}

u32 *__fastcall SoundObject_Save(u8 *object, void *, u32 *out, u8 *stream) {
    const u32 fields[3] = {at<u32>(object, 0x28), at<u32>(object, 0x2c), at<u32>(object, 0x30)};
    for (const u32 &field : fields) {
        s32 result, done;
        Stream_Write(&result, reinterpret_cast<Stream *>(stream), &field, 4, &done);
    }
    u32 result;
    SoundMixer_Save(at<u8 *>(object, 0x20), nullptr, &result, stream);
    *out = 0;
    return out;
}

u32 *__fastcall SoundObject_Load(u8 *object, void *, u32 *out, u8 *stream, u32 restore) {
    u32 dropped;
    for (int i = 0; i < 3; ++i) vcall<u32>(stream, 8, &dropped, 4u);
    u32 result = restore;
    SoundMixer_Load(at<u8 *>(object, 0x20), nullptr, &result, stream, restore);
    *out = 0;
    return out;
}

u8 *__fastcall SoundObject_CreateChannel(u8 *object) {
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x68));
    u8 *const channel = block ? SoundChannel_Ctor(block) : nullptr;
    at<u8 *>(object, 0x24) = channel;
    return channel;
}

u8 *__fastcall SoundObjectEax_CreateChannel(u8 *object) {
    u8 *const channel = static_cast<u8 *>(GameHeap_Alloc(0x70));
    if (channel) {
        SoundChannel_Ctor(channel);
        at<u32>(channel, 0x68) = 0;
        at<u32>(channel, 0x6c) = 0;
        at<u32>(channel, 0) = 0x0070a630;
    }
    at<u8 *>(object, 0x24) = channel;
    return channel;
}

}  // extern "C"
