#include "texatlas.h"

#include <cstdint>

#include "../core/gameheap.h"

namespace {

// The free rectangles (x, y, width, height) at 0x8c4928, their count at 0x8c4b30, the page they are in at 0x8c4b28 and the pages
// made at 0x8c4b2c.
struct Rect {
    s32 x, y, width, height;
};

Rect *const kFree = reinterpret_cast<Rect *>((uintptr_t)0x008c4928);
u32 &Page() { return *reinterpret_cast<u32 *>((uintptr_t)0x008c4b28); }
s32 &Pages() { return *reinterpret_cast<s32 *>((uintptr_t)0x008c4b2c); }
s32 &Count() { return *reinterpret_cast<s32 *>((uintptr_t)0x008c4b30); }

}  // namespace

extern "C" {

u32 __cdecl TexAtlas_Alloc(u32 *out, s32 size) {
    if (size >= 0x100) {
        ++Pages();
        *out = TrackedGlobal_Alloc(0x40000);
        return 1;
    }
    s32 best = -1, bestWaste = 0xfffffff;
    for (s32 i = 0; i < Count(); ++i) {
        const Rect &rect = kFree[i];
        if (rect.width < size || rect.height < size)
            continue;
        if (rect.width == size && rect.height == size) {
            *out = Page() + ((rect.y << 8) + rect.x) * 4;
            kFree[i] = kFree[--Count()];
            return 0;
        }
        const s32 waste = (s32)((u32)rect.width * (u32)rect.height - (u32)size * (u32)size);
        if (best == -1 || waste < bestWaste) {
            best = i;
            bestWaste = waste;
        }
    }
    if (best == -1) {
        const u32 page = TrackedGlobal_Alloc(0x40000);
        Page() = page;
        *out = page;
        kFree[0] = {0, size, 0x100, 0x100 - size};
        kFree[1] = {size, 0, 0x100 - size, size};
        ++Pages();
        Count() = 2;
        return 1;
    }
    Rect &rect = kFree[best];
    *out = Page() + ((rect.y << 8) + rect.x) * 4;
    if (rect.width == size) {
        rect.height -= size;
        rect.y += size;
        return 0;
    }
    if (rect.height == size) {
        rect.width -= size;
        rect.x += size;
        return 0;
    }
    Rect &split = kFree[Count()];
    split = rect;
    rect.height -= size;
    rect.y += size;
    const s32 count = Count();   // (read before the new rectangle's width, which is the count itself when it was 32: sic)
    split.height = size;
    split.width -= size;
    split.x += size;
    Count() = count + 1;
    return 0;
}
}
