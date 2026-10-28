#include "scenequeue.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/stdlist.h"
#include "../world/advisor.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(static_cast<uintptr_t>(address)); }

}  // namespace

extern "C" {

void __fastcall Class702da0_Clear(u8 *object) {
    if (global<u32>(0x00f7966c)) Advisor_StopSpeech();   // (the advisor's speaking flag)
    while (at<u32>(object, 0x14)) {
        u8 *const node = at<u8 *>(object, 0xc);
        void *item = nullptr;
        if (node) {
            u8 *const next = at<u8 *>(node, 4);
            at<u8 *>(object, 0xc) = next;
            if (next)
                at<u32>(next, 8) = 0;
            else
                at<u32>(object, 0x10) = 0;
            item = at<void *>(node, 0);
            --at<u32>(object, 0x14);
            GameHeap_Free(node);
        }
        GameHeap_Free(item);
    }
}

void __fastcall Class702da0_Push(u8 *object, void *, u32 id, u32 clear) {
    if (clear) Class702da0_Clear(object);   // (inlined)
    u32 *const item = static_cast<u32 *>(GameHeap_Alloc(4));
    *item = id;
    List_PushHead(object + 8, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(item)));
}
}
