#include "scenequeue.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/stdlist.h"
#include "../files/file.h"
#include "../world/advisor.h"
#include "../world/advisorspeech.h"

namespace {

template <typename T> T &at(u8 *base, u32 offset) { return *reinterpret_cast<T *>(base + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>(static_cast<uintptr_t>(address)); }

const SceneQueueEnv kGameEnv = {&Advisor_Say, &Time_GetMs, &List_PushHead};
const SceneQueueEnv *g_env = &kGameEnv;

}  // namespace

void SceneQueue_SetEnv(const SceneQueueEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall Class702da0_Step(u8 *object) {
    if (gAdvisor_State_f79660.speaking) {
        at<u32>(object, 0x28) = 1;
        return;
    }
    if (at<u32>(object, 0x14)) {
        u8 *const node = at<u8 *>(object, 0x10);
        u32 *item = nullptr;
        if (node) {
            u8 *const previous = at<u8 *>(node, 8);
            at<u8 *>(object, 0x10) = previous;
            if (previous)
                at<u32>(previous, 4) = 0;
            else
                at<u32>(object, 0xc) = 0;
            item = at<u32 *>(node, 0);
            --at<u32>(object, 0x14);
            GameHeap_Free(node);
        }
        g_env->say(static_cast<s32>(*item));
        GameHeap_Free(item);
        at<u32>(object, 0x28) = 1;
        return;
    }
    if (at<u32>(object, 0x18)) {
        const u32 now = g_env->now();
        if (at<u32>(object, 0x28)) {
            at<u32>(object, 0x20) = now;
            at<u32>(object, 0x28) = 0;
            return;
        }
        if (now > at<u32>(object, 0x1c) + at<u32>(object, 0x20)) {
            const u32 id = at<u32>(object, 0x24);
            at<u32>(object, 0x20) = now;
            u8 *const queue = at<u8 *>(global<u8 *>(0x00f85614), 0x14);
            u32 *const pushed = static_cast<u32 *>(GameHeap_Alloc(4));
            *pushed = id;
            g_env->pushHead(queue + 8, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(pushed)));
        }
    }
    at<u32>(object, 0x28) = 0;
}

void __fastcall Class702da0_Clear(u8 *object) {
    if (gAdvisor_State_f79660.speaking) Advisor_StopSpeech();   // (the advisor's speaking flag)
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

void __fastcall Class702da0_Show(u8 *object, void *, u32 id, u32 time) {
    at<u32>(object, 0x18) = 1;
    at<u32>(object, 0x1c) = time;
    at<u32>(object, 0x24) = id;
}

void __fastcall Class702da0_Push(u8 *object, void *, u32 id, u32 clear) {
    if (clear) Class702da0_Clear(object);   // (inlined)
    u32 *const item = static_cast<u32 *>(GameHeap_Alloc(4));
    *item = id;
    List_PushHead(object + 8, nullptr, static_cast<u32>(reinterpret_cast<uintptr_t>(item)));
}
}
