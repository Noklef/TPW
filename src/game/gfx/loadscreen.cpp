#include "loadscreen.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void close(u8 *screen) {
    if (global<u32>(0x008c478c)) {
        u32(__stdcall *const timeGetTime)() = *reinterpret_cast<u32(__stdcall *const *)()>((uintptr_t)0x006fd3ec);
        if (timeGetTime() - global<u32>(0x008c4780) < 3000) {
            while (timeGetTime() - global<u32>(0x008c4780) < 3000) {
            }
        }
    }
    for (u32 offset = 0x10; offset <= 0x14; offset += 4) {
        u32 *const block = *reinterpret_cast<u32 **>(screen + offset);
        if (!block)
            continue;
        if (*block) {
            TrackedGlobal_Free(*block);
            *block = 0;
        }
        GameHeap_Free(block);
        *reinterpret_cast<u32 **>(screen + offset) = nullptr;
    }
    global<u32>(0x008bcbc8) &= ~0x40000000u;
}

}  // namespace

extern "C" {

u32 __fastcall LoadScreen_Close(u8 *screen) {
    close(screen);
    return 1;
}

void __cdecl LoadScreen_StaticDtor() { close(reinterpret_cast<u8 *>((uintptr_t)0x008c4748)); }
}
