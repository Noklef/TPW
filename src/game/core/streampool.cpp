#include "streampool.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

using OnLock = void(__stdcall *)(void *);
OnLock Import(u32 slot) { return *reinterpret_cast<const OnLock *>((uintptr_t)slot); }

void Enter(void *lock) { Import(0x006fd140)(lock); }   // EnterCriticalSection
void Leave(void *lock) { Import(0x006fd130)(lock); }   // LeaveCriticalSection

struct Pool {
    u32 lock;    // the global holding the lock's block
    u32 list;    // the global holding the free list's first block
    u32 size;    // the blocks' size
    u32 next;    // where a block keeps the next one
};
Pool pool(StreamPoolGlobals &g, u32 size, u32 next) { return {(u32)(uintptr_t)&g.lock, (u32)(uintptr_t)&g.list, size, next}; }

const Pool kFilter = pool(gStreampool_Filter_fb2068, 0x18, 0x10);
const Pool kLoop = pool(gStreampool_Loop_fb2080, 0x24, 0x18);
const Pool k70abf8 = pool(gStreampool_70abf8_fb208c, 0x14, 0x10);
const Pool k70a6c8 = pool(gStreampool_70a6c8_fb20a4, 0x1c, 0x14);
const Pool kVoiceSample = pool(gStreampool_VoiceSample_fb20bc, 0x40, 0x28);
const Pool kVoiceStream = pool(gStreampool_VoiceStream_fb20c8, 0x5c, 0x58);
const Pool kSilence = pool(gStreampool_Silence_fb2074, 0x30, 0x28);
const Pool kFileSample = pool(gStreampool_FileSample_fb20b0, 0x48, 0x44);
const Pool kMp2Stream = pool(gStreampool_Mp2Stream_fb26e4, 0x5c, 0x58);
const Pool kMp2Sample = pool(gStreampool_Mp2Sample_fb26d8, 0x40, 0x28);
const Pool kCallback = pool(gStreampool_Callback_fb26f0, 0x1c, 0x14);
const Pool kMemorySample = pool(gStreampool_MemorySample_fb2098, 0x2c, 0x28);

void Init(const Pool &p) {
    void *const lock = GameHeap_Alloc(0x18);
    global<void *>(p.lock) = lock;
    Import(0x006fd144)(lock);   // InitializeCriticalSection
}

void Purge(const Pool &p) {
    Enter(global<void *>(p.lock));
    while (u8 *const block = global<u8 *>(p.list)) {
        global<u8 *>(p.list) = *reinterpret_cast<u8 **>(block + p.next);
        GameHeap_Free(block);
    }
    Leave(global<void *>(p.lock));
}

void Shutdown(const Pool &p) {
    Purge(p);
    Import(0x006fd0d0)(global<void *>(p.lock));   // DeleteCriticalSection
    GameHeap_Free(global<void *>(p.lock));
    global<void *>(p.lock) = nullptr;
}

void *New(const Pool &p, u32 size) {
    if (!size) size = 1;
    if (size != p.size) return GameHeap_Alloc(size);
    Enter(global<void *>(p.lock));
    u8 *block = global<u8 *>(p.list);
    if (block) {
        global<u8 *>(p.list) = *reinterpret_cast<u8 **>(block + p.next);
    } else {
        block = static_cast<u8 *>(GameHeap_Alloc(p.size));
    }
    Leave(global<void *>(p.lock));
    return block;
}

void Delete(const Pool &p, void *block, u32 size) {
    if (!block) return;
    if (size != p.size) {
        GameHeap_Free(block);
        return;
    }
    Enter(global<void *>(p.lock));
    *reinterpret_cast<u8 **>(static_cast<u8 *>(block) + p.next) = global<u8 *>(p.list);
    global<void *>(p.list) = block;
    Leave(global<void *>(p.lock));
}

}  // namespace

extern "C" {

void __cdecl StreamFilterPool_Init() { Init(kFilter); }
void __cdecl LoopStreamPool_Init() { Init(kLoop); }
void __cdecl Stream70abf8Pool_Init() { Init(k70abf8); }
void __cdecl Stream70a6c8Pool_Init() { Init(k70a6c8); }
void __cdecl VoiceSamplePool_Init() { Init(kVoiceSample); }
void __cdecl VoiceStreamPool_Init() { Init(kVoiceStream); }
void __cdecl StreamFilterPool_Shutdown() { Shutdown(kFilter); }
void __cdecl LoopStreamPool_Shutdown() { Shutdown(kLoop); }
void __cdecl Stream70abf8Pool_Shutdown() { Shutdown(k70abf8); }
void __cdecl Stream70a6c8Pool_Shutdown() { Shutdown(k70a6c8); }
void __cdecl VoiceSamplePool_Shutdown() { Shutdown(kVoiceSample); }
void __cdecl VoiceStreamPool_Shutdown() { Shutdown(kVoiceStream); }
void __cdecl StreamFilterPool_Purge() { Purge(kFilter); }
void __cdecl LoopStreamPool_Purge() { Purge(kLoop); }
void __cdecl Stream70abf8Pool_Purge() { Purge(k70abf8); }
void __cdecl Stream70a6c8Pool_Purge() { Purge(k70a6c8); }
void __cdecl VoiceSamplePool_Purge() { Purge(kVoiceSample); }
void __cdecl VoiceStreamPool_Purge() { Purge(kVoiceStream); }
void *__cdecl StreamFilterPool_New(u32 size) { return New(kFilter, size); }
void *__cdecl LoopStreamPool_New(u32 size) { return New(kLoop, size); }
void *__cdecl Stream70abf8Pool_New(u32 size) { return New(k70abf8, size); }
void *__cdecl Stream70a6c8Pool_New(u32 size) { return New(k70a6c8, size); }
void *__cdecl VoiceSamplePool_New(u32 size) { return New(kVoiceSample, size); }
void *__cdecl VoiceStreamPool_New(u32 size) { return New(kVoiceStream, size); }
void __cdecl StreamFilterPool_Delete(void *block, u32 size) { Delete(kFilter, block, size); }
void __cdecl LoopStreamPool_Delete(void *block, u32 size) { Delete(kLoop, block, size); }
void __cdecl Stream70abf8Pool_Delete(void *block, u32 size) { Delete(k70abf8, block, size); }
void __cdecl Stream70a6c8Pool_Delete(void *block, u32 size) { Delete(k70a6c8, block, size); }
void __cdecl VoiceSamplePool_Delete(void *block, u32 size) { Delete(kVoiceSample, block, size); }
void __cdecl VoiceStreamPool_Delete(void *block, u32 size) { Delete(kVoiceStream, block, size); }
void __cdecl SilenceStreamPool_Init() { Init(kSilence); }
void __cdecl SilenceStreamPool_Shutdown() { Shutdown(kSilence); }
void __cdecl SilenceStreamPool_Purge() { Purge(kSilence); }
void *__cdecl SilenceStreamPool_New(u32 size) { return New(kSilence, size); }
void __cdecl SilenceStreamPool_Delete(void *block, u32 size) { Delete(kSilence, block, size); }
void __cdecl FileSamplePool_Init() { Init(kFileSample); }
void __cdecl FileSamplePool_Shutdown() { Shutdown(kFileSample); }
void __cdecl FileSamplePool_Purge() { Purge(kFileSample); }
void *__cdecl FileSamplePool_New(u32 size) { return New(kFileSample, size); }
void __cdecl FileSamplePool_Delete(void *block, u32 size) { Delete(kFileSample, block, size); }
void __cdecl Mp2StreamSamplePool_Init() { Init(kMp2Stream); }
void __cdecl Mp2StreamSamplePool_Shutdown() { Shutdown(kMp2Stream); }
void __cdecl Mp2StreamSamplePool_Purge() { Purge(kMp2Stream); }
void *__cdecl Mp2StreamSamplePool_New(u32 size) { return New(kMp2Stream, size); }
void __cdecl Mp2StreamSamplePool_Delete(void *block, u32 size) { Delete(kMp2Stream, block, size); }
void __cdecl Mp2SamplePool_Init() { Init(kMp2Sample); }
void __cdecl Mp2SamplePool_Shutdown() { Shutdown(kMp2Sample); }
void __cdecl Mp2SamplePool_Purge() { Purge(kMp2Sample); }
void *__cdecl Mp2SamplePool_New(u32 size) { return New(kMp2Sample, size); }
void __cdecl Mp2SamplePool_Delete(void *block, u32 size) { Delete(kMp2Sample, block, size); }
void __cdecl NotifyStreamPool_Init() { Init(kCallback); }
void __cdecl NotifyStreamPool_Shutdown() { Shutdown(kCallback); }
void __cdecl NotifyStreamPool_Purge() { Purge(kCallback); }
void *__cdecl NotifyStreamPool_New(u32 size) { return New(kCallback, size); }
void __cdecl NotifyStreamPool_Delete(void *block, u32 size) { Delete(kCallback, block, size); }
void __cdecl MemorySamplePool_Init() { Init(kMemorySample); }
void __cdecl MemorySamplePool_Shutdown() { Shutdown(kMemorySample); }
void __cdecl MemorySamplePool_Purge() { Purge(kMemorySample); }
void *__cdecl MemorySamplePool_New(u32 size) { return New(kMemorySample, size); }
void __cdecl MemorySamplePool_Delete(void *block, u32 size) { Delete(kMemorySample, block, size); }

}  // extern "C"
