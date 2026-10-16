#include "small43.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Wavelet_ReadBits(u32 count) {
    u32 held = global<u32>(0x00aa67f8);
    if (held < count) {
        const u32 position = global<u32>(0x00be6800);
        const u32 word = at<u32>(reinterpret_cast<const void *>(position), global<u32>(0x00e74a40)) & 0xffffu;
        global<u32>(0x00be6800) = position + 2;
        global<u32>(0x00f74a48) |= word << (held & 31);
        held += 0x10;
    }
    held -= count;
    global<u32>(0x00aa67f8) = held;
    const u32 accumulator = global<u32>(0x00f74a48);
    const u32 result = ((1u << (count & 31)) - 1u) & accumulator;
    global<u32>(0x00f74a48) = accumulator >> (count & 31);
    return result;
}

u8 __stdcall Record_IsActive(u32 id) {
    if (id == 0x160 || id == 0x15f) return 0;
    const u8 *const record = reinterpret_cast<const u8 *>(0x0076e300 + id * 0x38);
    if (at<u32>(record, 4) != id) {
        bool found = false;
        for (u32 scan = 0x0076e304; (s32)scan < 0x00772fcc; scan += 0x38) {
            if (global<u32>(scan) == id) { found = true; break; }
        }
        if (!found) return 0;  // (the original prints that it was not found)
    }
    return at<u32>(record, 8) == 1 ? 1 : 0;
}

}  // extern "C"
