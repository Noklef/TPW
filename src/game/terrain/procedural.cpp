#include "procedural.h"

#include <cstdint>
#include <cstring>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/thunks.h"
#include "../core/misc.h"
#include "../math/x87.h"
#include "../render/engineerrors.h"
#include "../render/meshlist.h"
#include "../render/renderer.h"
#include "../render/worldview.h"
#include "land.h"
#include "texcache.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}

u32 &global(u32 address) { return *reinterpret_cast<u32 *>(address); }
const u8 *pointer(u32 address) { return *reinterpret_cast<const u8 *const *>(address); }

u32 &render_flags() { return global(0x008bd550); }
u32 &terrain_flags() { return global(0x007a1a8c); }

}  // namespace

extern "C" {

void __cdecl TerrainTex_Enable() {
    if (!(render_flags() & 0x10000000)) render_flags() |= 4;
}

void __cdecl TerrainTex_Disable() { render_flags() &= ~4u; }

void __cdecl TerrainTex_SetRect(u32 x0, u32 y0, u32 x1, u32 y1) {
    u8 *tex = *reinterpret_cast<u8 **>(0x00879d60);
    if (!tex) return;
    at<u32>(tex, 0x234) = x0;
    at<u32>(tex, 0x238) = y0;
    at<u32>(tex, 0x23c) = x1;
    at<u32>(tex, 0x240) = y1;
}

u32 __cdecl TerrainTex_Create() {
    u8 *&tex = *reinterpret_cast<u8 **>(0x00879d60);
    if (tex) return 0;
    u8 *const p = static_cast<u8 *>(GameHeap_Alloc(0x12974));
    if (p) {
        for (const u32 offset : {0x0u, 0x4u, 0x20u, 0x24u, 0x128u, 0x12cu, 0x230u, 0x234u, 0x238u, 0x2958u, 0x12968u, 0x12970u}) at<u32>(p, offset) = 0;
        at<u32>(p, 0x23c) = 0x7f;
        at<u32>(p, 0x240) = 0x7f;
    }
    tex = p;
    if (!p) {
        EngineErrors_Push(reinterpret_cast<u8 *>(0x00879ad0), nullptr, 3, 0xb0754000);
        return 0;
    }
    return 1;
}

u32 __cdecl TerrainTex_Destroy() {
    u8 *&global_tex = *reinterpret_cast<u8 **>(0x00879d60);
    if (!global_tex) return 1;
    TerrainTex_Unload(global_tex);
    if (u8 *const tex = global_tex) {
        TerrainTex_Unload(tex);   // sic: a second time
        GameHeap_Free(tex);
    }
    global_tex = nullptr;
    return 1;
}

void __fastcall TerrainTex_AddRect(u8 *tex, void *, s32 x0, s32 y0, s32 x1, s32 y1, float delta) {
    if (y0 > y1) return;
    s32 y = y0;
    do {
        if (y >= 0 && (u32)y <= at<u32>(pointer((uintptr_t)tex), 0x1c) && x0 <= x1) {
            s32 x = x0;
            do {
                if (x >= 0) {
                    const u32 width = at<u32>(pointer((uintptr_t)tex), 0x18);
                    if ((u32)x <= width) {
                        float *const cell = reinterpret_cast<float *>((uintptr_t)at<u32>(tex, 0x12970)) + ((width + 1) * (u32)y + (u32)x);
                        *cell = delta + *cell;
                    }
                }
                x = (s32)((u32)x + 1);
            } while (x <= x1);
        }
        y = (s32)((u32)y + 1);
    } while (y <= y1);
}

void __fastcall TerrainTex_StampFixed(u8 *tex) {
    // (the first six inlined in the original as cell - -4.0: the same sums)
    TerrainTex_AddRect(tex, nullptr, 0x1c, 0x0, 0x1e, 0xc, 4.0f);
    TerrainTex_AddRect(tex, nullptr, 0x1f, 0xb, 0x2b, 0xd, 4.0f);
    TerrainTex_AddRect(tex, nullptr, 0x2c, 0xb, 0x2e, 0x10, 4.0f);
    TerrainTex_AddRect(tex, nullptr, 0x1d, 0x0, 0x1d, 0xc, 4.0f);
    TerrainTex_AddRect(tex, nullptr, 0x1e, 0xb, 0x2c, 0xc, 4.0f);
    TerrainTex_AddRect(tex, nullptr, 0x2d, 0xb, 0x2d, 0x10, 4.0f);
    static const struct {
        s32 x0, y0, x1, y1;
        float delta;
    } kRects[] = {
        {0x1f, 0xa, 0x2f, 0xa, 1.0f},  {0x20, 0x8, 0x22, 0x8, 1.0f},  {0x1f, 0x9, 0x2e, 0x9, 1.0f},  {0x1f, 0x0, 0x20, 0x8, 1.0f},
        {0x21, 0x6, 0x22, 0x8, 1.0f},  {0x23, 0x7, 0x27, 0x8, 1.0f},  {0x42, 0x0, 0x44, 0xc, 4.0f},  {0x35, 0xb, 0x41, 0xd, 4.0f},
        {0x32, 0xb, 0x34, 0x10, 4.0f}, {0x43, 0x0, 0x43, 0xc, 4.0f},  {0x34, 0xb, 0x43, 0xc, 4.0f},  {0x33, 0xb, 0x33, 0x10, 4.0f},
        {0x31, 0xa, 0x41, 0xa, 1.0f},  {0x30, 0x8, 0x32, 0x8, 1.0f},  {0x31, 0x9, 0x41, 0x9, 1.0f},  {0x3f, 0x0, 0x41, 0x8, 1.0f},
        {0x3f, 0x6, 0x41, 0x8, 1.0f},  {0x3a, 0x7, 0x3e, 0x8, 1.0f},  {0x21, 0x0, 0x2e, 0x6, 1.4f},  {0x32, 0x0, 0x3c, 0x6, 1.4f},
        {0x2e, 0x0, 0x31, 0x4, 1.4f},
    };
    for (const auto &r : kRects)
        TerrainTex_AddRect(tex, nullptr, r.x0, r.y0, r.x1, r.y1, r.delta);
}

u32 __fastcall TerrainTex_Setup(u8 *tex, void *, const u8 *land) {
    const u8 *const header = at<const u8 *>(land, 0x6c);
    at<const u8 *>(tex, 0) = header;
    at<u32>(tex, 4) = at<u32>(land, 0x50);
    at<u32>(tex, 8) = at<u32>(header, 0x18) + 1;
    at<u32>(tex, 0xc) = at<u32>(header, 0x1c) + 1;
    at<float>(tex, 0x10) = (float)(X87Reg(1.0f) / X87Reg(at<float>(header, 0x10)));
    at<float>(tex, 0x14) = (float)(X87Reg(1.0f) / X87Reg(at<float>(header, 0x14)));
    const X87Reg span = X87Reg(at<float>(header, 0x24)) - X87Reg(at<float>(header, 0x20));
    if (span.v == 0.0 || span.v != span.v)  // (fcom 0.0: C3 for equal or unordered)
        at<u32>(tex, 0x18) = 0x3d800000;  // 0.0625
    else
        at<float>(tex, 0x18) = (float)(X87Reg(1.0f) / span);
    at<u32>(tex, 0x12968) = 0;
    if (global(0x008bcbc8) & 0x10000) {
        if (at<u32>(tex, 0x12970)) {
            TrackedGlobal_Free(at<u32>(tex, 0x12970));
            at<u32>(tex, 0x12970) = 0;
        }
        const u32 size = (u32)at<u16>(at<const u8 *>(tex, 0), 4) << 2;
        const u32 block = TrackedGlobal_Alloc(size);
        at<u32>(tex, 0x12970) = block;
        if (block) {
            const u8 *const from = at<const u8 *>(at<const u8 *>(tex, 0), 0x28);
            u8 *const to = reinterpret_cast<u8 *>((uintptr_t)block);
            for (u32 i = 0; i < size; ++i)  // rep movsd / movsb
                to[i] = from[i];
            TerrainTex_StampFixed(tex);
        }
    }
    return TerrainTex_SetView(tex);
}

u32 __fastcall TerrainTex_SetView(u8 *tex) {
    const u8 *header = at<const u8 *>(tex, 0);
    WorldView_SetRangeX(const_cast<u8 *>(pointer(0x00879d54)), nullptr, at<float>(header, 8),
                        (float)(X87Reg(X87_MulInt(at<u32>(header, 0x18), at<float>(header, 0x10))) + X87Reg(at<float>(header, 8))),
                        at<float>(header, 0x10));
    header = at<const u8 *>(tex, 0);
    WorldView_SetRangeY(const_cast<u8 *>(pointer(0x00879d54)), nullptr, at<float>(header, 0xc),
                        (float)(X87Reg(X87_MulInt(at<u32>(header, 0x1c), at<float>(header, 0x14))) + X87Reg(at<float>(header, 0xc))),
                        at<float>(header, 0x14));
    header = at<const u8 *>(tex, 0);
    Obj_Set30_34(const_cast<u8 *>(pointer(0x00879d54)), nullptr, at<u32>(header, 0x20), at<u32>(header, 0x24));
    WorldView_UpdateDiagonal(const_cast<u8 *>(pointer(0x00879d54)));
    header = at<const u8 *>(tex, 0);
    Renderer_MakeFrameBuffer(const_cast<u8 *>(pointer(0x0087a6d0)), nullptr, at<s32>(header, 0x18), at<s32>(header, 0x1c));
    return 1;
}

u32 __fastcall TerrainTex_Unload(u8 *tex) {
    if (at<u32>(tex, 0x230)) {
        TerrainTexCache_Dtor(reinterpret_cast<u8 *>((uintptr_t)at<u32>(tex, 0x230)));
        if (u8 *const cache = reinterpret_cast<u8 *>((uintptr_t)at<u32>(tex, 0x230))) {
            Wrap_TerrainTexCache_Dtor_572100(cache);
            GameHeap_Free(cache);
        }
        at<u32>(tex, 0x230) = 0;
    }
    if (const u32 block = at<u32>(tex, 0x12970)) {
        TrackedGlobal_Free(block);
        at<u32>(tex, 0x12970) = 0;
    }
    at<u32>(tex, 0) = 0;
    at<u32>(tex, 4) = 0;
    return 1;
}

void __cdecl Terrain_ProceduralEnter() {
    const u32 flags = terrain_flags();
    if (!(flags & 0x2000) || (flags & 0x4000)) return;
    const u32 option = global(0x0078d8d8);
    if (option == 1) {
        terrain_flags() = flags | 0x4000;
        TerrainTex_Enable();
        Terrain_SetProceduralCells(1);
    } else if (option == 0) {
        terrain_flags() = flags & ~0x6000u;
    }
}

void __cdecl Terrain_ProceduralLeave() {
    const u32 flags = terrain_flags();
    if (!(flags & 0x2000) || !(flags & 0x4000)) return;
    terrain_flags() = flags & ~0x4000u;
    TerrainTex_Disable();
    Terrain_SetProceduralCells(0);
}

u32 __cdecl Terrain_ProceduralOn() { return (terrain_flags() & 0x2000) && (terrain_flags() & 0x4000) ? 1 : 0; }

void __cdecl Terrain_SetProceduralCells(u32 on) {
    // Read either way (volatile, so a missing loaded grid faults as in the original).
    const u8 *loaded = *reinterpret_cast<const u8 *const volatile *>((uintptr_t)pointer(0x0079fc54) + 0x2c);
    const u8 *grid = at<const u8 *>(pointer(0x007a0854), 0x6c);
    u8 *cell = at<u8 *>(grid, 0x2c);
    const u32 n = at<u16>(grid, 6);
    for (u32 i = 0; i < n; ++i, cell += 4, loaded += 4) {
        const u32 flags = at<u16>(cell, 0);
        if (flags & 0x81) continue;
        if (on) {
            at<u16>(cell, 0) = (u16)(flags & 0xff87);
        } else {
            at<u16>(cell, 0) = (u16)(flags & 0xff07);
            at<u16>(cell, 0) = (u16)((loaded[0] & 0xf8) | (flags & 0xff07));  // the byte read after the store
        }
    }
}

void __cdecl Terrain_RestoreCells() { Terrain_CopyAttrs(pointer(0x007a0854), pointer(0x0079fc54)); }

void __cdecl Terrain_InitUnusedOne() { *reinterpret_cast<float *>(0x007a0b28) = 1.0f; }

void __cdecl Terrain_InitUnusedEpsilon() { *reinterpret_cast<float *>(0x007a0b48) = 1.0f / 8388608.0f; }

void __fastcall Extents_Include(u8 *extents, void *, s32 low, s32 first, s32 high, s32 last) {
    if (at<s32>(extents, 0x20) > first)
        at<s32>(extents, 0x20) = first;
    if (at<s32>(extents, 0x24) < last)
        at<s32>(extents, 0x24) = last;
    for (s32 column = first; column <= last; column++) {
        s8 &lowest = at<s8>(extents, 0x28 + (u32)column);
        s8 &highest = at<s8>(extents, 0xa8 + (u32)column);
        if (lowest > low)
            lowest = (s8)low;
        if (highest < high)
            highest = (s8)high;
    }
}

void __fastcall TerrainTex_LightGrid(u8 *tex) {
    if (at<u32>(tex, 0x12968) == global(0x008bd50c) && !(global(0x008bd508) & 0x40000000))
        return;
    struct {
        float normals[0x600 / 4];
        float points[0x800 / 4];
    } frame;
    const u8 *header = at<const u8 *>(tex, 0);
    float z = at<float>(header, 0xc);
    const float *h = at<const float *>(header, 0x28);
    u32 *colours = &at<u32>(tex, 0x2968);
    for (s32 row = 0; row < at<s32>(tex, 0xc); ++row) {
        const s32 cols = at<s32>(tex, 8);
        X87Reg x = X87Reg(at<float>(header, 8));
        for (s32 i = 0; i < cols; ++i) {
            float *const point = &frame.points[4 * i];
            float *const normal = &frame.normals[3 * i];
            point[0] = (float)x;
            std::memcpy(&point[1], h, 4);
            std::memcpy(&point[2], &z, 4);
            const X87Reg dx = i > 0 ? (i < cols - 1 ? X87Reg(h[-1]) - h[1] : X87Reg(h[-1]) - h[0]) : X87Reg(h[0]) - h[1];
            normal[0] = (float)dx;
            normal[0] = (float)(X87Reg(normal[0]) * at<float>(tex, 0x18));
            const u32 one = 0x3f800000;
            std::memcpy(&normal[1], &one, 4);
            X87Reg dz = X87Reg(h[0]) - h[cols];
            if (row > 0)
                dz = row < at<s32>(tex, 0xc) - 1 ? X87Reg(h[-cols]) - h[cols] : X87Reg(h[-128]) - h[0];
            normal[2] = (float)dz;
            normal[2] = (float)(X87Reg(normal[2]) * at<float>(tex, 0x18));
            ++h;
            header = at<const u8 *>(tex, 0);
            x = x + at<float>(header, 8);
        }
        Meshes_LightPoints(reinterpret_cast<u8 *>((uintptr_t)global(0x0087a248)), nullptr, colours, frame.normals, frame.points, (u32)cols);
        header = at<const u8 *>(tex, 0);
        z = (float)(X87Reg(at<float>(header, 0x14)) + z);
        colours += 0x80;
    }
    at<u32>(tex, 0x12968) = global(0x008bd50c);
}
}
