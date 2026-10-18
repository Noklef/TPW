#include "msgqueue.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "window.h"

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

u32 __cdecl WordPairs_Contains(const s16 *pairs, u32 a, u32 b) {
    for (; pairs[1] != -1; pairs += 2)
        if (pairs[0] == (s16)a && pairs[1] == (s16)b) return 1;
    return 0;
}

u32 __fastcall MsgQueue_DispatchOne(u8 *queue) {
    if (!at<u8 *>(queue, 0)) return 0;
    u32 *const entry = reinterpret_cast<u32 *>(at<u8 *>(queue, 0) + (at<u32>(queue, 8) << 4));
    if (!entry[0]) return 0;
    if (entry[1] != 0x80808080 && entry[2] != 0x80808080 && entry[3] != 0x80808080) {
        const auto window = [](u32 address) { return *reinterpret_cast<u8 **>((uintptr_t)address); };
        const auto takes = [&](u32 address) { return window(address) && Fn_65d57c(window(address)) & 1; };
        if (entry[1] == 0x1000b || entry[1] == 0x1000a || entry[1] == 0x1000c) {
            u8 *target = nullptr;
            if (window(0x00faa578) && takes(0x00faa57c) &&
                WordPairs_Contains(reinterpret_cast<const s16 *>(window(0x00faa578)), (u16)entry[2], (entry[3] >> 16) & 0xff))
                target = window(0x00faa57c);
            else if (takes(0x00faa5ec))
                target = window(0x00faa5ec);
            else if (takes(0x00faa5dc))
                target = window(0x00faa5dc);
            if (target) Window_CallHandler(target, nullptr, entry[1], entry[2], entry[3]);
        } else {
            Window_CallHandler(reinterpret_cast<u8 *>(entry[0]), nullptr, entry[1], entry[2], entry[3]);
        }
    }
    entry[0] = 0;
    at<u32>(queue, 0x10) -= 1;
    at<s32>(queue, 8) -= 1;
    if (at<s32>(queue, 8) < 0) at<s32>(queue, 8) = at<s32>(queue, 4) - 1;
    return 1;
}
}
