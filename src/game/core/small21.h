// Small helpers: a freed-if-set pointer, a dword fill, a constant, two wrappers of the keyboard maps and the global-memory wrappers that count or log:
// reimplementation of 0x598760, 0x598770, 0x5f4ea0, 0x486b60, 0x486b70, 0x51b4a0, 0x51b4d0, 0x51f630, 0x51f660 and 0x556ab0. Named by what they do until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x598760 (thiscall on a pointer variable): the block it points to is freed when it is set; the pointer is left.
void __fastcall Ptr_FreeIfSet(u8 **ref);

// 0x598770 (cdecl; pointer, count): count dwords from the pointer are set to 0. Returns 0.
u32 __cdecl Dwords_Zero(u32 *to, u32 count);

// 0x5f4ea0: returns 1.
u32 __fastcall ReturnOne();

// 0x486b60 / 0x486b70 (cdecl): KeyMaps_EnableAll / KeyMaps_ResetAll, then the byte at 0x7c24d0 = 0 / 1.
void __cdecl KeyMaps_EnableAllAndClearFlag();
void __cdecl KeyMaps_ResetAllAndSetFlag();

// 0x51b4a0 / 0x51b4d0 (cdecl; size / block): 0 when the argument is 0; otherwise the dword at 0x802bd8 is raised / lowered and the global memory of the file layer is
// allocated (Mem_GlobalAlloc) / freed (Mem_GlobalFree); returns what that returns.
u32 __cdecl Sound_Alloc(u32 size);
u32 __cdecl Sound_Free(void *block);

// 0x51f630 / 0x556ab0 / 0x51f660 (cdecl; size / size / block): Mem_GlobalAlloc / Mem_GlobalAlloc / Mem_GlobalFree when the argument is not 0 (the debug output of the
// others is gone), otherwise 0.
u32 __cdecl Mem_AllocChecked(u32 size);
u32 __cdecl Mem_AllocLogged(u32 size);
u32 __cdecl Mem_FreeChecked(void *block);

}  // extern "C"
