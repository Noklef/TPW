#include "small35.h"

#include <cstdint>

#include "gameheap.h"
#include "string.h"
#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

void __fastcall Class6fd9d8_Dtor(u8 *object) {
    at<u32>(object, 0) = 0x006fd9d8;
    if (u8 *member = at<u8 *>(object, 0x3c))
        vcall<u32>(member, 0xc, 1u);
    u8 *list = at<u8 *>(object, 0x2c);
    at<u32>(object, 0x3c) = 0;
    if (list) {
        while (at<u32>(list, 0xc) > 0) {
            u8 *node = at<u8 *>(list, 4);
            u8 *held = nullptr;
            if (node) {
                u8 *next = at<u8 *>(node, 4);
                at<u8 *>(list, 4) = next;
                if (next)
                    at<u32>(next, 8) = 0;
                else
                    at<u32>(list, 8) = 0;
                held = at<u8 *>(node, 0);
                at<u32>(list, 0xc) = at<u32>(list, 0xc) - 1;
                GameHeap_Free(node);
            }
            if (held)
                vcall<u32>(held, 0x2c, 1u);
            list = at<u8 *>(object, 0x2c);
        }
        if (u8 *l = at<u8 *>(object, 0x2c))
            vcall<u32>(l, 0, 1u);
        at<u32>(object, 0x2c) = 0;
    }
    String_Dtor(reinterpret_cast<String *>(object + 0x18));
    String_Dtor(reinterpret_cast<String *>(object + 4));
}

}  // extern "C"
