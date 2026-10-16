#include "small16.h"

#include <cstdint>

#include "small9.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out) {
    const u16 handle = at<u16>(object, 0x5e);
    if (handle == 0) {
        *out = 0;
        return out;
    }
    if (global<u32>(global<u32>(0x0080239c) + 0x1da70c) > at<u32>(object, 0x60) + 0x64) {
        const u8 *thing = global<u8 *>(0x007cfb90 + 20 * (u32)handle);
        if (!Person_HasWord(thing, nullptr, at<u16>(object, 0))) {
            at<u16>(object, 0x5e) = 0;
            at<u32>(object, 0x60) = 0;
            *out = 0;
            return out;
        }
    }
    *out = at<u16>(object, 0x5e);
    return out;
}

}  // extern "C"
