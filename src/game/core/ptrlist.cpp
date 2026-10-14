// The game's pointer list: see ptrlist.h.
#include "ptrlist.h"

#include <cstdint>

namespace {

const PtrListEnv kGameEnv = {
    reinterpret_cast<void *(__cdecl *)(u32)>(static_cast<std::uintptr_t>(0x0067acc0)),
};
const PtrListEnv *g_env = &kGameEnv;

}  // namespace

void PtrList_SetEnv(const PtrListEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" {

void __fastcall PtrList_Append(PtrList *l, void *, void *item) {
    PtrListNode *const n = static_cast<PtrListNode *>(g_env->alloc(sizeof(PtrListNode)));
    if (n) {
        n->next = nullptr;
        n->item = item;
        n->prev = nullptr;
    }
    n->prev = l->tail;
    if (!l->tail) {
        l->head = n;
        l->tail = n;
    } else {
        l->tail->next = n;
        l->tail = n;
    }
    ++l->count;
}

}  // extern "C"
