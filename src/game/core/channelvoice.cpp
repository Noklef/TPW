#include "channelvoice.h"

#include <cstdint>

#include "soundchannel.h"
#include "vcall.h"

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

void __fastcall ChannelVoices_SetCount(u8 *pool, void *, const u8 *settings) {
    const u32 wanted = at<u32>(settings, 0x40);
    const u32 count = wanted < at<u32>(pool, 0) ? wanted : at<u32>(pool, 0);
    ChannelVoices_ReleaseAll(pool);
    at<u32>(pool, 8) = count;
}

}  // extern "C"
