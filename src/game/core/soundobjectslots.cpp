// The sound object's small vtable slots and two update functors: see soundobjectslots.h.
#include "soundobjectslots.h"

#include <cstdint>

#include "channelvoice.h"
#include "soundchannel.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

const SoundObjectSlotsEnv kGameEnv = {SoundUpdate_Apply, ChannelVoices_Holds, ChannelVoices_PlayRecord, ChannelVoices_PlayNotify, ChannelVoice_Mix};
const SoundObjectSlotsEnv *g_env = &kGameEnv;

}  // namespace

void SoundObjectSlots_SetEnv(const SoundObjectSlotsEnv *env) { g_env = env ? env : &kGameEnv; }

u32 __fastcall SoundObject_PlayRecord(u8 *object, void *, u32 handle, u8 *record) {
    u32 played = (u32)(uintptr_t)record;  // sic: the argument's own slot
    const u32 answer = g_env->play(at<u8 *>(object, 0x14), nullptr, handle, record, &played);
    if (answer) {
        u32 functor[4] = {0x0070a580, answer, (u32)(uintptr_t)record, played};
        vcall<u32 *>(object, 0x10, &handle, functor);
    }
    return answer;
}

u32 __fastcall SoundObject_PlayNotify(u8 *object, void *, u32 handle, u8 *record) { return g_env->notify(at<u8 *>(object, 0x14), nullptr, handle, record); }

u32 *__fastcall SoundUpdateById_Apply(const u8 *update, void *, u32 *out, u8 *channel) {
    g_env->apply(update, nullptr, out, channel, at<u32>(update, 0xc));
    return out;
}

u32 *__fastcall SoundUpdateByHandle_Apply(const u8 *update, void *, u32 *out, u8 *channel) {
    u32 handle = at<u32>(update, 4);
    if (handle) {
        u32 id = (u32)(uintptr_t)channel;  // sic: the argument's own slot
        if ((u8)g_env->holds(at<u8 *>(channel, 0x14), nullptr, &handle, &id)) {
            g_env->apply(update, nullptr, out, channel, id);
            return out;
        }
    }
    *out = 0xffffffffu;
    return out;
}

float __fastcall SoundObject_SlotZero(void *, void *, u32) { return 0.0f; }

float __fastcall SoundObject_SlotOne(void *, void *, u32) { return 1.0f; }

float __fastcall SoundObject_SlotByFlag(const u8 *object) { return at<u8>(object, 0x28) & 0x40 ? 0.2f : 0.8f; }

u32 __stdcall SoundVoice_MixCallback(u8 *dest, u32 size, u8 *voice) { return g_env->mix(voice, nullptr, dest, size); }
