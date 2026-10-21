#include "mp2pool.h"

#include <cstdint>
#include <cstring>

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
constexpr Pool kBuffer = {0x00fb2710, 0x00fb270c, 0x2478, 0x2404};
constexpr Pool kStream = {0x00fb2718, 0x00fb2714, 0x2c80, 0x2478};

u8 *&Next(u8 *block, const Pool &p) { return *reinterpret_cast<u8 **>(block + p.next); }

void Init(const Pool &p) {
    void *const lock = GameHeap_Alloc(0x18);
    global<void *>(p.lock) = lock;
    Import(0x006fd144)(lock);   // InitializeCriticalSection
}

void Purge(const Pool &p) {
    Enter(global<void *>(p.lock));
    while (u8 *const block = global<u8 *>(p.list)) {
        global<u8 *>(p.list) = Next(block, p);
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
        global<u8 *>(p.list) = Next(block, p);
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
    Next(static_cast<u8 *>(block), p) = global<u8 *>(p.list);
    global<void *>(p.list) = block;
    Leave(global<void *>(p.lock));
}

}  // namespace

extern "C" {

void __cdecl Mp2BufferPool_Init() { Init(kBuffer); }
void __cdecl Mp2StreamPool_Init() { Init(kStream); }
void __cdecl Mp2BufferPool_Shutdown() { Shutdown(kBuffer); }
void __cdecl Mp2StreamPool_Shutdown() { Shutdown(kStream); }
void __cdecl Mp2BufferPool_Purge() { Purge(kBuffer); }
void __cdecl Mp2StreamPool_Purge() { Purge(kStream); }
void *__cdecl Mp2BufferPool_New(u32 size) { return New(kBuffer, size); }
void *__cdecl Mp2StreamPool_New(u32 size) { return New(kStream, size); }
void __cdecl Mp2BufferPool_Delete(void *block, u32 size) { Delete(kBuffer, block, size); }
void __cdecl Mp2StreamPool_Delete(void *block, u32 size) { Delete(kStream, block, size); }

}  // extern "C"
