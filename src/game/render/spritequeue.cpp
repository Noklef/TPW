#include "spritequeue.h"

#include <cstdint>
#include <cstring>

#include "../core/addrcalls.h"
#include "../core/gameheap.h"
#include "../core/liveobjects.h"
#include "../render/engineerrors.h"
#include "../gfx/texture.h"
#include "../gfx/texman.h"
#include "../gfx/texcache.h"
#include "../core/staticobjects.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
u32 address(const void *p) { return (u32)(uintptr_t)p; }

AddrCalls g_calls;
AddrCalls g_drawCalls;

// SpriteQueue_Draw's sort: the frames of one texture and key in a list, the nodes a tree by those two, drawn in the order they were made.
struct SortNode {
    SortNode *lower, *higher;
    u32 texture;   // 0xffffffff: not filled yet
    u8 *frames;    // by depth, far first, on through the frames' +0x40
    s32 key;
};

u32 CloseBatch() { return g_drawCalls.Th(0x005658c0, global<u32>(0x00879ab0), 0xbf800000u); }   // -1.0f

void SortFrame(SortNode *nodes, SortNode *&next, u8 *frame, u32 texture, s32 key) {
    at<u32>(frame, 0x40) = 0;
    SortNode *node = nodes;
    if (next == nodes) ++next;
    else while (node->texture != 0xffffffff) {
        if (node->texture == texture && node->key == key) {
            u8 *before = nullptr, *cur = node->frames;
            while (cur && !(at<float>(frame, 8) >= at<float>(cur, 8))) {
                before = cur;
                cur = at<u8 *>(cur, 0x40);
            }
            at<u8 *>(frame, 0x40) = cur;
            if (before) at<u8 *>(before, 0x40) = frame;
            else node->frames = frame;
            return;
        }
        SortNode *&link = node->texture > texture || (node->texture == texture && node->key > key) ? node->higher : node->lower;
        if (link == reinterpret_cast<SortNode *>((uintptr_t)0xffffffff)) link = next++;
        node = link;
    }
    node->frames = frame;
    node->texture = texture;
    node->key = key;
}

}  // namespace

void SpriteQueue_SetCalls(const AddrCall *table, u32 count) { g_calls.Set(table, count); }

extern "C" u32 __fastcall SpriteQueue_DrawWithPanels(u8 *self) {
    if (!g_drawCalls.Th(0x0058a770, (u32)(uintptr_t)self)) return 0;   // SpriteQueue_Draw
    for (u32 i = at<u32>(self, 0x2408); i > 0; --i)
        if (!g_drawCalls.Th(0x0058a010, at<u32>(self, 0x2000 + (i - 1) * 4))) return 0;   // TiledPanel_Draw
    return 1;
}
void SpriteQueue_SetDrawCalls(const AddrCall *table, u32 count) { g_drawCalls.Set(table, count); }

extern "C" {

u32 __fastcall SpriteQueue_Draw(u8 *queue) {
    const u32 render = global<u32>((u32)(uintptr_t)&gView_8bcbc8);
    reinterpret_cast<u32 &>(global<u8 *>((u32)(uintptr_t)&gView_8bcbc8 + 0x934)) = 0;
    if (render & 0x80) {
        for (u32 i = at<u32>(queue, 0x2404); i--;) {
            if (!g_drawCalls.Th(0x00589f90, at<u32>(queue, i * 4))) return 0;
            if (!(render & 0x20000) && !g_drawCalls.Th(0x00589770, at<u32>(queue, i * 4))) return 0;
        }
        return 1;
    }

    SortNode nodes[0x800];
    std::memset(nodes, 0xff, sizeof(nodes));
    SortNode *next = nodes;
    s32 key = 0;   // (sic: unset stack in the original until a frame with flag 4)
    for (u32 i = at<u32>(queue, 0x2404); i--;) {
        u8 *const frame = at<u8 *>(queue, i * 4);
        const u32 flags = at<u32>(frame, 0x10);
        if (flags & 4) key = (flags & 0x2000 ? 0x30 : 0) + 0x200522;
        if (flags & 0x200000) key |= 0x200000;
        if (flags & 0x20000) key |= 0x20000;
        if (flags & 0x4000) key |= 0x800;
        if (flags & 0x10000) key |= 0x8000000;
        const u8 *const texture = at<const u8 *>(at<const u8 *>(frame, 0x28), 0);
        SortFrame(nodes, next, frame, at<u32>(texture, 8) & 0x1000 ? at<u32>(texture, 0x1c) : 0, key);
    }

    at<u32>(global<u8 *>(0x00879ab0), 0x20) = 0;
    for (SortNode *node = nodes; node < next; ++node) {
        if (!node->frames) continue;
        for (u8 *frame = node->frames; frame; frame = at<u8 *>(frame, 0x40))
            if (!g_drawCalls.Th(0x00589fd0, address(frame))) return 0;
        if (!CloseBatch()) return 0;
    }

    const u32 after = global<u32>((u32)(uintptr_t)&gView_8bcbc8);
    if (after & 0x20000) return 1;
    if (after & 0x40000) {
        if (!at<u32>(queue, 0x2400)) return 1;
        for (SortNode *node = nodes; node < next; ++node)
            for (u8 *frame = node->frames; frame; frame = at<u8 *>(frame, 0x40))
                if (!g_drawCalls.Th(0x00589990, address(frame), at<u32>(queue, 0x2400))) return 0;
        return CloseBatch() != 0;
    }
    for (SortNode *node = nodes; node < next; ++node) {
        if (!node->frames) continue;
        for (u8 *frame = node->frames; frame; frame = at<u8 *>(frame, 0x40))
            if (!g_drawCalls.Th(0x00589990, address(frame), 0u)) return 0;
        if ((node + 1 == next || node->texture != node[1].texture) && !CloseBatch()) return 0;
    }
    return 1;
}

u32 __fastcall SpriteQueue_Ready(u8 *setPlus20) {
    u8 *set = setPlus20 - 0x20;
    const u32 stampA = *reinterpret_cast<u32 *>((u32)(uintptr_t)&gView_8bcbc8 + 0x78), stampB = *reinterpret_cast<u32 *>((u32)(uintptr_t)&gView_8bcbc8 + 0x7c);
    if (at<u32>(set, 0xc) == stampA && at<u32>(set, 0x10) == stampB)
        return 1;
    u8 *queue = *reinterpret_cast<u8 **>(0x008c47a0);
    u32 result = 0;
    if (at<s32>(queue, 0x240c) < 0x80) {
        at<u32>(queue, 0x2200 + at<s32>(queue, 0x240c) * 4) = (u32)(uintptr_t)set;
        at<s32>(queue, 0x240c) = at<s32>(queue, 0x240c) + 1;
        result = 1;
    }
    at<u32>(set, 0xc) = stampA;
    at<u32>(set, 0x10) = stampB;
    return result;
}

u32 __fastcall SpriteQueue_Add(u8 *queue, void *, u32 frame) {
    s32 n = at<s32>(queue, 0x2404);
    if (n >= 0x800)
        return 0;
    reinterpret_cast<u32 *>(queue)[n] = frame;
    at<s32>(queue, 0x2404) = at<s32>(queue, 0x2404) + 1;
    return 1;
}

u32 __fastcall SpriteQueue_Add2(u8 *queue, void *, u32 frame) {
    s32 n = at<s32>(queue, 0x2408);
    if (n >= 0x800)
        return 0;
    reinterpret_cast<u32 *>(queue + 0x2000)[n] = frame;
    // re-read: the entry may have been the count itself
    at<s32>(queue, 0x2408) = at<s32>(queue, 0x2408) + 1;
    return 1;
}

u32 __cdecl SpriteQueue_Free() {
    u8 *const queue = *reinterpret_cast<u8 **>(0x008c47a0);
    if (!queue)
        return 0;
    if (u8 *const textures = at<u8 *>(queue, 0x2400)) {
        Texture_DestroyArray(textures);
        at<u32>(queue, 0x2400) = 0;
    }
    GameHeap_Free(queue);
    *reinterpret_cast<u8 **>(0x008c47a0) = nullptr;
    return 1;
}

u8 *__fastcall SpriteSet_DeletingDtor(u8 *plus20, void *, u32 flags) {
    u8 *const set = plus20 - 0x20;
    const s32 base = at<s32>(at<u8 *>(set, 0), 4);
    at<u32>(set, base) = (u32)(uintptr_t)Class6ff308_vtable;
    const s32 again = at<s32>(at<u8 *>(set, 0), 4);
    at<s32>(set, again - 4) = again - 0x20;
    TrackedArray_Free(set);
    LiveObjects_Remove(set ? set + at<s32>(at<u8 *>(set, 0), 4) : nullptr);   // (the base read again after the release)
    at<u32>(set, 0x20) = 0x006fdce8;
    if (flags & 1)
        GameHeap_Free(set);
    return set;
}

void __fastcall SpriteSet_Dtor(u8 *set) {
    const s32 base = at<s32>(at<u8 *>(set, 0), 4);
    at<u32>(set, base) = (u32)(uintptr_t)Class6ff308_vtable;
    const s32 again = at<s32>(at<u8 *>(set, 0), 4);
    at<s32>(set, again - 4) = again - 0x20;
    TrackedArray_Free(set);
    LiveObjects_Remove(set ? set + at<s32>(at<u8 *>(set, 0), 4) : nullptr);   // (the base read again after the release)
    at<u32>(set, 0x20) = 0x006fdce8;
}

u32 __cdecl SpriteQueue_Create() {
    u8 *&slot = *reinterpret_cast<u8 **>(0x008c47a0);
    if (slot)
        return 0;
    u8 *const queue = static_cast<u8 *>(GameHeap_Alloc(0x2410));
    if (queue) {
        at<u32>(queue, 0x2404) = 0;
        at<u32>(queue, 0x2408) = 0;
        at<u32>(queue, 0x2400) = 0;
        at<u32>(queue, 0x240c) = 0;
    }
    slot = queue;
    if (!queue) {
        EngineErrors_Push(reinterpret_cast<u8 *>((uintptr_t)0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __fastcall SpriteQueue_LoadTexture(u8 *queue, void *, const char *name) {
    u8 **const slot = reinterpret_cast<u8 **>(queue + 0x2400);
    if (*slot != nullptr) {
        Texture_DestroyArray(*slot);
        *slot = nullptr;
    }
    *slot = TexManager_Load(*reinterpret_cast<u8 *const *>(&gTexman_87a268), nullptr, name);
    if (*slot != nullptr)
        *reinterpret_cast<u32 *>(*slot + 8) = (*reinterpret_cast<u32 *>(*slot + 8) & ~0x80u) | 0x40040;
    return *slot != nullptr;
}

u32 __fastcall SpriteQueue_Clear(u8 *queue) {
    at<u32>(queue, 0x2404) = 0;
    at<u32>(queue, 0x2408) = 0;
    at<u32>(queue, 0x240c) = 0;
    return g_calls.Th(0x58c9f0, gTexcache_8c4b44, at<u32>(queue, 0x2400), 0u);
}

u32 __fastcall SpriteQueue_ShowAll(u8 *queue) {
    for (u32 l = at<u32>(queue, 0x240c); l != 0; l--) {
        const u8 *const list = at<const u8 *>(queue, 0x2200 + (l - 1) * 4);
        for (u32 f = at<u32>(list, 4); f != 0; f--)
            g_calls.Th(0x588ef0, at<const u32 *>(list, 0x14)[f - 1]);
    }
    return 1;
}

u32 __fastcall Global8c47b0_Touch(u8 *object) {
    g_calls.Th(0x58c9f0, gTexcache_8c4b44, at<u32>(object, 8), 0x3f000000u);
    return 1;
}

void __cdecl Global8c47b0_Release() {
    u8 *const record = reinterpret_cast<u8 *>((uintptr_t)0x008c47b0);
    if (at<u8 *>(record, 8)) {
        Texture_Release(at<u8 *>(record, 8));
        if (u8 *const textures = at<u8 *>(record, 8))
            Texture_DestroyArray(textures);
    }
    if (const u32 block = at<u32>(record, 0xc)) {
        TrackedGlobal_Free(block);
        at<u32>(record, 4) = 0;
        at<u32>(record, 0) = 0;
    }
}
}
