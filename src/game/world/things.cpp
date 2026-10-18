// The park's thing table: reimplementation of 0x515f30-0x516310, 0x50b350, 0x51ad40 and
// 0x51ad90.
// The original's Debug_Printf asserts (an empty function in the release build) are left
// out.
#include "things.h"

#include <cstdint>

#include "../core/accessors.h"
#include "../core/gameheap.h"
#include "../core/methods.h"
#include "../core/savegame.h"
#include "../files/file.h"
#include "../sprite/script.h"
#include "../sprite/sprite.h"
#include "mapcell.h"
#include "nav.h"
#include "objectdefs.h"
#include "random.h"
#include "thingscan.h"

namespace {

constexpr u32 kSlots = 0x2800;
ThingSlot *slots() { return reinterpret_cast<ThingSlot *>(0x007cfb90); }
ThingSlot *&used() { return *reinterpret_cast<ThingSlot **>(0x007cf56c); }
ThingSlot *&freeSlots() { return *reinterpret_cast<ThingSlot **>(0x00801bbc); }

HandleQueue *deleteQueue() { return reinterpret_cast<HandleQueue *>(0x00801f38); }

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// The map cell at a column and row (0x44 bytes each, from [0x8023a0]).
u8 *CellOf(u32 column, u32 row) {
    const u32 id = ((row << 7) + column + 1) & 0xffff;
    return global<u8 *>(0x008023a0) + id * 0x44 - 0x44;
}

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

extern "C" u32 __fastcall World_Attach(u8 *world) {
    *reinterpret_cast<u8 **>(0x0080239c) = world;
    *reinterpret_cast<u8 **>(0x008023a0) = world + 0x2d8;
    Things_InitTable();
    MemRanges_Set(reinterpret_cast<u8 *>(0x007cdb30), nullptr, 0, (u32)(uintptr_t)world, 0x1da748, "World Struture");
    return 1;
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

// The thing's own methods: the constructor 0x50afe0, the move between cells 0x50b6a0 and the small ones that follow.
extern "C" {

u8 *__fastcall Thing_Construct(u8 *thing, void *, u32 cell, u32 kind) {
    at<u16>(thing, 0) = 0;
    at<u8>(thing, 2) = (u8)kind;
    at<u8>(thing, 3) = 0;
    at<u16>(thing, 8) = 0;
    at<u16>(thing, 0xa) = 0;
    u16 handle;
    at<u16>(thing, 0) = *World_AddThing(global<void *>(0x0080239c), nullptr, &handle, thing);
    at<u8>(thing, 4) = 0x80;
    at<u8>(thing, 6) = 0x80;
    const u32 id = cell & 0xffff;
    const u32 index = id - 1;
    at<u8>(thing, 5) = (u8)((u8)cell - 1) & 0x7f;
    at<u8>(thing, 7) = (u8)(index >> 7);
    Obj_ReplaceHandle24(global<u8 *>(0x008023a0) + id * 0x44 - 0x44, nullptr, at<u16>(thing, 0));
    // (the print "Creating thing at ..." is left out)
    return thing;
}

void __fastcall Thing_SetPos(u8 *thing, void *, u32 x, u32 y) {
    const u32 column = (x >> 8) & 0xff, row = (y >> 8) & 0xff;
    if (column == thing[5] && row == thing[7]) {
        at<u16>(thing, 4) = (u16)x;
        at<u16>(thing, 6) = (u16)y;
        return;
    }
    // (the assert on the new cell is left out)
    const u32 handle = at<u16>(thing, 0);
    Things_UnlinkFromChain(CellOf(thing[5], thing[7]), nullptr, handle);
    at<u16>(thing, 4) = (u16)x;
    at<u16>(thing, 6) = (u16)y;
    Obj_ReplaceHandle24(CellOf(thing[5], thing[7]), nullptr, handle);
}

void __fastcall Thing_PlaceAt(u8 *thing, void *, u32 x, u32 y) {
    Nav_SetPosVel(thing + 0xd4, nullptr, (u32)((s32)(s16)x << 8), (u32)((s32)(s16)y << 8), 0, 0);
    Nav_GetPos(thing + 0xd4, nullptr, &at<u32>(thing, 0x190), &at<u32>(thing, 0x194));
    Thing_SetPos(thing, nullptr, x, y);
}

void __fastcall Thing_ResetPlaceAt(u8 *thing, void *, u32 x, u32 y) {
    at<u32>(thing, 0x39c) = 0;
    Set4_4217f0(thing + 0xc, nullptr, 3);
    Thing_PlaceAt(thing, nullptr, x, y);
}

u32 __fastcall Thing_SpriteChoice(u8 *thing) {
    if (SpriteFolder_Count(3) != 0) {
        const u32 flags = at<u32>(thing, 0x20);
        return at<u32>(thing, 0x24) == 2 ? flags | 0x80000000u : flags;
    }
    u16 handle;
    World_SetSeed(global<u8 *>(0x0080239c), nullptr, *Thing_GetHandle(thing, nullptr, &handle));
    return SpriteFolder_RandomSprite(1);
}

u8 *__fastcall Thing_IngredientRecord(u8 *thing) {
    const u8 *definition = ObjectDefs_Find(reinterpret_cast<u8 *>(0x007890a0), nullptr, at<u16>(thing, 0xe));
    const u32 kind = at<u32>(definition, 0x158);
    if (kind - 1 > 3)
        return nullptr;
    return Records_DataIfTerminated(global<const u8 *>(0x0078ba8c), nullptr, kind - 1);
}

void __fastcall Obj_ReplaceHandle24(u8 *object, void *, u32 handle) {
    u8 *replacement = global<u8 *>(0x007cfb90 + 20 * (handle & 0xffff));
    const u16 old = at<u16>(object, 0x24);
    if (old != 0)
        at<u16>(global<u8 *>(0x007cfb90 + 20 * (u32)old), 8) = (u16)handle;
    at<u16>(replacement, 8) = 0;
    at<u16>(replacement, 0xa) = at<u16>(object, 0x24);
    at<u16>(object, 0x24) = (u16)handle;
}

u8 *__stdcall Thing_ByHandle(u32 handle) { return reinterpret_cast<u8 *const *>(0x007cfb90)[(handle & 0xffff) * 5]; }

u8 *__fastcall ThingKindE_Ctor(u8 *object) {
    Thing_Construct(object, nullptr, 1, 0xe);
    for (u32 offset = 0x1414; offset < 0x1414 + 5 * 0x10; offset += 0x10) {
        at<u8>(object, offset + 4) = 1;
        at<u32>(object, offset) = 0xffffffff;
    }
    return object;
}

u32 __fastcall Thing_Serialize(u8 *thing, void *, Stream *stream, u32 save, s32 version, u32) {
    if (version < 0 || version > 1)
        return 0;
    if (!Save_Field(stream, save, thing + 4, 2, 0x00761cbc) || !Save_Field(stream, save, thing + 6, 2, 0x00761cb8) ||
        !Save_Field(stream, save, thing + 0xa, 2, 0x00761cac) || !Save_Field(stream, save, thing + 8, 2, 0x00761ca0))
        return 1;
    if (save == 0)
        thing[3] = 0;
    return 0;
}

u32 __fastcall ThingBasic_Serialize(u8 *thing, void *, Stream *stream, u32 save, s32 version, u32 unused) {
    return Thing_Serialize(thing, nullptr, stream, save, version, unused) != 0 ? 1 : 0;
}
}  // extern "C"
