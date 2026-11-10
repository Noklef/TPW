#include "sync.h"

#include <cstdint>
#include "imports.h"

namespace {

template <typename F> F Import(u32 slot) { return *reinterpret_cast<const F *>((uintptr_t)slot); }

using OnObject = u32(__stdcall *)(void *);

}  // namespace

extern "C" {

u8 *__fastcall CritSec_Ctor(u8 *section) {
    Import<OnObject>(IMPORT(Kernel32_InitializeCriticalSection))(section);
    return section;
}

void __fastcall CritSec_Dtor(u8 *section) { Import<OnObject>(IMPORT(Kernel32_DeleteCriticalSection))(section); }

u32 __fastcall CritSec_Enter(u8 *section) {
    Import<OnObject>(IMPORT(Kernel32_EnterCriticalSection))(section);
    return 0;
}

u32 __fastcall CritSec_Leave(u8 *section) {
    Import<OnObject>(IMPORT(Kernel32_LeaveCriticalSection))(section);
    return 0;
}

u32 *__fastcall SyncEvent_Ctor(u32 *event) {
    using Create = u32(__stdcall *)(void *, s32, s32, const char *);
    *event = Import<Create>(IMPORT(Kernel32_CreateEventA))(nullptr, 0, 0, nullptr);
    return event;
}

bool __fastcall SyncEvent_Wait(const u32 *event, void *, s32 timeout) {
    using Wait = u32(__stdcall *)(u32, u32);
    return Import<Wait>(IMPORT(Kernel32_WaitForSingleObject))(*event, timeout < 0 ? 0xffffffffu : static_cast<u32>(timeout)) == 0;
}

u32 __fastcall SyncEvent_Set(const u32 *event) {
    using Set = u32(__stdcall *)(u32);
    Import<Set>(IMPORT(Kernel32_SetEvent))(*event);
    return 0;
}
}  // extern "C"
