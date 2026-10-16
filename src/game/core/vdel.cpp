#include "vdel.h"

#include <cstdint>

#include "gameheap.h"
#include "small14.h"
#include "vdtor.h"
#include "vobj2.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *Finish(u8 *object, u32 flags) {
    if (flags & 1)
        GameHeap_Free(object);
    return object;
}

}  // namespace

extern "C" {

u8 *__fastcall Class702880_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702880_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702890_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702890_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7028d0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7028d0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7028f8_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7028f8_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702920_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702920_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702da0_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702da0_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Class7099a4_ScalarDtor(u8 *object, void *, u32 flags) {
    Class7099a4_Dtor(object);
    return Finish(object, flags);
}

u8 *__fastcall Counted2_ScalarDtor(u8 *object, void *, u32 flags) {
    Counted2_Clear(object);
    return Finish(object, flags);
}

u8 *__fastcall Class702a90_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702a90;
    at<u32>(object, 0x20) = 0;
    return object;
}

u32 __fastcall Class702a90_Release(u8 *object) {
    at<u32>(object, 0) = 0x00702a90;
    return Class7029d0_Release(object);
}

u8 *__fastcall Class702a90_ScalarDtor(u8 *object, void *, u32 flags) {
    Class702a90_Release(object);
    return Finish(object, flags);
}

}  // extern "C"
