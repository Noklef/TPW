#include "small21.h"

#include <cstdint>

#include "../files/file.h"
#include "gameheap.h"
#include "keymap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Ptr_FreeIfSet(u8 **ref) {
    if (*ref)
        GameHeap_Free(*ref);
}

u32 __cdecl Dwords_Zero(u32 *to, u32 count) {
    for (u32 i = 0; i < count; ++i)
        to[i] = 0;
    return 0;
}

u32 __fastcall ReturnOne() { return 1; }

void __cdecl KeyMaps_EnableAllAndClearFlag() {
    KeyMaps_EnableAll();
    global<u8>(0x007c24d0) = 0;
}

void __cdecl KeyMaps_ResetAllAndSetFlag() {
    KeyMaps_ResetAll();
    global<u8>(0x007c24d0) = 1;
}

u32 __cdecl Sound_Alloc(u32 size) {
    if (!size)
        return 0;
    global<u32>(0x00802bd8) += 1;
    return (u32)(uintptr_t)Mem_GlobalAlloc(size);
}

u32 __cdecl Sound_Free(void *block) {
    if (!block)
        return 0;
    global<u32>(0x00802bd8) -= 1;
    return (u32)(uintptr_t)Mem_GlobalFree(block);
}

u32 __cdecl Mem_AllocChecked(u32 size) { return size ? (u32)(uintptr_t)Mem_GlobalAlloc(size) : 0; }

u32 __cdecl Mem_AllocLogged(u32 size) { return size ? (u32)(uintptr_t)Mem_GlobalAlloc(size) : 0; }

u32 __cdecl Mem_FreeChecked(void *block) { return block ? (u32)(uintptr_t)Mem_GlobalFree(block) : 0; }

}  // extern "C"
