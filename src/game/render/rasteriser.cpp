#include "rasteriser.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __stdcall Rasteriser_ChooseSpan(u32 flags) {
    u32 routine;
    if (flags & 0x100) {
        if (flags & 0x200000) {
            routine = global<u32>(0x006f19f8);
        } else {
            global<u32>(0x00fbe010) = (((flags & 0x200) ? 0xe0 : 0) + 0x10) >> 2;
            routine = (flags & 0x800) ? global<u32>(0x006f19f4) : global<u32>(0x006f1a20);
        }
    } else if (flags & 0x10000000) {
        if (flags & 0x20000000)
            routine = global<u32>(0x006f1a1c);
        else if (flags & 0x40000000)
            routine = global<u32>(0x006f1a18);
        else
            routine = global<u32>(0x006f1a14);
    } else {
        routine = global<u32>(0x006f19f0);
    }
    global<u32>(0x00fbe044) = routine;
    return 1;
}
}
