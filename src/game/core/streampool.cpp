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
constexpr Pool kFilter = {0x00fb206c, 0x00fb2068, 0x18, 0x10};
constexpr Pool kLoop = {0x00fb2084, 0x00fb2080, 0x24, 0x18};
constexpr Pool k70abf8 = {0x00fb2090, 0x00fb208c, 0x14, 0x10};
constexpr Pool k70a6c8 = {0x00fb20a8, 0x00fb20a4, 0x1c, 0x14};

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

void __cdecl StreamFilterPool_Delete(void *block, u32 size) { Delete(kFilter, block, size); }
void __cdecl LoopStreamPool_Delete(void *block, u32 size) { Delete(kLoop, block, size); }
void __cdecl Stream70abf8Pool_Delete(void *block, u32 size) { Delete(k70abf8, block, size); }
void __cdecl Stream70a6c8Pool_Delete(void *block, u32 size) { Delete(k70a6c8, block, size); }

}  // extern "C"
