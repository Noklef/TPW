#include "sync.h"

#include <cstdint>

namespace {

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }

using OnObject = u32(__stdcall *)(void *);

}  // namespace

extern "C" {

u8 *__fastcall CritSec_Ctor(u8 *section) {
    Import<OnObject>(0x006fd144)(section);
    return section;
}

void __fastcall CritSec_Dtor(u8 *section) { Import<OnObject>(0x006fd0d0)(section); }

u32 __fastcall CritSec_Enter(u8 *section) {
    Import<OnObject>(0x006fd140)(section);
    return 0;
}

u32 __fastcall CritSec_Leave(u8 *section) {
    Import<OnObject>(0x006fd130)(section);
    return 0;
}

u32 *__fastcall SyncEvent_Ctor(u32 *event) {
    using Create = u32(__stdcall *)(void *, s32, s32, const char *);
    *event = Import<Create>(0x006fd0d4)(nullptr, 0, 0, nullptr);
    return event;
}

bool __fastcall SyncEvent_Wait(const u32 *event, void *, s32 timeout) {
    using Wait = u32(__stdcall *)(u32, u32);
    return Import<Wait>(0x006fd128)(*event, timeout < 0 ? 0xffffffffu : static_cast<u32>(timeout)) == 0;
}

u32 __fastcall SyncEvent_Set(const u32 *event) {
    using Set = u32(__stdcall *)(u32);
    Import<Set>(0x006fd124)(*event);
    return 0;
}
}  // extern "C"
