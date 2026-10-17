#include "small58.h"

#include <cstdint>

#include "../render/renderer.h"
#include "../world/analysercell.h"
#include "../world/mapcell.h"
#include "../world/thingscan.h"
#include "small15.h"
#include "small37.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

// -1, 0 or 1: the 16-bit characters up to the shorter length, then the lengths.
s32 Compare(const u16 *a, u32 aLength, const u16 *b, u32 bLength) {
    for (u32 n = aLength < bLength ? aLength : bLength; n != 0; --n, ++a, ++b) {
        if (*a != *b)
            return *a < *b ? -1 : 1;
    }
    if (aLength < bLength)
        return -1;
    return aLength != bLength ? 1 : 0;
}

u8 *CellOf(u32 column, u32 row) {
    const u32 id = ((row << 7) + column + 1) & 0xffff;
    return global<u8 *>(0x008023a0) + id * 0x44 - 0x44;
}

}  // namespace

extern "C" {

u32 *__fastcall WStringMap_Find(u8 *map, void *, u32 *out, const u8 *key) {
    u8 *const found = WStringMap_LowerBound(map, nullptr, key);
    u8 *const head = at<u8 *>(map, 4);
    if (found != head) {
        const u16 *chars = at<const u16 *>(found, 0x10);
        if (!chars)
            chars = reinterpret_cast<const u16 *>(0x006fd6c4);
        // the key against the node's string
        if (Compare(at<const u16 *>(key, 4), at<u32>(key, 8), chars, at<u32>(found, 0x14)) >= 0) {
            *out = (u32)(uintptr_t)found;
            return out;
        }
    }
    *out = (u32)(uintptr_t)head;
    return out;
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

void __fastcall Object_BackOfQueue(u8 *object, void *, u16 *out) {
    if (at<u16>(object, 0x3a) != 0) {
        *out = at<u16>(object, 0x3a);
        return;
    }
    u16 cell = 0;
    Object_NeighbourCellId(object, nullptr, &cell);
    at<u32>(object, 0x40) = 0;
    at<u16>(object, 0x3a) = cell;
    s32 budget = 1000, left;
    for (;;) {
        left = budget;
        if (cell == 0)
            break;
        left = budget - 1;
        if (!(0 < budget))
            break;
        at<u16>(object, 0x3a) = cell;
        u16 next;
        cell = *Map_FindFacingNeighbour(&next, cell);
        at<u32>(object, 0x40) = at<u32>(object, 0x40) + 1;
        budget = left;
    }
    if (0 < left) {
        *out = at<u16>(object, 0x3a);
        return;
    }
    *out = 0;  // (the message "GetBackOfQueue crashed" is left out)
}

void __cdecl Holder_Sleep(u8 *holder) {
    u8 *const part = at<u8 *>(holder, 0x1c);
    const u32 flags = at<u32>(holder, 0);
    u8 *key = nullptr;
    if (part)
        key = part + at<s32>(at<u8 *>(part, 0), 4);
    u32 tag;
    if (flags & 0x10)
        tag = 0x10;
    else if (flags & 8)
        tag = 0x20;
    else
        tag = 0x70;
    Renderer_RemoveFromSlot(global<u8 *>(0x0087a6d0), nullptr, (u32)(uintptr_t)key, tag);
    at<u32>(holder, 0) = at<u32>(holder, 0) & 0xffffffe6;
}

}  // extern "C"
