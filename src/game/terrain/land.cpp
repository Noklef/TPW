#include "land.h"

#include <cstdint>
#include <cstring>

#include "../core/buffers.h"
#include "../core/gameheap.h"
#include "../core/globalmem.h"
#include "../gfx/array3d.h"
#include "../math/x87.h"
#include "../render/frame.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

const u8 *map() { return *reinterpret_cast<const u8 *const *>(0x007a0854); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Terrain_IsParkLand(u32 x, u32 y) {
    const u8 *grid = map() ? at<const u8 *>(map(), 0x6c) : nullptr;
    if (!grid) return 1;
    const u32 width = at<u32>(grid, 0x18);
    if (x >= width || y >= at<u32>(grid, 0x1c)) return 0;
    return ~at<const u8 *>(grid, 0x2c)[(y * width + x) * 4] & 1;
}

void __cdecl Terrain_CopyAttrs(const u8 *map, const u8 *grid) {
    if (!map) return;
    const u8 *own = at<const u8 *>(map, 0x6c);
    if (!own || !at<u8 *>(own, 0x2c) || !at<const u8 *>(grid, 0x2c) || !at<u16>(own, 6)) return;
    u32 *to = at<u32 *>(own, 0x2c);  // rep movsd: forward, a dword at a time
    const u32 *from = at<const u32 *>(grid, 0x2c);
    for (u32 n = at<u16>(own, 6); n; --n) *to++ = *from++;
}

void __cdecl Terrain_PaintCell(const u8 *map, const u8 *sets, s32 x, s32 y, s32 set, s32 tile, u32 rotation) {
    const bool erase = !set && tile == 8, restore = !set && tile == 0x37;
    if (!erase && !restore) {
        if (!set) set = 1;
        const s32 count = at<s32>(sets, 0);
        if (count <= 0) return;
        const u8 *entries = at<const u8 *>(sets, 4);
        if (at<s32>(entries, (u32)set * 12 - 8) <= 0) return;  // sic: before set wraps
        set = (s32)((u32)set - 1) % count + 1;
        tile %= at<s32>(entries, (u32)set * 12 - 8);
    }
    if (x < 0) return;
    const u8 *grid = at<const u8 *>(map, 0x6c);
    const s32 width = at<s32>(grid, 0x18);
    if (x >= width || y < 0 || y >= at<s32>(grid, 0x1c)) return;
    const u32 offset = ((u32)y * (u32)width + (u32)x) * 4;
    auto flags = [&]() -> u16 & { return at<u16>(at<const u8 *>(at<const u8 *>(map, 0x6c), 0x2c), offset); };
    if (erase) {
        flags() |= 0x101;
        flags() &= 0xff7f;
        return;
    }
    if (restore) {
        if (!*reinterpret_cast<const u8 *const *>(0x0079fc54)) return;
        auto loaded = [&]() { return at<const u8 *>(*reinterpret_cast<const u8 *const *>(0x0079fc54), 0x2c) + offset; };
        at<u16>(&flags(), 2) = at<u16>(loaded(), 2);
        flags() = at<u16>(loaded(), 0);
        reinterpret_cast<u8 *>(&flags())[1] |= 1;
        if (*reinterpret_cast<const u32 *>(0x007a1a8c) & 0x4000) flags() &= 0xff87;
        return;
    }
    const u8 *tiles = at<const u8 *>(at<const u8 *>(sets, 4), (u32)set * 12 - 4);
    at<u16>(&flags(), 2) = at<u16>(tiles, (u32)tile * 12 + 8);
    flags() &= 0xff87;
    flags() &= 0xfffe;
    flags() |= 0x180;
    if (rotation == 90)
        flags() |= 0x20;
    else if (rotation == 180)
        flags() |= 0x10;
    else if (rotation == 270)
        flags() |= 8;
}

void __cdecl Terrain_GridSize(u32 *width, u32 *height) {
    *width = at<u32>(at<const u8 *>(map(), 0x6c), 0x18);
    *height = at<u32>(at<const u8 *>(map(), 0x6c), 0x1c);
}

void __cdecl Vec3_Cross(const float *a, const float *b, float *out) {
    out[0] = (float)(X87Reg(b[2]) * a[1] - X87Reg(a[2]) * b[1]);
    out[1] = (float)(X87Reg(b[0]) * a[2] - X87Reg(a[0]) * b[2]);
    out[2] = (float)(X87Reg(a[0]) * b[1] - X87Reg(b[0]) * a[1]);
}

void __cdecl Terrain_CloneAttrs(const u8 *map, u8 **out) {
    const u8 *const grid = map ? at<const u8 *>(map, 0x6c) : nullptr;
    if (!grid) {
        *out = nullptr;
        return;
    }
    u8 *const block = static_cast<u8 *>(GlobalMem_Alloc((at<u16>(grid, 6) + at<u16>(grid, 4)) * 4u + 0x30, 1));
    u8 *cells = block + 0x30;
    if (block) std::memcpy(block, grid, 0x30);
    at<u8 *>(block, 0x28) = cells;
    const u32 first = at<u16>(grid, 4) * 4u;
    cells += first;
    if (at<u8 *>(block, 0x28) && at<const u8 *>(grid, 0x28) && first)
        std::memcpy(at<u8 *>(block, 0x28), at<const u8 *>(grid, 0x28), first);
    at<u8 *>(block, 0x2c) = cells;
    if (cells && at<const u8 *>(grid, 0x2c) && at<u16>(grid, 6))
        std::memcpy(cells, at<const u8 *>(grid, 0x2c), at<u16>(grid, 6) * 4u);
    *out = block;
}

void __cdecl Terrain_RotateFootprint(s32 *x, s32 *y, s32 angle, s32 *outX, s32 *outY, s32 width, s32 height) {
    switch (angle) {
    case 90:
        ++*y;
        *outX = *x;
        *outY = *y - width;
        break;
    case 180:
        ++*x;
        ++*y;
        *outX = *x - width;
        *outY = *y - height;
        break;
    case 270:
        ++*x;
        *outX = *x - height;
        *outY = *y;
        break;
    default:
        *outX = *x;
        *outY = *y;
    }
}

void __cdecl Terrain_PaintAt(s32 x, s32 y, const s32 *paint) {
    const u8 *map = *reinterpret_cast<const u8 *const *>(0x007a0854);
    if (!map || !(at<u8>(map, 0x30) & 2)) return;
    Terrain_PaintCell(map, reinterpret_cast<const u8 *>(0x0074c8a8), x, y, paint[0], paint[1], (u32)paint[2]);
    Frame_AddFlags(1);
}

void __fastcall TileSet_Clear(u8 *object) {
    at<u8>(object, 4) = 0;
    at<u32>(object, 0) = 0;
    for (u32 offset = 8; offset <= 0xc8; offset += 0x10)
        Array3D_Free(object + offset);
    for (u32 offset = 0xc8; offset >= 0x28; offset -= 0x10) {
        u8 *const array = object + offset;
        if (at<void *>(array, 0xc)) GameHeap_Free(at<void *>(array, 0xc));
        at<u32>(array, 0) = 0;
        at<u32>(array, 4) = 0;
        at<u32>(array, 8) = 0;
        at<u32>(array, 0xc) = 0;
    }
    BufferC_FreeAndClear(object + 0x18);
    BufferC_FreeAndClear(object + 8);
}

void __fastcall Lnd_SetTileSize(u8 *object, void *, u32 size) {
    at<u8>(object, 0) = (u8)size;
    if (size == 0x40) {
        at<u8>(object, 1) = 0;
    } else if (size == 0x20) {
        at<u8>(object, 1) = 1;
    } else if (size == 0x10) {
        at<u8>(object, 1) = 2;
    } else {
        at<u8>(object, 0) = 0x20;
        at<u8>(object, 1) = 1;
    }
    if (global<u32>(0x008798c8))
        GameHeap_Free(reinterpret_cast<void *>(global<u32>(0x008798c8)));
    const u32 side = at<u8>(object, 0);
    global<u32>(0x008798c8) = (u32)(uintptr_t)GameHeap_Alloc(side * side * 4);
}

void __fastcall Lnd_Clear(u8 *object) {
    for (u32 offset = 2; offset <= 0xb; offset++)
        at<u8>(object, offset) = 0;
    at<u32>(object, 0xc) = 0;
    at<u32>(object, 0x10) = 0;
    Array3D_Free(object + 0x14);
    for (u32 level = 0; level < 3; level++) {
        u8 *const tiles = object + 0x2c + 0xd8 * level;
        at<u8>(tiles, -4) = 0;
        at<u32>(tiles, -8) = 0;
        for (u32 offset = 0; offset <= 0xc0; offset += 0x10)
            Array3D_Free(tiles + offset);
        at<u32>(tiles, -8) = 0x40 >> level;
        at<u8>(tiles, -4) = static_cast<u8>(level);
    }
}
}
