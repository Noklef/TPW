#include "releaselist.h"

#include <cstdint>

#include "gameheap.h"

namespace {

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }
using OnLock = u32(__stdcall *)(void *);

const ReleaseListEnv kGameEnv = {reinterpret_cast<s32(__cdecl *)(void(__cdecl *)())>((uintptr_t)0x0067a7d0)};
const ReleaseListEnv *g_env = &kGameEnv;

ReleaseList *instance() { return reinterpret_cast<ReleaseList *>((uintptr_t)0x00fb2730); }

}  // namespace

void ReleaseList_SetEnv(const ReleaseListEnv *env) { g_env = env ? env : &kGameEnv; }

extern "C" {

void __fastcall ReleaseList_Dtor(ReleaseList *list) {
    while (list->count) {   // (a null head with a count left spins, as the original)
        ReleaseNode *const node = list->head;
        if (!node) continue;
        ReleaseNode *const next = node->next;
        list->head = next;
        if (!next) list->tail = nullptr;
        else next->prev = nullptr;
        --list->count;
        GameHeap_Free(node);
    }
    Import<OnLock>(0x006fd0d0)(list->lock);   // DeleteCriticalSection
    ReleaseList_BaseDtor(list);
}

void __fastcall ReleaseList_BaseDtor(ReleaseList *list) {
    ReleaseNode *node = list->head;
    list->vtable = 0x00711c1c;
    list->tail = nullptr;
    list->head = nullptr;
    while (node) {
        ReleaseNode *const next = node->next;
        GameHeap_Free(node);
        node = next;
    }
    list->count = 0;
}

ReleaseList *__cdecl ReleaseList_Get() {
    u8 &flag = *reinterpret_cast<u8 *>((uintptr_t)0x00fb2728);
    if (!(flag & 1)) {
        flag |= 1;
        ReleaseList_Ctor(instance());
        g_env->atExit(reinterpret_cast<void(__cdecl *)()>((uintptr_t)0x006d74a0));
    }
    return instance();
}

void __cdecl ReleaseList_AtExit() { ReleaseList_Dtor(instance()); }

ReleaseList *__fastcall ReleaseList_Ctor(ReleaseList *list) {
    list->head = nullptr;
    list->tail = nullptr;
    list->count = 0;
    list->vtable = 0x00711c20;
    Import<OnLock>(0x006fd144)(list->lock);   // InitializeCriticalSection
    return list;
}

ReleaseList *__fastcall ReleaseList_BaseDeletingDtor(ReleaseList *list, void *, u32 flags) {
    ReleaseList_BaseDtor(list);
    if (flags & 1) GameHeap_Free(list);
    return list;
}

u32 __fastcall ReleaseList_Push(ReleaseList *list, void *, u32 value) {
    Import<OnLock>(0x006fd140)(list->lock);   // EnterCriticalSection
    ReleaseNode *const node = static_cast<ReleaseNode *>(GameHeap_Alloc(sizeof(ReleaseNode)));
    if (node) {
        node->value = value;
        node->next = nullptr;
        node->prev = nullptr;
    }
    volatile ReleaseNode *const to = node;   // (null when the allocation failed: written through all the same)
    to->next = list->head;
    if (list->head) list->head->prev = node;
    else list->tail = node;
    list->head = node;
    ++list->count;
    return Import<OnLock>(0x006fd130)(list->lock);   // LeaveCriticalSection
}

ReleaseList *__fastcall ReleaseListBase_DeletingDtor(ReleaseList *list, void *, u32 flags) {
    ReleaseList_BaseDtor(list);   // (inlined in the original)
    if (flags & 1) GameHeap_Free(list);
    return list;
}

}  // extern "C"
