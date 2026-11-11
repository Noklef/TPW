#include "defids.h"

#include <cstdint>
#include "../world/objectdefs.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

const u8 *ObjectDef_FindById(u32 id) {
    if (id == 0x160 || id == 0x15f)
        return nullptr;
    const u32 byIndex = (u32)(uintptr_t)&gObjectdefs_76e300 + id * 0x38;
    if (global<u32>(byIndex + 4) == id)
        return reinterpret_cast<const u8 *>((uintptr_t)byIndex);
    for (u32 p = ((u32)(uintptr_t)&gObjectdefs_76e300 + 0x4); p < ((u32)(uintptr_t)&gObjectdefs_76e300 + 0x4ccc); p += 0x38) {
        if (global<u32>(p) == id)
            return reinterpret_cast<const u8 *>((uintptr_t)byIndex);
    }
    return nullptr;
}
