#include "stringmapsrecord.h"

#include <cstdint>

#include "buffers.h"
#include "gameheap.h"
#include "stdlist.h"
#include "stdtree.h"

namespace {
template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}  // namespace

extern "C" {

void __fastcall StringMapsRecord_Dtor(u8 *record) {
    u8 *const list = record + 0x54;
    u8 *const head = at<u8 *>(list, 4);
    u32 out;
    for (u8 *node = at<u8 *>(head, 0); node != head;) {
        u8 *const erase = node;
        node = at<u8 *>(node, 0);
        List_EraseNodeCounted2(list, nullptr, &out, erase);
    }
    GameHeap_Free(at<void *>(list, 4));
    at<u32>(list, 4) = 0;
    at<u32>(list, 8) = 0;
    StdTree_DtorE(record + 0x40);
    StdTree_DtorC(record + 0x30);
    StdTree_DtorD(record + 0x20);
    CountedBuf_Reset(record + 0x10, nullptr, 1);
    CountedBuf_Reset(record, nullptr, 1);
}
}
