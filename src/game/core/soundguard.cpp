// The holders the sound classes' exception cleanups destroy: see soundguard.h.
#include "soundguard.h"

#include <cstdint>

#include "gameheap.h"
#include "vcall.h"
#include "imports.h"

extern "C" {

void __fastcall SoundGuard_DropSlot2(u8 **holder) {
    if (*holder) vcall<u32>(*holder, 8, 3u);
}

void __fastcall SoundGuard_FreeBlock(u8 **holder) { GameHeap_Free(*holder); }

void __fastcall SoundGuard_DeleteArray(u8 **holder) {
    if (*holder) vcall<u32>(*holder, 0, 3u);
}

void __fastcall SoundGuard_Delete(u8 **holder) {
    if (*holder) vcall<u32>(*holder, 0, 1u);
}

void __fastcall SoundGuard_Release(u8 **holder) {
    if (reinterpret_cast<u8 *>(holder)[4]) vcall<u32>(*holder, 0xc);
}

u32 __fastcall SoundGuard_CloseHandle(u32 *holder) { return (*reinterpret_cast<u32(__stdcall *const *)(u32)>((uintptr_t)IMPORT(Kernel32_CloseHandle)))(*holder); }

}  // extern "C"
