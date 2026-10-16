#include "small32.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"
#include "vdtor.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u8 *NewList(u32 vtable) {
    u8 *list = static_cast<u8 *>(GameHeap_Alloc(0x10));
    if (!list)
        return nullptr;
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
    at<u32>(list, 0xc) = 0;
    at<u32>(list, 0) = vtable;
    return list;
}

void DrainList(u8 *object, u32 field) {
    u8 *list = at<u8 *>(object, field);
    while (at<u32>(list, 0xc) != 0) {
        u8 *node = at<u8 *>(list, 4);
        u8 *held = nullptr;
        if (node) {
            u8 *next = at<u8 *>(node, 4);
            at<u8 *>(list, 4) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(list, 8) = 0;
            const u32 left = at<u32>(list, 0xc) - 1;
            held = at<u8 *>(node, 0);
            at<u32>(list, 0xc) = left;
            GameHeap_Free(node);
        }
        if (held)
            vcall<u32>(held, 0, 1u);
        list = at<u8 *>(object, field);
    }
    if (at<u8 *>(object, field))
        vcall<u32>(at<u8 *>(object, field), 0, 1u);
    at<u32>(object, field) = 0;
}

}  // namespace

extern "C" {

u8 *__fastcall Class702b08_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702b08;
    at<u8 *>(object, 4) = NewList(0x00702b00);
    return object;
}

u8 *__fastcall Class702d78_Ctor(u8 *object) {
    Class7029d0_Ctor(object);
    at<u32>(object, 0) = 0x00702d78;
    at<u8 *>(object, 0x18) = NewList(0x00702d70);
    return object;
}

void __fastcall Class702b08_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702b08;
    DrainList(object, 4);
    Class7029d0_Release(object);
}

void __fastcall Class702cb0_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702cb0;
    DrainList(object, 8);
    Class702d48_Release(object);
}

void __fastcall Class702d78_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x00702d78;
    DrainList(object, 0x18);
    Class7029d0_Release(object);
}

}  // extern "C"
