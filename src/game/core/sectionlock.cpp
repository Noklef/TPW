// The critical section wrappers: see sectionlock.h.
#include "sectionlock.h"

#include <cstdint>

#include "gameheap.h"
#include "imports.h"

namespace {

using OnSection = void(__stdcall *)(u32);

OnSection section_import(u32 slot) { return *reinterpret_cast<OnSection const *>(static_cast<uintptr_t>(slot)); }

}  // namespace

extern "C" {

u32 *__fastcall OwnedSection_Ctor(u32 *self) {
    const u32 block = static_cast<u32>(reinterpret_cast<uintptr_t>(GameHeap_Alloc(0x18)));
    *self = block;
    section_import(IMPORT(Kernel32_InitializeCriticalSection))(block);   // InitializeCriticalSection
    return self;
}

void __fastcall OwnedSection_Dtor(u32 *self) {
    section_import(IMPORT(Kernel32_DeleteCriticalSection))(*self);   // DeleteCriticalSection
    GameHeap_Free(reinterpret_cast<void *>(static_cast<uintptr_t>(*self)));
}

void __fastcall SectionGuard_Dtor(u32 *self) { SectionGuard_Leave(self); }

void __fastcall SectionGuard_Enter(u32 *self) {
    section_import(IMPORT(Kernel32_EnterCriticalSection))(*self);   // EnterCriticalSection
    self[1] = 1;
}

void __fastcall SectionGuard_Leave(u32 *self) {
    if (self[1] && !self[2]) {
        section_import(IMPORT(Kernel32_LeaveCriticalSection))(*self);   // LeaveCriticalSection
        self[1] = 0;
    }
}

}  // extern "C"
