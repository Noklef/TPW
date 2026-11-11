// The dial-up connection let go: see dialuphangup.h.
#include "dialuphangup.h"

#include <cstdint>
#include "dialog7cc5f4dial.h"
#include "dialog7cc534handler.h"
#include "dialog7cc530open.h"
#include "../core/imports.h"

namespace {
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }
void hangUp(u32 handle) { global<u32(__fastcall *)(void *, void *, u32)>(IMPORT(Wearasr_Dialup_Disconnect))(reinterpret_cast<void *>((uintptr_t)0x007c24c0), nullptr, handle); }
}  // namespace

extern "C" {

u32 __cdecl Dialup_HangUpIfBusy(u32 handle) {
    const u32 state = gDialog7cc534handler_7cc538;
    if (state == 1 || state == 2) hangUp(handle);
    return gDialog7cc534handler_7cc538 == 0;
}

void __cdecl Dialup_HangUpIfOpen() {
    if (!gDialog7cc530open_7cc540) return;
    hangUp(gDialog7cc5f4dial_7cc50c);
    gDialog7cc530open_7cc540 = 0;
}
}
