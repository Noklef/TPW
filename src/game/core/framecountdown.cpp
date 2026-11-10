#include "framecountdown.h"

#include <cstdint>
#include "../ui/gamepause.h"

namespace {

template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

u32 __cdecl Countdown7cc150_Step() {
    s32 &counter = gGamepause_7cc150;
    if (counter > 0)
        counter--;
    return counter == 0 && gGamepause_7cc190 != 0 ? 1 : 0;
}
}  // extern "C"
