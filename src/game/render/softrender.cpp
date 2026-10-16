#include "softrender.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __fastcall SoftRenderer_ClearBuffer(u8 *renderer) {
    std::memset(at<u8 *>(renderer, 8), 0, at<u32>(renderer, 0xc));
    return 1;
}
}
