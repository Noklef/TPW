#include "channelvoice.h"

#include <cstdint>
#include <cstring>

#include <emmintrin.h>

#include "../math/x87.h"

#include "crt.h"
#include "gameheap.h"
#include "language.h"
#include "mp2sample.h"
#include "semaphore.h"
#include "soundchannel.h"
#include "soundindex.h"
#include "soundsample.h"
#include "soundstream.h"
#include "streampool.h"
#include "vcall.h"
#include "voiceevents.h"
#include "voicesample.h"
#include "imports.h"
#include "staticobjects.h"
#include "addrcalls.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }
using OnObject = s32(__stdcall *)(void *);

void Enter(u8 *voice) { Import<OnObject>(IMPORT(Kernel32_EnterCriticalSection))(voice + 0x74); }   // EnterCriticalSection
void Leave(u8 *voice) { Import<OnObject>(IMPORT(Kernel32_LeaveCriticalSection))(voice + 0x74); }   // LeaveCriticalSection

// A sound resource reference's resource (+8: 0x41a830) and its stream (slot 11).
u8 *Source(u8 *ref) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(Language_Id(ref))); }
u8 *Stream(u8 *ref) { return reinterpret_cast<u8 *>(static_cast<uintptr_t>(SoundResourceRef_Slot11(ref))); }

}  // namespace

extern "C" {

void __cdecl ChannelVoiceCount_Reset() { *reinterpret_cast<s32 *>((uintptr_t)0x00fb2058) = 0; }

s32 __fastcall ChannelVoiceCount_Increment(s32 *count) { return Import<OnObject>(IMPORT(Kernel32_InterlockedIncrement))(count); }

s32 __fastcall ChannelVoiceCount_Decrement(s32 *count) { return Import<OnObject>(IMPORT(Kernel32_InterlockedDecrement))(count); }

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

void __fastcall ChannelVoice_Dtor(u8 *voice) {
    at<u32>(voice, 0) = (u32)(uintptr_t)Class70a7a8_vtable;
    Import<OnObject>(IMPORT(Kernel32_DeleteCriticalSection))(voice + 0x74);   // DeleteCriticalSection
    Import<OnObject>(IMPORT(Kernel32_CloseHandle))(reinterpret_cast<void *>(static_cast<uintptr_t>(at<u32>(voice, 0x6c))));   // CloseHandle
    GameHeap_Free(at<u8 *>(voice, 0x64));
    at<u32>(voice, 0) = (u32)(uintptr_t)Class70a778_vtable;
}

u8 *__fastcall ChannelVoice_DeletingDtor(u8 *voice, void *, u32 flags) {
    if (flags & 2) {
        CRT_VectorDtorIterator(voice, 0x8c, at<s32>(voice, static_cast<u32>(-4)), reinterpret_cast<void(__fastcall *)(void *)>(ChannelVoice_Dtor));
        GameHeap_Free(voice - 4);   // (sic: flag 1 not tested)
        return voice;
    }
    ChannelVoice_Dtor(voice);
    if (flags & 1) GameHeap_Free(voice);
    return voice;
}

u8 *__fastcall ChannelVoice_SetNext(u8 *voice, void *, u8 *next) {
    Enter(voice);
    u8 *const result = SoundStream_SetNext(voice, nullptr, next);
    Leave(voice);
    return result;
}

u8 *__fastcall ChannelVoice_Find(u8 *voice, void *, const u32 *id) {
    Enter(voice);
    u8 *const result = SoundStream_Find(voice, nullptr, *id);
    Leave(voice);
    return result;
}

u32 *__fastcall ChannelVoice_Query(u8 *voice, void *, u32 *out, u8 *record) {
    Enter(voice);
    u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(record));   // (the record argument's own stack slot)
    SoundStream_Query(voice, nullptr, &slot, record);
    if (at<u32>(record, 0x14) & 0x400) at<u32>(voice, 0x58) = at<u32>(record, 0x2c);
    *out = 0;   // (sic: the query's answer dropped)
    Leave(voice);
    return out;
}

u32 __fastcall ChannelVoice_Read(u8 *voice, void *, u8 *buffer, u32 bytes, u32 *out, u32) {
    Enter(voice);
    *out = 0;
    u32 ended = 0;
    if (u8 *const next = at<u8 *>(voice, 4)) {
        if (vcall<u32>(next, 0x1c, buffer, bytes, out, 0) == 1) {
            ChannelVoice_Release(voice, nullptr, 0);
            ended = 1;
        }
    }
    std::memset(buffer + *out, 0, bytes - *out);
    Leave(voice);
    return ended;
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
    at<u32>(voice, 0) = (u32)(uintptr_t)Class70a778_vtable;
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
    at<u32>(voice, 0x6c) = Import<CreateEvent>(IMPORT(Kernel32_CreateEventA))(nullptr, 1, 0, nullptr);   // CreateEventA
    Import<OnObject>(IMPORT(Kernel32_InitializeCriticalSection))(voice + 0x74);   // InitializeCriticalSection
    at<u32>(voice, 0x58) = 0;
    at<u32>(voice, 0) = (u32)(uintptr_t)Class70a7a8_vtable;
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

namespace {

const ChannelVoicesEnv kVoicesEnv = {GameAddress<u32(__cdecl *)(u32, u32, void *)>(0x0067e050), ChannelVoices_Find};
const ChannelVoicesEnv *g_voices = &kVoicesEnv;

u32 createEvent() {
    using Create = u32(__stdcall *)(void *, u32, u32, const char *);
    return Import<Create>(IMPORT(Kernel32_CreateEventA))(nullptr, 1, 0, nullptr);   // CreateEventA
}

}  // namespace

void ChannelVoices_SetEnv(const ChannelVoicesEnv *env) { g_voices = env ? env : &kVoicesEnv; }

u32 *__fastcall ChannelVoices_Start(u8 *pool, void *, u32 *out, u32 requested) {
    const u32 count = at<u32>(pool, 0);
    at<u32>(pool, 8) = requested < count ? requested : count;
    if (count) {
        for (u32 i = 0; (u16)i < at<u32>(pool, 0); ++i) {
            VoiceId_SetHigh(reinterpret_cast<u32 *>(at<u8 *>(pool, 4) + i * 0x8c + 0x2c), nullptr, i);
            at<u32 *>(at<u8 *>(pool, 0x10), 0x10)[i] = createEvent();
            at<u32 *>(at<u8 *>(pool, 4) + i * 0x8c, 0x70) = at<u32 *>(at<u8 *>(pool, 0x10), 0x10) + i;
        }
    }
    u32 *const stop = at<u32 *>(at<u8 *>(pool, 0x10), 0x10) + at<u32>(pool, 0);
    *stop = createEvent();
    const u32 thread = g_voices->beginThread(0x006ba220, 0, pool);
    using SetPriority = s32(__stdcall *)(u32, s32);
    Import<SetPriority>(IMPORT(Kernel32_SetThreadPriority))(thread, 0);   // SetThreadPriority
    *out = thread ? 0 : 0xffffffffu;
    return out;
}

u32 *__fastcall ChannelVoiceHandle_Resume(const u8 *self, void *, u32 *out, u8 *owner) {
    SemaphoreLock lock;
    SemaphoreLock_Ctor(&lock, nullptr, reinterpret_cast<const Semaphore *>((uintptr_t)&gSemaphore_fb205c), 0);
    SemaphoreLock_Acquire(&lock, nullptr, 1);
    u32 handle = at<u32>(self, 4);
    u8 *const voice = handle ? g_voices->find(at<u8 *>(owner, 0x14), nullptr, &handle) : nullptr;
    if (voice && Import<OnObject>(IMPORT(Kernel32_ResetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(at<u32>(voice, 0x6c)))) &&   // ResetEvent
        Import<OnObject>(IMPORT(Kernel32_SetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(*at<u32 *>(voice, 0x70))))) {          // SetEvent
        vcall<void>(at<u8 *>(owner, 0x18), 0x24, out, voice);
    } else {
        *out = 0xffffffffu;
    }
    SemaphoreLock_Dtor(&lock);
    return out;
}

u32 __fastcall ChannelVoice_Refill(u8 *voice) {
    u32 done;   // (the read's out: not read after)
    const u32 read = vcall<u32>(voice, 0x1c, at<u32>(voice, 0x68), 0x2400u, &done, 0u);
    at<u32>(voice, 0x30) = read != 1;
    Import<OnObject>(IMPORT(Kernel32_ResetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(*at<u32 *>(voice, 0x70))));   // ResetEvent
    Import<OnObject>(IMPORT(Kernel32_SetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(at<u32>(voice, 0x6c))));    // SetEvent
    return at<u32>(voice, 0x30);
}

void __fastcall ChannelVoices_Dtor(u8 *pool) {
    ChannelVoices_ReleaseEach(pool);
    VoiceEvents_Stop(*reinterpret_cast<u8 **>(pool + 0x10));
    u8 *const events = *reinterpret_cast<u8 **>(pool + 0x10);
    while (*reinterpret_cast<volatile u8 *>(events + 0x14) & 2) {
    }
    if (events) vcall<void>(events, 0, 1u);
    if (u8 *const voices = *reinterpret_cast<u8 **>(pool + 4)) vcall<void>(voices, 0, 3u);
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

u8 *__fastcall ChannelVoices_CreateStream(u8 *, void *, u8 *voice, u32 *play) {
    u32 buffer[10];
    u32 *record = buffer;   // (the record read may point it elsewhere)
    u8 *stream = nullptr;
    if (static_cast<u8>(SoundResourceHandle_ReadRecord(static_cast<u32>(reinterpret_cast<uintptr_t>(&record)), play)) && record[1]) {
        if (voice) {
            for (u32 i = 0; i < 10; ++i) at<u32>(voice, 0x3c + 4 * i) = record[i];
            if (play[5] & 0x8000) at<u32>(voice, 0x20) = play[7];
        }
        u8 *const ref = reinterpret_cast<u8 *>(static_cast<uintptr_t>(play[1]));
        const u8 kind = reinterpret_cast<const u8 *>(record)[0x1b];
        const u8 *const header = reinterpret_cast<const u8 *>(record);
        if (kind & 0x20) {   // MP2
            if (SoundResourceRef_Slot13(ref)) {
                const u32 start = SoundResourceRef_Offset(ref, nullptr, play[0]) + 0x28;
                if (u8 *const block = static_cast<u8 *>(Mp2StreamSamplePool_New(0x5c))) {
                    u8 *const source = Source(ref);
                    stream = Mp2StreamSample_Ctor(block, nullptr, Stream(ref), header, start, source);
                }
            } else if (u8 *const block = static_cast<u8 *>(Mp2SamplePool_New(0x40))) {
                stream = Mp2Sample_Ctor(block, nullptr, header, Source(ref));
            }
        } else if (kind & 0x10) {   // voice-coded
            if (SoundResourceRef_Slot13(ref)) {
                SoundResourceRef_Offset(ref, nullptr, play[0]);   // (the answer unused)
                if (u8 *const block = static_cast<u8 *>(VoiceStreamPool_New(0x5c))) {
                    u8 *const source = Source(ref);
                    stream = VoiceStream_Ctor(block, nullptr, Stream(ref), header, source);
                }
            } else if (u8 *const block = static_cast<u8 *>(VoiceSamplePool_New(0x40))) {
                stream = VoiceSample_Ctor(block, nullptr, header, Source(ref));
            }
        } else if (kind & 0x40) {
            u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(play));   // (the play argument's own stack slot)
            SoundResourceRef_Slot15(ref, nullptr, &slot, static_cast<u32>(reinterpret_cast<uintptr_t>(play)));
            return nullptr;   // (no voice released, sic)
        } else if (SoundResourceRef_Slot13(ref)) {   // PCM from a file
            const u32 start = SoundResourceRef_Offset(ref, nullptr, play[0]) + 0x28;
            if (u8 *const block = static_cast<u8 *>(FileSamplePool_New(0x48))) {
                u8 *const source = Source(ref);
                stream = FileSample_Ctor(block, nullptr, Stream(ref), reinterpret_cast<u8 *>(record), start, source);
            }
        } else if (u8 *const block = static_cast<u8 *>(MemorySamplePool_New(0x2c))) {   // PCM in memory
            stream = SoundSample_Ctor(block, nullptr, reinterpret_cast<u8 *>(record), Source(ref));
            at<u32>(stream, 0) = (u32)(uintptr_t)Class70a6f8_vtable;
        }
        if (stream) stream = SoundStream_BuildChain(reinterpret_cast<u8 *>(play), header, stream);
        if (stream) {
            if (play[5] & 0x800) {
                u32 *const wrap = static_cast<u32 *>(Stream70a6c8Pool_New(0x1c));
                if (wrap) {
                    wrap[1] = 0;
                    wrap[2] = 0;
                    wrap[3] = 0;
                    wrap[4] = 0;
                    wrap[6] = play[0x13];
                    wrap[0] = (u32)(uintptr_t)Class70a6c8_vtable;
                }
                stream = SoundStream_Chain(stream, reinterpret_cast<u8 *>(wrap));
                play[5] &= ~0x800u;
            }
            const u32 id = ++at<u32>(voice, 0x2c);   // (no voice: a fault, as in the original)
            vcall<void>(stream, 0x28, id);
            u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(play));   // (the play argument's own stack slot)
            if (static_cast<s32>(*vcall<const u32 *>(stream, 0x24, &slot, play)) >= 0) {
                if (voice && (reinterpret_cast<const u8 *>(record)[0x1b] & 0x20)) {
                    if (at<u16>(voice, 0x54) != vcall<u32>(stream, 0x18)) at<u16>(voice, 0x54) = static_cast<u16>(vcall<u32>(stream, 0x18));
                }
                return stream;
            }
            vcall<void>(stream, 8);
        }
    }
    if (voice) ChannelVoice_Release(voice, nullptr, 0);
    return nullptr;
}

void __fastcall CriticalSectionLock_Leave(void **lock) { Import<OnObject>(IMPORT(Kernel32_LeaveCriticalSection))(*lock); }

void __fastcall ChannelVoice_SoundAndValue(u8 *voice, void *, u32 *sound, u32 *value) {
    Enter(voice);
    *sound = at<u32>(voice, 4);
    *value = at<u32>(voice, 0x58);
    Leave(voice);
}

u32 *__fastcall ChannelVoice_Send(u8 *voice, void *, u32 *out, u32 handle, u32 value) {
    Enter(voice);
    if (u8 *const target = vcall<u8 *>(voice, 0x30, &handle)) {
        vcall<void>(target, 0x24, out, value);
    } else {
        *out = 0xffffffff;
    }
    Leave(voice);
    return out;
}

u32 __fastcall ChannelVoice_Mix(u8 *voice, void *, u8 *dest, u32 size) {
    using Wait = u32(__stdcall *)(u32, u32);
    if (Import<Wait>(IMPORT(Kernel32_WaitForSingleObject))(at<u32>(voice, 0x6c), 500) == 0x102) {   // (WaitForSingleObject: timed out, silence)
        std::memset(dest, 0, size);
        return 1;
    }
    u8 *const copier = *reinterpret_cast<u8 **>(static_cast<uintptr_t>(0x00fb2018));
    vcall<void>(copier, 0, dest, at<u32>(voice, 0x68), size);
    Import<OnObject>(IMPORT(Kernel32_ResetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(at<u32>(voice, 0x6c))));   // ResetEvent
    if (at<u32>(voice, 0x30)) Import<OnObject>(IMPORT(Kernel32_SetEvent))(reinterpret_cast<void *>(static_cast<uintptr_t>(*at<u32 *>(voice, 0x70))));   // SetEvent
    return at<u32>(voice, 0x30);
}

u32 *__fastcall ChannelVoices_Send(u8 *pool, void *, u32 *out, u32 handle, u32 value) {
    const u32 id = handle;   // (slot 12 has the argument's own slot; the id sent is the one passed)
    if (u8 *const voice = ChannelVoices_Find(pool, nullptr, &handle)) return ChannelVoice_Send(voice, nullptr, out, id, value);
    *out = 0xffffffff;
    return out;
}

u8 *__fastcall ChannelVoices_Acquire(u8 *pool, void *, u8 *desc) {
    u8 *const first = at<u8 *>(pool, 4);
    u8 *const end = first + at<u32>(pool, 8) * 0x8c;
    u8 *best = nullptr;
    u32 oldest = 0xffffffff;
    u32 slot = static_cast<u32>(reinterpret_cast<uintptr_t>(desc));   // (the argument's slot: the releases' out)
    for (u8 *voice = first; voice;) {
        u32 sound, value;
        ChannelVoice_SoundAndValue(voice, nullptr, &sound, &value);
        if (at<u32>(desc, 0xc) & 0x200 && desc[0x14] & 4 && at<u32>(voice, 0x1c) == at<u32>(desc, 8)) {
            SoundChannel_ReleaseVoice(at<u8 *>(pool, 0xc), nullptr, &slot, voice, 0);
            return voice;
        }
        if (!sound) {
            const u32 index = at<u32>(voice, 0x2c) >> (*reinterpret_cast<const u8 *>(static_cast<uintptr_t>(0x00782db8)) & 31);
            if (!vcall<u32>(at<u8 *>(at<u8 *>(pool, 0xc), 0x18), 0x44, index)) return voice;
        }
        if (value < oldest) {
            oldest = value;
            best = voice;
        }
        voice += 0x8c;
        if (voice >= end) voice = nullptr;
    }
    if (at<u32>(desc, 0x2c) <= oldest) return nullptr;
    SoundChannel_ReleaseVoice(at<u8 *>(pool, 0xc), nullptr, &slot, best, 1);
    at<u32>(desc, 0x10) = at<u32>(best, 0x2c);
    at<u32>(desc, 8) = at<u32>(best, 0x1c);
    return best;
}

u8 *__fastcall ChannelVoices_AcquireBy(u8 *pool, void *, u32 handle, u8 **out, u8 *desc) {
    if (handle) {
        u32 answer = 0;
        u8 *const voice = ChannelVoices_FindOut(pool, nullptr, &handle, &answer);
        *out = voice;
        return voice ? reinterpret_cast<u8 *>(static_cast<uintptr_t>(answer)) : *out;
    }
    u8 *const voice = ChannelVoices_Acquire(pool, nullptr, desc);
    *out = voice;
    if (voice) {
        at<u32>(voice, 0x58) = at<u32>(desc, 0x2c);
        ChannelVoiceCount_Increment(reinterpret_cast<s32 *>(static_cast<uintptr_t>(0x00fb2058)));
    }
    return *out;
}

u32 __fastcall ChannelVoices_PlayRecord(u8 *pool, void *, u32 handle, u8 *record, u32 *out) {
    const u32 flags = at<u32>(record, 0x14);
    if (!(flags & 0x80) || !(flags & 8)) return 0;
    if (!SoundResourceRef_Slot12(at<u8 *>(record, 4)))
        return static_cast<u32>(reinterpret_cast<uintptr_t>(ChannelVoices_CreateStream(pool, nullptr, nullptr, reinterpret_cast<u32 *>(record))));
    u8 *voice;
    u8 *const answer = ChannelVoices_AcquireBy(pool, nullptr, handle, &voice, record);   // (the found voice's slot 12's answer, called below; sic)
    if (!answer) return 0;
    u8 *const stream = ChannelVoices_CreateStream(pool, nullptr, voice, reinterpret_cast<u32 *>(record));
    if (!vcall<u32>(answer, 4, stream)) return 0;
    return ChannelVoice_Apply(voice, nullptr, record, out);
}

u32 __fastcall ChannelVoices_PlayNotify(u8 *pool, void *, u32 handle, u8 *record) {
    if (!SoundResourceRef_Slot12(at<u8 *>(record, 4))) return 0;
    if (!(at<u32>(record, 0x14) & 0x2000)) return 0;
    u8 *voice;
    u8 *const answer = ChannelVoices_AcquireBy(pool, nullptr, handle, &voice, record);
    if (!answer) return 0;
    u8 *const stream = NotifyStream_Create(pool, nullptr, record);
    u8 *const next = vcall<u8 *>(answer, 4, stream);
    if (!next) return 0;
    const u32 counter = ++at<u32>(voice, 0x2c);
    return vcall<u32>(next, 0x28, counter);
}

}  // extern "C"
