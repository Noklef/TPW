#include "objmodel.h"

#include <cstdint>

#include "../core/gameheap.h"
#include "../core/vobj2.h"
#include "../gfx/texset.h"
#include "../terrain/procedural.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) {
    return *reinterpret_cast<T *>((uintptr_t)p + offset);
}
u8 *ptr(const void *p, u32 offset) { return reinterpret_cast<u8 *>((uintptr_t)at<u32>(p, offset)); }

}  // namespace

extern "C" {

u32 __fastcall ObjModel_Unload(u8 *object) {
    if (ptr(object, 8)) {
        if (*reinterpret_cast<const u8 *>(0x008bd550) & 0x20) {
            u8 *mesh = ptr(ptr(object, 8), 0x70);
            for (s32 i = 0; i < (s32)at<u16>(ptr(object, 8), 0x44); ++i, mesh += 0xa0) {
                if (at<u32>(mesh, 0) & 0x200) continue;
                s32 j = (s32)at<u16>(mesh, 0x5a) - 1;
                if (j >= 0) {
                    const u8 *face = ptr(mesh, 0x6c) + 0x10 * j;
                    for (; j >= 0; --j, face -= 0x10) {
                        const u8 *const p = ptr(face, 0);
                        if (p && (at<u16>(p, 0) & 0x1000)) break;
                    }
                }
                if (j >= 0) TrackedGlobal_Free(at<u32>(mesh, 0x74));
            }
        }
        Object_ReleaseSavedPairs130(object);
        Object_ReleaseSavedPairs(object);
        Object_FreeBlock13c(object);
        for (s32 i = 0; i < (s32)at<u16>(ptr(object, 8), 0x36); ++i) {
            u8 *const set = ptr(ptr(ptr(object, 8), 0x50), 8 * i + 4);
            if (set && TexSet_Release(set)) {
                GameHeap_Free(ptr(ptr(ptr(object, 8), 0x50), 8 * i + 4));
                at<u32>(ptr(ptr(object, 8), 0x50), 8 * i + 4) = 0;
            }
        }
        TrackedGlobal_Free(at<u32>(object, 0x144));
        const u8 *model = ptr(object, 8);
        at<u32>(object, 0x148) = 0;
        at<u32>(object, 0x144) = 0;
        for (s32 i = 0; i < (s32)at<u16>(model, 0x44); ++i) {
            at<u32>(ptr(model, 0x70), 0xa0 * i + 0x9c) = 0;
            model = ptr(object, 8);
        }
        if ((at<u8>(ptr(object, 8), 0x30) & 2) && *reinterpret_cast<u8 *const *>(0x00879d60))
            TerrainTex_Unload(*reinterpret_cast<u8 *const *>(0x00879d60));
    }
    const u32 flags = at<u32>(object, 4);
    at<u32>(object, 8) = 0;
    at<u32>(object, 4) = flags & ~0x10000000u;
    return 1;
}
}
