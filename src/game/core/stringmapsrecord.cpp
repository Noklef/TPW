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

u8 *__fastcall StringMapsRecord_Ctor(u8 *record) {
    // (sic: the original copies its allocator bytes, +0, +0x10, +0x20 / +0x21, +0x30 / +0x31, +0x40 / +0x41 and +0x54, from
    // uninitialised stack; the port writes 0s)
    at<u8>(record, 0) = 0;
    at<u32>(record, 4) = 0;
    at<u32>(record, 8) = 0;
    at<u32>(record, 0xc) = 0;
    at<u8>(record, 0x10) = 0;
    at<u32>(record, 0x14) = 0;
    at<u32>(record, 0x18) = 0;
    at<u32>(record, 0x1c) = 0;
    const auto map = [record](u32 offset, void(__fastcall *ctor)(u8 *)) {
        at<u8>(record, offset) = 0;
        at<u8>(record, offset + 1) = 0;
        at<u8>(record, offset + 8) = 0;
        ctor(record + offset);
    };
    map(0x20, &StdTree_CtorD);
    map(0x30, &StdTree_CtorC);
    map(0x40, &StdTree_CtorE);
    at<u8>(record, 0x50) = 0;
    at<u8>(record, 0x51) = 0;
    at<u8>(record, 0x54) = 0;
    u8 *const head = static_cast<u8 *>(GameHeap_Alloc(0x18));
    at<u8 *>(head, 0) = head;
    at<u8 *>(head, 4) = head;
    at<u8 *>(record, 0x58) = head;
    at<u32>(record, 0x5c) = 0;
    return record;
}

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
