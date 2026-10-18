#include "msgqueue.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

template <typename T> T &at(void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
}

extern "C" {

void __fastcall MsgQueue_Free(u8 *queue) {
    if (!at<u8 *>(queue, 0)) return;
    GameHeap_Free(at<u8 *>(queue, 0));
    at<u8 *>(queue, 0) = nullptr;
}

u32 __fastcall MsgQueue_Push(u8 *queue, void *, u32 window, u32 message, u32 a, u32 b) {
    // (the entry's address is worked out again for each store, as the original does)
    const auto field = [queue](u32 k) -> u32 & { return at<u32>(at<u8 *>(queue, 0) + (at<u32>(queue, 0xc) << 4), 4 * k); };
    if (field(0) || !window) return 0;
    field(0) = window;
    field(1) = message;
    field(2) = a;
    field(3) = b;
    at<u32>(queue, 0x10) += 1;
    at<s32>(queue, 0xc) -= 1;
    if (at<s32>(queue, 0xc) < 0) at<s32>(queue, 0xc) = at<s32>(queue, 4) - 1;
    return 1;
}
}
