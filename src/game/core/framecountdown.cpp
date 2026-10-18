#include "framecountdown.h"

#include <cstdint>

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Countdown7cc150_Step() {
    s32 &counter = global<s32>(0x007cc150);
    if (counter > 0)
        counter--;
    return counter == 0 && global<u32>(0x007cc190) != 0 ? 1 : 0;
}
}  // extern "C"
