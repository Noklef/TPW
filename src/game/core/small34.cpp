#include "small34.h"

#include <cstdint>

#include "../coaster/queue.h"
#include "../render/mode.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

}  // namespace

extern "C" {

u32 __cdecl Graphics_Level() {
    if (RenderMode_Size() == 0x4000) {
        const u32 depth = RenderMode_Depth();
        if (depth == 8) return 8;
        if (depth == 0x10) return 6;
        return depth == 0x20 ? 4 : 0;
    }
    switch (*reinterpret_cast<u32 *>(0x007858d0)) {
    case 0: return 2;
    case 1: return 4;
    case 2: return 6;
    default: return 8;
    }
}

void __cdecl Coaster_SetTrainCount(u8 *coaster, u32 count) {
    if (count == 0) return;
    if ((at<u8>(coaster, 8) & 0xc) != 0) {
        for (u32 i = count; i < at<u32>(coaster, 0xf4); ++i) {
            u8 *const train = at<u8 *>(coaster, 0x128) + (i << 7);
            CoasterTrain_Unload(coaster, train, 0);
            CoasterTrain_Unload(coaster, train, 1);
        }
    }
    at<u32>(coaster, 0xf4) = count;
}

}  // extern "C"
