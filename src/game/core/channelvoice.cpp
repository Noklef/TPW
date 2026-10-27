#include "channelvoice.h"

#include <cstdint>

#include <emmintrin.h>

#include "../math/x87.h"

#include "crt.h"
#include "gameheap.h"
#include "soundchannel.h"
#include "vcall.h"
#include "voiceevents.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }
using OnObject = s32(__stdcall *)(void *);

void Enter(u8 *voice) { Import<OnObject>(0x006fd140)(voice + 0x74); }   // EnterCriticalSection
void Leave(u8 *voice) { Import<OnObject>(0x006fd130)(voice + 0x74); }   // LeaveCriticalSection

}  // namespace

extern "C" {

void __cdecl ChannelVoiceCount_Reset() { *reinterpret_cast<s32 *>((uintptr_t)0x00fb2058) = 0; }

s32 __fastcall ChannelVoiceCount_Increment(s32 *count) { return Import<OnObject>(0x006fd0c0)(count); }

s32 __fastcall ChannelVoiceCount_Decrement(s32 *count) { return Import<OnObject>(0x006fd12c)(count); }

u8 *__fastcall ChannelVoice_Sound(u8 *voice) {
    Enter(voice);
    u8 *const sound = at<u8 *>(voice, 4);
    Leave(voice);
    return sound;
}

u8 __fastcall ChannelVoice_Release(u8 *voice, void *, u32 value) {
    Enter(voice);
    u8 released = 0;
    if (at<u32>(voice, 4)) {
        vcall<void>(voice, 0x2c, value);
        vcall<void>(at<u8 *>(voice, 4), 8);
        at<u32>(voice, 4) = 0;
        ChannelVoiceCount_Decrement(reinterpret_cast<s32 *>((uintptr_t)0x00fb2058));
        released = 1;
    }
    at<u32>(voice, 0x10) &= ~2u;
    at<u32>(voice, 0x58) = 0;
    Leave(voice);
    return released;
}

void __fastcall ChannelVoice_SetFlag(u8 *voice, void *, u8 on, u32 mask) {
    if (on)
        at<u32>(voice, 0x10) |= mask;
    else
        at<u32>(voice, 0x10) &= ~mask;
}

u32 __fastcall ChannelVoice_Apply(u8 *voice, void *, u8 *record, u32 *out) {
    if (!(voice[0x10] & 2)) {
        at<u32>(voice, 0x24) = 0;
        at<u32>(voice, 0x1c) = 0;
        at<u32>(voice, 0x10) = 2;
        at<u32>(voice, 0x14) = at<u32>(record, 0xc);
    }
    if (at<u32>(record, 0x14) & 0x400) {
        at<u32>(voice, 0x58) = at<u32>(record, 0x2c);
        at<u32>(record, 0x14) &= ~0x400u;
    }
    at<u32>(voice, 0x1c) = at<u32>(record, 8);
    if (record[0x14] & 2) at<u32>(voice, 0x18) = at<u32>(record, 0x28);
    if (at<u32>(record, 0x14) & 0x40000) {
        const u32 *const pair = at<const u32 *>(record, 0x94);
        at<u32>(voice, 0x34) = pair[0];
        at<u32>(voice, 0x38) = pair[1];
        at<u32>(voice, 0x24) = at<u32>(record, 0x98);
    }
    *out = at<u32>(voice, 0x2c) >> (*reinterpret_cast<const u8 *>(static_cast<uintptr_t>(0x00782db8)) & 31);
    return at<u32>(voice, 0x2c);
}

void __fastcall ChannelVoices_ReleaseAll(u8 *pool) {
    u8 *voice = at<u8 *>(pool, 4);
    const u32 count = at<u32>(pool, 8);
    if (!voice) return;
    u8 *const end = voice + count * 0x8c;
    do {
        u32 out;
        SoundChannel_ReleaseVoice(at<u8 *>(pool, 0xc), nullptr, &out, voice, 1);
        voice += 0x8c;
    } while (voice < end);
}

u32 __fastcall ChannelVoice_ScaledRate(const u8 *voice) {
    const u32 rate = at<u32>(voice, 0x18);
    const u32 mask = at<u32>(voice, 0x24);
    if (!mask) return rate;
    X87Reg factor = 1.0;
    if (mask & 1) factor = X87Reg(at<float>(at<const u8 *>(voice, 0x34), 0));
    if (mask & 2) factor = factor * at<float>(at<const u8 *>(voice, 0x34), 4);
    if (mask & 4) factor = factor * at<float>(at<const u8 *>(voice, 0x34), 8);
    if (mask & 8) factor = factor * at<float>(at<const u8 *>(voice, 0x34), 0xc);
    const float scaled = static_cast<float>(X87_MulUIntDouble(rate, factor.v));
    return static_cast<u32>(_mm_cvtss_si32(_mm_set_ss(scaled)));   // (fld; fistp)
}

void __fastcall VoiceId_SetHigh(u32 *id, void *, u32 value) {
    u8 &guard = *reinterpret_cast<u8 *>(static_cast<uintptr_t>(0xfb1f18));
    u32 &kept = *reinterpret_cast<u32 *>(static_cast<uintptr_t>(0xfb1f1c));
    const u32 *const shift = reinterpret_cast<const u32 *>(static_cast<uintptr_t>(0x782db8));
    u32 mask;
    if (!(guard & 1)) {
        guard |= 1;
        mask = (1u << ((*shift + 1) & 31)) - 1;
        kept = mask;
    } else {
        mask = kept;
    }
    *id &= mask;
    *id += (value & 0xffff) << (*shift & 31);
}

u8 *__fastcall ChannelVoices_Find(u8 *pool, void *, const u32 *handle) {
    u8 *const voice = at<u8 *>(pool, 4) + (*handle >> (*reinterpret_cast<const u8 *>(static_cast<uintptr_t>(0x00782db8)) & 31)) * 0x8c;
    return vcall<u32>(voice, 0x30, handle) ? voice : nullptr;
}

u8 *__fastcall ChannelVoices_FindOut(u8 *pool, void *, const u32 *handle, u32 *out) {
    u8 *const voice = at<u8 *>(pool, 4) + (*handle >> (*reinterpret_cast<const u8 *>(static_cast<uintptr_t>(0x00782db8)) & 31)) * 0x8c;
    const u32 answer = vcall<u32>(voice, 0x30, handle);
    *out = answer;
    return answer ? voice : nullptr;
}

u32 __fastcall ChannelVoices_Holds(u8 *pool, void *, const u32 *handle, u32 *out) {
    u8 *const voices = at<u8 *>(pool, 4);
    const u32 index = *handle >> (*reinterpret_cast<const u8 *>(static_cast<uintptr_t>(0x00782db8)) & 31);
    *out = index;
    return vcall<u32>(voices + index * 0x8c, 0x30, handle) ? 1 : 0;
}

u8 *__fastcall ChannelVoice_Ctor(u8 *voice) {
    at<u32>(voice, 4) = 0;
    at<u32>(voice, 8) = 0;
    at<u32>(voice, 0xc) = 0;
    at<u32>(voice, 0) = 0x0070a778;
    at<u32>(voice, 0x10) = 0;
    at<u32>(voice, 0x18) = 0;
    at<u32>(voice, 0x1c) = 0;
    at<u32>(voice, 0x20) = 0;
    at<u32>(voice, 0x24) = 0;
    at<u32>(voice, 0x2c) = 0;
    at<u32>(voice, 0x34) = 0;
    at<u32>(voice, 0x38) = 0;
    const uintptr_t block = reinterpret_cast<uintptr_t>(GameHeap_Alloc(0x2420));
    at<u32>(voice, 0x64) = static_cast<u32>(block);
    at<u32>(voice, 0x68) = static_cast<u32>((block + 0x1f) & ~static_cast<uintptr_t>(0x1f));
    using CreateEvent = u32(__stdcall *)(void *, u32, u32, const char *);
    at<u32>(voice, 0x6c) = Import<CreateEvent>(0x006fd0d4)(nullptr, 1, 0, nullptr);   // CreateEventA
    Import<OnObject>(0x006fd144)(voice + 0x74);   // InitializeCriticalSection
    at<u32>(voice, 0x58) = 0;
    at<u32>(voice, 0) = 0x0070a7a8;
    return voice;
}

u8 *__fastcall ChannelVoices_New(u8 *holder, void *, u32 count) {
    u8 *array = nullptr;
    if (u32 *const block = static_cast<u32 *>(GameHeap_Alloc(count * 0x8c + 4))) {
        block[0] = count;
        array = reinterpret_cast<u8 *>(block + 1);
        CRT_VectorCtorIterator(array, 0x8c, static_cast<s32>(count), reinterpret_cast<void(__fastcall *)(void *)>(ChannelVoice_Ctor),
                               reinterpret_cast<void *>(static_cast<uintptr_t>(0x006bb050)));
    }
    at<u8 *>(holder, 0) = array;
    return holder;
}

void __fastcall ChannelVoices_SetCount(u8 *pool, void *, const u8 *settings) {
    const u32 wanted = at<u32>(settings, 0x40);
    const u32 count = wanted < at<u32>(pool, 0) ? wanted : at<u32>(pool, 0);
    ChannelVoices_ReleaseAll(pool);
    at<u32>(pool, 8) = count;
}

u8 *__fastcall ChannelVoices_Ctor(u8 *pool, void *, u8 *channel, u32 count) {
    at<u32>(pool, 0) = count;
    ChannelVoices_New(pool + 4, nullptr, count);
    at<u32>(pool, 8) = 0;
    at<u8 *>(pool, 0xc) = channel;
    u8 *const block = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(pool, 0x10) = block ? VoiceEvents_Ctor(block, nullptr, at<u8 *>(pool, 4), at<u32>(pool, 0)) : nullptr;
    return pool;
}

u32 __fastcall ChannelVoice_Refill(u8 *voice) {
    u32 done;   // (the read's out: not read after)
    const u32 read = vcall<u32>(voice, 0x1c, at<u32>(voice, 0x68), 0x2400u, &done, 0u);
    at<u32>(voice, 0x30) = read != 1;
    Import<OnObject>(0x006fd168)(reinterpret_cast<void *>(static_cast<uintptr_t>(*at<u32 *>(voice, 0x70))));   // ResetEvent
    Import<OnObject>(0x006fd124)(reinterpret_cast<void *>(static_cast<uintptr_t>(at<u32>(voice, 0x6c))));    // SetEvent
    return at<u32>(voice, 0x30);
}

u32 __fastcall ChannelVoices_ReleaseEach(u8 *pool) {
    u8 *voice = at<u8 *>(pool, 4);
    u8 *const end = voice + at<u32>(pool, 8) * 0x8c;
    u32 released = 0;
    if (voice) {
        do {   // (the first voice released whatever the count, sic)
            released += ChannelVoice_Release(voice, nullptr, 0);
            voice += 0x8c;
        } while (voice < end && voice);
    }
    return released;
}

}  // extern "C"
