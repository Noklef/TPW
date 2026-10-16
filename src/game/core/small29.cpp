#include "small29.h"

#include <cstdint>

#include "gameheap.h"
#include "small20.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

void TakeOffParentList(u8 *object) {
    u8 *list = at<u8 *>(at<u8 *>(object, 4), 0x2c);
    u8 *node = at<u8 *>(list, 4);
    while (node && at<u8 *>(node, 0) != object)
        node = at<u8 *>(node, 4);
    if (!node)
        return;
    if (node == at<u8 *>(list, 4))
        at<u8 *>(list, 4) = at<u8 *>(node, 4);
    else
        at<u8 *>(at<u8 *>(node, 8), 4) = at<u8 *>(node, 4);
    if (node == at<u8 *>(list, 8))
        at<u8 *>(list, 8) = at<u8 *>(node, 8);
    else
        at<u8 *>(at<u8 *>(node, 4), 8) = at<u8 *>(node, 8);
    at<u32>(list, 0xc) -= 1;
    GameHeap_Free(node);
}

void ResetAndRelease(u8 *object, u32 a, u32 b, u32 elementSize, u32 destructorAt) {
    u8 *shared = at<u8 *>(object, 0x30);
    at<u32>(object, 4) = 0xffffffffu;
    at<u32>(object, 0x10) = 0;
    at<u32>(object, 0x14) = 0;
    at<u32>(object, 8) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x18) = 0;
    at<u32>(object, 0x1c) = a;
    at<u32>(object, 0x20) = b;
    if (!shared)
        return;
    const u32 uses = at<u32>(shared, 0xc) - 1;
    at<u32>(shared, 0xc) = uses;
    if (uses == 0) {
        if (u8 *array = at<u8 *>(shared, 4)) {
            CRT_VectorDtorIterator(array, elementSize, at<s32>(array, (u32)-4), global<void(__fastcall *)(void *)>(destructorAt));
            GameHeap_Free(array - 4);
        }
        at<u32>(shared, 4) = 0;
        GameHeap_Free(shared);
    }
    at<u32>(object, 0x30) = 0;
}

}  // namespace

extern "C" {

void __fastcall Class702938_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702938;
    TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x44));
    WString_Dtor((WString *)(object + 0x20));
    WString_Dtor((WString *)(object + 0xc));
}

void __fastcall Class702940_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702940;
    TakeOffParentList(object);
    String_Dtor((String *)(object + 0x34));
    String_Dtor((String *)(object + 0x20));
    String_Dtor((String *)(object + 0xc));
}

void __fastcall Class702948_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702948;
    TakeOffParentList(object);
    WString_Dtor((WString *)(object + 0x4c));
    WString_Dtor((WString *)(object + 0x20));
    WString_Dtor((WString *)(object + 0xc));
}

void __fastcall Obj_ResetAndRelease26(u8 *object, void *, u32 a, u32 b) { ResetAndRelease(object, a, b, 0x26, 0x006fd508); }

void __fastcall Obj_ResetAndRelease34(u8 *object, void *, u32 a, u32 b) { ResetAndRelease(object, a, b, 0x34, 0x006fd510); }

}  // extern "C"
