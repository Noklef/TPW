#include "liveobjects.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void **list() { return reinterpret_cast<void **>((uintptr_t)0x0087a6d8); }
s32 &count() { return global<s32>(0x0087aed8); }

}  // namespace

extern "C" {

u32 __cdecl LiveObjects_Add(void *object) {
    s32 n = count();
    if (n < 0x200) {
        list()[n] = object;
        count() = n + 1;
    }
    return 1;
}

u32 __cdecl LiveObjects_Remove(void *object) {
    s32 last = count() - 1;
    s32 i = last;
    while (i >= 0 && list()[i] != object)
        --i;
    if (i < 0)
        return 0;
    count() = last;
    list()[i] = list()[last];
    return 1;
}
}
