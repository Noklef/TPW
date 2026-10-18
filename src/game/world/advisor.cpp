#include "advisor.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Table768fb8_Lookup(u32 id) {
    for (u32 entry = 0x00768fb8; global<u32>(entry) != 0x270f; entry += 0x20) {
        if (global<u32>(entry) == id)
            return global<u32>(entry + 0x1c);
    }
    return 0x17f;
}
}
