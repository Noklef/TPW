#include "small52.h"

#include <cstdint>

#include "gameheap.h"
#include "small5.h"
#include "vwrap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall ArrayHolder_Clear(u8 *object) {
    Array3D_Free(object + 0x2c);
    Array3D_Free(object + 0x3c);
    Array3D_Free(object + 0x4c);
    Array3D_Free(object + 0x5c);
    for (u32 offset = 0x5c; offset >= 0x2c; offset -= 0x10) {
        u8 *const array = object + offset;
        if (at<void *>(array, 0xc)) GameHeap_Free(at<void *>(array, 0xc));
        at<u32>(array, 0) = 0;
        at<u32>(array, 4) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 0xc) = 0;
    }
}

u8 __fastcall Eleven_Read(u8 *object, void *, u8 *stream) {
    for (u32 offset = 0; offset < 0x2c; offset += 4)
        Object_CallSlot8Product((u32)(uintptr_t)(object + offset), 4, 1, stream);
    return 1;
}

}  // extern "C"
