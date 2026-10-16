#include "small27.h"

#include <cstdint>

#include "../files/file.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__stdcall Thing_ByHandle(u32 handle) { return reinterpret_cast<u8 *const *>(0x007cfb90)[(handle & 0xffff) * 5]; }

u16 *__cdecl Cell_MakeHandle(u16 *out, u32 x, u32 z) {
    *out = (u16)((z << 7) + x + 1);
    return out;
}

void __fastcall Obj_FreeFourBuffers(u8 *object) {
    for (u32 offset = 0x4f4, i = 0; i < 4; ++i, offset += 4) {
        u8 *buffer = at<u8 *>(object, offset);
        if (buffer && buffer != reinterpret_cast<u8 *>(0x006fd880)) {
            Mem_GlobalFree(buffer);
            at<u32>(object, offset) = 0;
        }
    }
}

}  // extern "C"
