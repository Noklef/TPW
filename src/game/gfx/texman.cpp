#include "texman.h"

#include <cstdint>

#include "texture.h"
#include "../core/gameheap.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl TexManager_Destroy() {
    u8 *const manager = global<u8 *>(0x0087a268);
    if (!manager)
        return 0;
    if (at<s32>(manager, 0x10000) > 0) {
        s32 i = 0;
        do {
            u8 *const slot = manager + 8 + 0x10 * i;
            if (u8 *const textures = at<u8 *>(slot, 0)) {
                // (the original first walks to the last texture chained by 0x800000 and does nothing with it)
                Texture_Release(textures);
                if (u8 *const array = at<u8 *>(slot, 0))
                    Texture_DestroyArray(array);
                at<u32>(slot, 0) = 0;
            }
            at<u32>(slot, 4) = 0;
        } while (++i < at<s32>(manager, 0x10000));
    }
    at<u32>(manager, 0x10000) = 0;
    GameHeap_Free(global<u8 *>(0x0087a268));
    global<u32>(0x0087a268) = 0;
    return 1;
}

void __fastcall TexManager_SetLoadFlags(u8 *manager, void *, u32 flags) { at<u32>(manager, 0x10008) = flags; }
}
