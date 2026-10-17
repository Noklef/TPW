#include "renderer.h"

#include <cstdint>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/vcall.h"
#include "../core/vobj3.h"
#include "../gfx/array3d.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

u32 &SceneFlags() { return *reinterpret_cast<u32 *>(0x008bd508); }

// A node {key, next} goes on the front of the free list.
void Free(u8 *renderer, u8 *node) {
    at<u32>(node, 4) = at<u32>(renderer, 4);
    at<u32>(renderer, 4) = (u32)(uintptr_t)node;
    at<u32>(renderer, 0xc)--;
}

void FreeList(u8 *renderer, u8 *node) {
    while (node) {
        u8 *next = reinterpret_cast<u8 *>(at<uintptr_t>(node, 4));
        Free(renderer, node);
        node = next;
    }
}

// The first node with the key leaves the list at head; whether there was one.
bool Remove(u8 *renderer, u32 *head, u32 key) {
    u8 *prev = nullptr;
    for (u8 *node = reinterpret_cast<u8 *>(*head); node; node = reinterpret_cast<u8 *>(at<uintptr_t>(node, 4))) {
        if (at<u32>(node, 0) == key) {
            if (prev)
                at<u32>(prev, 4) = at<u32>(node, 4);
            else
                *head = at<u32>(node, 4);
            Free(renderer, node);
            return true;
        }
        prev = node;
    }
    SceneFlags() |= 0x80000000;
    return false;
}

u32 *Slot(u8 *renderer, u32 tag) { return &at<u32>(renderer, 0x10 + ((s32)tag >> 4) * 4); }

// Four arrays from base + 0x2c in steps of 0x10: Array3D_Free on each, the block of the last freed and cleared, then BufferC_FreeAndClear on the first three from the third back.
void ClearGroup(u8 *base) {
    Array3D_Free(base + 0x2c);
    Array3D_Free(base + 0x3c);
    Array3D_Free(base + 0x4c);
    Array3D_Free(base + 0x5c);
    u8 *const last = base + 0x5c;
    if (at<void *>(last, 0xc)) GameHeap_Free(at<void *>(last, 0xc));
    at<u32>(last, 0) = 0;
    at<u32>(last, 4) = 0;
    at<u32>(last, 8) = 0;
    at<u32>(last, 0xc) = 0;
    BufferC_FreeAndClear(base + 0x4c);
    BufferC_FreeAndClear(base + 0x3c);
    BufferC_FreeAndClear(base + 0x2c);
}

}  // namespace

extern "C" {

void __fastcall Renderer_SetRestoreCallback(u8 *renderer, void *, u32 fn) { at<u32>(renderer, 0x78) = fn; }

void __fastcall Renderer_SetStateFlags(u8 *renderer, void *, u32 flags) { at<u32>(renderer, 0x48) = flags; }

u32 __fastcall Renderer_StateFlags(const u8 *renderer) { return at<u32>(renderer, 0x48); }

u32 __fastcall Renderer_RemoveFromSlot(u8 *renderer, void *, u32 key, u32 tag) {
    if (!(tag & 0x70))
        return 0;
    return Remove(renderer, Slot(renderer, tag), key);
}

u32 __fastcall Renderer_ClearSlot(u8 *renderer, void *, u32 tag) {
    if (!(tag & 0x70))
        return 0;
    u32 *slot = Slot(renderer, tag);
    FreeList(renderer, reinterpret_cast<u8 *>(*slot));
    *slot = 0;
    SceneFlags() |= 0x80000000;
    return 1;
}

u32 __fastcall Renderer_RemoveInRect(u8 *renderer, void *, s32 x0, s32 y0, s32 x1, s32 y1, u32 key) {
    if (!at<u32>(renderer, 0x10))
        return 1;
    const s32 width = at<s32>(renderer, 0x38), height = at<s32>(renderer, 0x3c);
    if (x0 >= width || x1 < 0 || y0 >= height || y1 < 0)
        return 0;
    if (x0 < 0)
        x0 = 0;
    if (x1 >= width)
        x1 = width - 1;
    if (y0 < 0)
        y0 = 0;
    if (y1 >= height)
        y1 = height - 1;
    u32 all = 1;
    for (s32 y = y0; y <= y1; y++)
        for (s32 x = x0; x <= x1; x++)
            all &= Remove(renderer, reinterpret_cast<u32 *>(at<uintptr_t>(renderer, 0x10) + (y * width + x) * 4), key);
    return all;
}

u32 __fastcall Renderer_ClearAll(u8 *renderer) {
    const s32 width = at<s32>(renderer, 0x38), height = at<s32>(renderer, 0x3c);
    if (width > 0 && height > 0) {
        for (s32 y = 0; y < height; y++) {
            for (s32 x = 0; x < width; x++) {
                u32 *cell = reinterpret_cast<u32 *>(at<uintptr_t>(renderer, 0x10) + (y * at<s32>(renderer, 0x38) + x) * 4);
                FreeList(renderer, reinterpret_cast<u8 *>(*cell));
                *cell = 0;
                SceneFlags() |= 0x80000000;
            }
        }
    }
    for (u32 offset = 0x14; offset < 0x30; offset += 4) {
        FreeList(renderer, reinterpret_cast<u8 *>(at<uintptr_t>(renderer, offset)));
        at<u32>(renderer, offset) = 0;
        SceneFlags() |= 0x80000000;
    }
    return 1;
}

u32 __fastcall Grid_AddObject(u8 *grid, void *, s32 x0, s32 y0, s32 x1, s32 y1, u8 *object, u32) {
    u32 result = 1;
    const s32 width = at<s32>(grid, 0x38);
    const s32 height = at<s32>(grid, 0x3c);
    if (x0 >= width || x1 < 0 || y0 >= height || y1 < 0) return 0;
    if (x0 < 0) x0 = 0;
    if (x1 >= width) x1 = width - 1;
    if (y0 < 0) y0 = 0;
    if (y1 >= height) y1 = height - 1;
    for (s32 x = x0; x <= x1; ++x) {
        for (s32 y = y0; y <= y1; ++y)
            result &= Pool_AddSorted(grid, nullptr, reinterpret_cast<u8 **>(at<u8 *>(grid, 0x10) + (y * at<s32>(grid, 0x38) + x) * 4), object);
    }
    return result;
}

void __fastcall Renderer_Teardown(u8 *object) {
    vcall<u32>(at<u8 *>(object, 0x14), 0x18);
    ClearGroup(object + 0x3c4);
    ClearGroup(object + 0x358);
    static const u32 offsets[2] = {0x28u, 0x18u};
    for (u32 offset : offsets) {
        u8 *const block = object + offset;
        if (at<void *>(block, 0xc)) GameHeap_Free(at<void *>(block, 0xc));
        at<u32>(block, 0) = 0;
        at<u32>(block, 4) = 0;
        at<u32>(block, 8) = 0;
        at<u32>(block, 0xc) = 0;
    }
}
}
