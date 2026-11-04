// The dial-up connection let go: see dialuphangup.h.
#include "dialuphangup.h"

#include <cstdint>

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
void hangUp(u32 handle) { global<u32(__fastcall *)(void *, void *, u32)>(0x006fd5b8)(reinterpret_cast<void *>((uintptr_t)0x007c24c0), nullptr, handle); }
}  // namespace

extern "C" {

u32 __cdecl Dialup_HangUpIfBusy(u32 handle) {
    const u32 state = global<u32>(0x007cc538);
    if (state == 1 || state == 2) hangUp(handle);
    return global<u32>(0x007cc538) == 0;
}

void __cdecl Dialup_HangUpIfOpen() {
    if (!global<u8>(0x007cc540)) return;
    hangUp(global<u32>(0x007cc50c));
    global<u8>(0x007cc540) = 0;
}
}
