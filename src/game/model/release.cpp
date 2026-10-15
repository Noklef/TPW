#include "release.h"

#include <cstdint>

#include "../core/globalmem.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(address); }

}  // namespace

extern "C" {

void __cdecl Instance_FreeRecordBlock(const u8 *instance) {
    const u8 *const model = at<const u8 *>(at<const u8 *>(instance, 8), 4);
    if (!(at<u32>(model, 0x30) & 0x8000)) return;
    u8 *record = at<u8 *>(model, 0x54);
    for (u32 i = 0; i < at<u16>(model, 0x36); ++i, record += 0x10) {
        if (*record & 8) {
            GlobalMem_Free(at<void *>(record, 0xc));
            at<void *>(record, 0xc) = nullptr;
            return;
        }
    }
}

void __cdecl Instance_FreeBlockCC(u8 *instance) {
    GlobalMem_Free(at<void *>(instance, 0xcc));
    at<void *>(instance, 0xcc) = nullptr;
}

void __cdecl Instance_FreeHolderBlock(const u8 *instance) {
    GlobalMem_Free(at<void *>(at<u8 *>(instance, 8), 0x18));
    at<void *>(at<u8 *>(instance, 8), 0x18) = nullptr;
}

s32 __cdecl Int_Add(s32 a, s32 b) { return a + b; }

void __cdecl Release_InitUnusedOne() { global<float>(0x007b0468) = 1.0f; }

void __cdecl Release_InitUnusedEpsilon() { global<float>(0x007b046c) = 1.0f / 8388608.0f; }
}
