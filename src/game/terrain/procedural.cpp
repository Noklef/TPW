#include "procedural.h"

#include <cstdint>
#include <initializer_list>

#include "../core/gameheap.h"
#include "../core/thunks.h"
#include "../render/engineerrors.h"
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
}
