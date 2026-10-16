#include "vdtor.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

}  // namespace

extern "C" {

u8 *__fastcall Class702d48_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x00702d48;
    at<u32>(object, 4) = 0;
    vcall<u32>(global<u8 *>(0x00f85614), 0x18, object);
    return object;
}

u32 __fastcall Class702d48_Release(u8 *object) {
    at<u32>(object, 0) = 0x00702d48;
    return vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
}

u8 *__fastcall Class702d48_DeletingDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x00702d48;
    vcall<u32>(global<u8 *>(0x00f85614), 0x1c, object);
    return Finish(object, flags);
}

u8 *__fastcall Class7029d0_Ctor(u8 *object) {
    at<u32>(object, 0) = 0x007029d0;
    vcall<u32>(global<u8 *>(0x00f82884), 0, object);
    return object;
}

u32 __fastcall Class7029d0_Release(u8 *object) {
    at<u32>(object, 0) = 0x007029d0;
    return vcall<u32>(global<u8 *>(0x00f82884), 4, object);
}

u8 *__fastcall Class7029d0_DeletingDtor(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x007029d0;
    vcall<u32>(global<u8 *>(0x00f82884), 4, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702ae8_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 4);
    at<u32>(object, 0) = 0x00702ae8;
    vcall<u32>(member, 8, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702bb4_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 8);
    at<u32>(object, 0) = 0x00702bb4;
    vcall<u32>(member, 0x3c, object);
    return Finish(object, flags);
}

u8 *__fastcall Class702bb8_DeletingDtor(u8 *object, void *, u32 flags) {
    u8 *member = at<u8 *>(object, 8);
    at<u32>(object, 0) = 0x00702bb8;
    vcall<u32>(member, 0x38, object);
    return Finish(object, flags);
}

u32 __fastcall Obj_CallSlot20ThenSub1c(u8 *object) {
    vcall<u32>(object, 0x20);
    return vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x1c);
}

u32 __fastcall Obj_CallSlot20ThenSub1cAnd48(u8 *object) {
    vcall<u32>(object, 0x20);
    vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x1c);
    return vcall<u32>(at<u8 *>(global<u8 *>(0x00f85614), 4), 0x48);
}

u32 __fastcall VBase_CallSlot8(u8 *object, void *, u32 a) {
    u8 *base = object - 0x18;
    u8 *target = base + at<u32>(at<u8 *>(base, 0), 4);
    return vcall<u32>(target, 8, a);
}

}  // extern "C"
