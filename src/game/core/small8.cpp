#include "small8.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *VBase_Deleter(u8 *object, u32 flags, u32 vtable) {
    u8 *base = object - 8;
    at<u32>(base, at<u32>(at<u8 *>(base, 0), 4)) = vtable;
    const u32 offset = at<u32>(at<u8 *>(base, 0), 4);
    at<u32>(base, offset - 4) = offset - 8;
    at<u32>(base, 8) = 0x00701940;
    if (flags & 1)
        GameHeap_Free(base);
    return base;
}

}  // namespace

extern "C" {

void __fastcall CountedBuf_ReleaseAndClear(u8 *object) {
    u8 *buffer = at<u8 *>(object, 4);
    if (buffer) {
        const u8 uses = buffer[-1];
        if (uses == 0 || uses == 0xff)
            GameHeap_Free(buffer - 2);
        else
            buffer[-1] = (u8)(uses - 1);
    }
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

u8 __stdcall Bytes785058_Get(u32 index) {
    switch (index) {
    case 0: return *reinterpret_cast<u8 *>(0x00785058);
    case 1: return *reinterpret_cast<u8 *>(0x0078505c);
    case 2: return *reinterpret_cast<u8 *>(0x00785060);
    default: return 0;
    }
}

void __fastcall Visitor_Set1f8One(u8 *object) { at<u32>(object, 0x1f8) = 1; }

u8 *__fastcall Ring_FromEnd(u8 *object, void *, u32 index) {
    const u32 count = at<u32>(object, 0x88);
    if (count >= index)
        return object + (count - index) * 4 + 4;
    return object + (count - index) * 4 + 0x84;
}

u8 *__fastcall VBase_Deleter_7018d0(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x007018d0); }

u8 *__fastcall VBase_Deleter_701920(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x00701920); }

u8 *__fastcall VBase_Deleter_7018f8(u8 *object, void *, u32 flags) { return VBase_Deleter(object, flags, 0x007018f8); }

u32 __stdcall Ring_Splice(u8 *position, u32, u8 *first, u8 *last) {
    at<u8 *>(at<u8 *>(last, 4), 0) = position;
    at<u8 *>(at<u8 *>(first, 4), 0) = last;
    at<u8 *>(at<u8 *>(position, 4), 0) = first;
    u8 *lastPrev = at<u8 *>(last, 4);
    u8 *positionPrev = at<u8 *>(position, 4);
    at<u8 *>(position, 4) = lastPrev;
    at<u8 *>(last, 4) = at<u8 *>(first, 4);
    at<u8 *>(first, 4) = positionPrev;
    return (u32)(uintptr_t)first;
}

}  // extern "C"
