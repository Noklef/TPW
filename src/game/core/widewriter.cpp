#include "widewriter.h"

#include <cstdint>

#include "vcall.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

void __fastcall WideWriter_Finish(u8 *writer) {
    u16 *const cursor = at<u16 *>(writer, 4);
    if (cursor) {
        if (!at<u8>(writer, 0x10) && cursor == at<u16 *>(writer, 8)) at<u8>(writer, 0x10) = 1;
        if (!at<u8>(writer, 0x10))
            cursor[0] = 0;
        else
            cursor[-1] = 0;
        at<u16 *>(writer, 4) = nullptr;
    }
    u8 *const holder = at<u8 *>(writer, 0xc);
    if (holder) {
        at<u8 *>(writer, 0xc) = nullptr;
        vcall<void>(holder, 0);
    }
}
