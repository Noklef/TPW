#include "small7.h"

#include <cstdint>

#include "gameheap.h"
#include "misc.h"
#include "small5.h"
#include "tiny_a.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u8 *__fastcall Deleter_Minus60_42a090(u8 *object, void *, u32 flags) {
    at<u32>(object, 0) = 0x006fdce8;
    if (flags & 1)
        GameHeap_Free(object - 0x60);
    return object - 0x60;
}

u8 *__fastcall Deleter_Minus8_56a660(u8 *object, void *, u32 flags) {
    Vtbl701260_Set_565230(object);
    if (flags & 1)
        GameHeap_Free(object - 8);
    return object - 8;
}

void __fastcall BufferC_FreeAndClear(u8 *object) {
    if (at<void *>(object, 0xc))
        GameHeap_Free(at<void *>(object, 0xc));
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
}

u8 *__fastcall Class702958_Ctor(u8 *object) {
    Class70295c_Ctor(object);
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = 0;
    at<u32>(object, 0x420) = 0;
    at<u32>(object, 0) = 0x00702958;
    for (u32 i = 0; i < 0x100; ++i)
        at<u32>(object, 0x20 + 4 * i) = 0;
    return object;
}

u8 *__fastcall ListHead_CtorFrom(u8 *object, void *, const u8 *source) {
    at<u8>(object, 0) = at<u8>(source, 0);
    u8 *head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(object, 4) = head;
    at<u32>(object, 8) = 0;
    return object;
}

u8 *__stdcall Node0c_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0xc));
    at<u32>(node, 0) = a ? a : (u32)(uintptr_t)node;
    at<u32>(node, 4) = b ? b : (u32)(uintptr_t)node;
    return node;
}

void __fastcall Array3Ds_Free4(u8 *object) {
    Array3D_Free(object + 0x2c);
    Array3D_Free(object + 0x3c);
    Array3D_Free(object + 0x4c);
    Array3D_Free(object + 0x5c);
}

void __fastcall Obj_Set118(u8 *object, void *, u32 value) { at<u32>(object, 0x118) = value; }

}  // extern "C"
