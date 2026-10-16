// The park's thing table: reimplementation of 0x515f30-0x516310, 0x50b350, 0x51ad40 and
// 0x51ad90.
// The original's Debug_Printf asserts (an empty function in the release build) are left
// out.
#include "things.h"

#include "../core/gameheap.h"

namespace {

constexpr u32 kSlots = 0x2800;
ThingSlot *slots() { return reinterpret_cast<ThingSlot *>(0x007cfb90); }
ThingSlot *&used() { return *reinterpret_cast<ThingSlot **>(0x007cf56c); }
ThingSlot *&freeSlots() { return *reinterpret_cast<ThingSlot **>(0x00801bbc); }

HandleQueue *deleteQueue() { return reinterpret_cast<HandleQueue *>(0x00801f38); }

void __cdecl game_destroy(void *thing) { reinterpret_cast<void(__fastcall *)(void *)>(0x0050b780)(thing); }
void __cdecl game_free(void *block) { GameHeap_Free(block); }  // the game's operator delete / new (0x67acb0 / 0x67acc0), through core/gameheap
void *__cdecl game_alloc(u32 size) { return GameHeap_Alloc(size); }
const ThingTableEnv kGameEnv = {
    &game_free,
    &game_alloc,
    &game_destroy,
};
const ThingTableEnv *g_env = &kGameEnv;

u16 *handle_of(ThingSlot *slot, u16 *out) {
    *out = slot ? (u16)slot->id : 0;
    return out;
}

}  // namespace

void Things_SetEnv(const ThingTableEnv *e) { g_env = e ? e : &kGameEnv; }

extern "C" void __cdecl Things_InitTable() {
    ThingSlot *const s = slots();
    s[0].nextFree = nullptr;
    s[0].next = nullptr;
    s[0].prev = nullptr;
    s[0].thing = nullptr;
    s[0].id = 0;
    for (u32 i = 1; i < kSlots - 1; ++i) {
        s[i].nextFree = &s[i + 1];
        s[i].next = nullptr;
        s[i].prev = nullptr;
        s[i].thing = nullptr;
        s[i].id = i;
    }
    s[kSlots - 1].nextFree = nullptr;
    s[kSlots - 1].next = nullptr;
    s[kSlots - 1].prev = nullptr;
    s[kSlots - 1].thing = nullptr;
    s[kSlots - 1].id = kSlots - 1;
    used() = nullptr;
    freeSlots() = &s[1];
}

extern "C" u16 *__fastcall World_FirstThing(void *, void *, u16 *out) { return handle_of(used(), out); }

extern "C" u16 *__fastcall World_NextThing(void *, void *, u16 *out, const u8 *thing) {
    u16 handle;
    Thing_GetHandle(thing, nullptr, &handle);
    return handle_of(slots()[handle].next, out);
}

extern "C" u16 *__fastcall World_PrevThing(void *, void *, u16 *out, const u8 *thing) {
    u16 handle;
    Thing_GetHandle(thing, nullptr, &handle);
    return handle_of(slots()[handle].prev, out);
}

extern "C" u16 *__fastcall World_AddThing(void *, void *, u16 *out, void *thing) {
    ThingSlot *const slot = freeSlots();
    if (!slot) {
        *out = 0;
        return out;
    }
    freeSlots() = slot->nextFree;
    slot->nextFree = nullptr;
    slot->next = used();
    if (used()) used()->prev = slot;
    used() = slot;
    slot->thing = thing;
    *out = (u16)used()->id;
    return out;
}

extern "C" void __fastcall ThingSlot_Release(ThingSlot *slot) {
    if (slot->next) slot->next->prev = slot->prev;
    if (slot->prev) slot->prev->next = slot->next;
    else used() = slot->next;
    slot->next = nullptr;
    slot->prev = nullptr;
    slot->nextFree = freeSlots();
    freeSlots() = slot;
    g_env->free(slot->thing);
    slot->thing = nullptr;
}

extern "C" u16 *__fastcall Thing_GetHandle(const u8 *thing, void *, u16 *out) {
    *out = *reinterpret_cast<const u16 *>(thing);
    return out;
}

extern "C" void __fastcall HandleQueue_Push(HandleQueue *q, void *, u32 handle) {
    HandleNode *node = static_cast<HandleNode *>(g_env->alloc(sizeof(HandleNode)));
    if (node) {
        node->next = nullptr;
        node->handle = (u16)handle;
        node->prev = nullptr;
    }
    node->next = q->head;
    if (!q->head) q->tail = node;
    else q->head->prev = node;
    q->head = node;
    ++q->count;
}

extern "C" u16 *__fastcall HandleQueue_Pop(HandleQueue *q, void *, u16 *out) {
    HandleNode *const node = q->head;
    if (!node) return out;
    q->head = node->next;
    if (!q->head) q->tail = nullptr;
    else q->head->prev = nullptr;
    --q->count;
    const u16 handle = node->handle;
    g_env->free(node);
    *out = handle;
    return out;
}

extern "C" void __fastcall World_QueueDelete(void *, void *, u32 handle) {
    HandleQueue_Push(deleteQueue(), nullptr, handle);
}

extern "C" void __cdecl Things_ReleaseDeleted() {
    u16 handle = 0;  // (the original keeps the caller's SI when the queue is inconsistent: a count with no head)
    while (deleteQueue()->count) {
        HandleQueue_Pop(deleteQueue(), nullptr, &handle);
        ThingSlot_Release(&slots()[handle]);
    }
}

extern "C" void __fastcall World_DeleteAllThings(u8 *world) {
    *reinterpret_cast<u32 *>(world + 0x1da738) = 2;
    u16 handle;
    handle_of(used(), &handle);
    while (handle) {
        u8 *const thing = static_cast<u8 *>(slots()[handle].thing);
        if (!thing[3]) g_env->destroy(thing);
        u16 index;
        Thing_GetHandle(static_cast<const u8 *>(slots()[handle].thing), nullptr, &index);
        handle_of(slots()[index].next, &handle);
    }
    while (deleteQueue()->count) {
        HandleQueue_Pop(deleteQueue(), nullptr, &handle);  // inlined in the original
        ThingSlot_Release(&slots()[handle]);
    }
    *reinterpret_cast<u32 *>(world + 0x1da738) = 1;
}
