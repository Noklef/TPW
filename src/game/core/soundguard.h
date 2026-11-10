// The holders the sound classes' exception cleanups (0x6e5xxx) destroy. Reimplementation of 0x6b70b0, 0x6b70c0, 0x6bafd0, 0x6bafe0, 0x6bb0c0 and 0x6bb210; the names are invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b70b0 (thiscall; called from 0x6e5781, 0x6e57b1): the object at +0 (unless null): its slot 2 (3).
void __fastcall SoundGuard_DropSlot2(u8 **holder);

// 0x6b70c0 (thiscall; called from 0x6e578c and others): the block at +0 freed.
void __fastcall SoundGuard_FreeBlock(u8 **holder);

// 0x6bafd0 (thiscall; called from 0x6e5af6, 0x6e5b16): the object at +0 (unless null): its slot 0 (3).
void __fastcall SoundGuard_DeleteArray(u8 **holder);

// 0x6bafe0 (thiscall; called from 0x6e5a16 and others): the object at +0 (unless null): its slot 0 (1).
void __fastcall SoundGuard_Delete(u8 **holder);

// 0x6bb210 (thiscall; called from 0x6e5b42 and others): with byte +4 set, the object at +0: its slot 3.
void __fastcall SoundGuard_Release(u8 **holder);

// 0x6bb0c0 (thiscall; reached from 0x6e5c19, 0x6e5c49 on an object's +0x6c, the voice's handle): CloseHandle of the handle at +0; its answer.
u32 __fastcall SoundGuard_CloseHandle(u32 *holder);

}  // extern "C"
