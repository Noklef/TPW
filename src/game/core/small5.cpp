#include "small5.h"

#include <cstdint>

#include "gameheap.h"
#include "misc.h"
#include "tiny_a.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __cdecl Debug_Nop_46cfb0() {}

void __fastcall Buffer_FreeIfOwned(u8 *object) {
    if (at<u8>(object, 0) != 0)
        GameHeap_Free(at<void *>(object, 4));
}

u32 __fastcall Buffer_FreeAndClear(u8 *object) {
    GameHeap_Free(at<void *>(object, 4));
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    return 0;
}

u8 *__stdcall Node28_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0x28));
    at<u32>(node, 4) = a;
    at<u32>(node, 0x24) = b;
    return node;
}

u8 *__stdcall Node18_New(u32 a, u32 b) {
    u8 *node = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u32>(node, 0) = a ? a : (u32)(uintptr_t)node;
    at<u32>(node, 4) = b ? b : (u32)(uintptr_t)node;
    return node;
}

u8 *__fastcall ListHead_Ctor(u8 *object) {
    u8 top = (u8)((u32)(uintptr_t)object >> 24);  // sic
    at<u8>(object, 0) = top;
    u8 *head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(object, 4) = head;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = at<u32>(head, 0);
    return object;
}

u8 *__fastcall Array3D_Init(u8 *object, void *, u32 a, u32 b, u32 c) {
    at<u32>(object, 8) = c;
    at<u32>(object, 0) = a;
    at<u32>(object, 4) = b;
    at<void *>(object, 0xc) = GameHeap_Alloc(c * b * a);
    return object;
}

void __fastcall Array3D_Free(u8 *object) {
    void *block = at<void *>(object, 0xc);
    at<u32>(object, 0) = 0;
    at<u32>(object, 4) = 0;
    at<u32>(object, 8) = 0;
    if (block) {
        GameHeap_Free(block);
        at<u32>(object, 0xc) = 0;
    }
}

u32 __cdecl Obj_RemoveRectAndClear14(u8 *object, u32 from, u32 to) {
    Obj_RemoveFromRect(at<u8 *>(object, 0x24), from, to);
    at<u32>(object, 0) &= ~0x4000u;
    return 1;
}

u8 *__fastcall Deleter_Minus2c_569c80(u8 *object, void *, u32 flags) {
    Vtbl701260_Set_565230(object);
    if (flags & 1)
        GameHeap_Free(object - 0x2c);
    return object - 0x2c;
}

}  // extern "C"
