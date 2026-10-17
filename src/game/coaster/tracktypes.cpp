#include "tracktypes.h"

#include <cstdint>

#include "../core/vwrap.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

s32 __cdecl TrackTypeSlot_ReadIntoFree(u8 *stream) {
    if (global<u32>(0x00816d40) == 0)
        return -1;
    s32 index = 0;
    for (u32 slot = 0x00804488; slot != 0 && global<u8>(slot) != 0;) {
        slot += 0x140;
        index++;
        if ((s32)slot > 0x80c7c7)
            return -1;
    }
    Object_CallSlot8Product(0x00804370 + (u32)index * 0x140, 0x140, 1, stream);
    return index;
}

}  // extern "C"
