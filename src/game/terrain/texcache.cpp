#include "texcache.h"

#include <cstdint>
#include <cstring>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall TerrainTexCache_Ctor(u8 *cache) {
    at<u32>(cache, 4) = 0x40;
    at<u32>(cache, 0x10008) = 0;
    at<u32>(cache, 0x10010) = 0;
    std::memset(cache + 8, 0, 0x10000);
    at<u32>(cache, 0) = 0;
    return cache;
}
}
