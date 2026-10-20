// The ten message tags' windows (the 0x208-byte records from 0x7c26b0: +0 the window, +4 a word, +6 their text): reimplementation of
// 0x48ea00 and 0x48eaa0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48ea00 (cdecl (id)): a window of 0x138 bytes (GameHeap_Alloc; 0 when that fails) built (Window_Ctor: no parent, flags 0, the id, the
// rectangle (0, 0, 70, 65)), given the object of the GUI window registered as "msgtag" (GuiWindows_FindByName; Window_SetObject98), the
// handler 0x48dae0 (Window_SetHandler) and +0x90 = 0x1db. Returns the window or 0.
u8 *__cdecl MsgTag_NewWindow(u32 id);

// 0x48eaa0 (cdecl): for each of the ten records (i 0-9) the word +4 = 0, the window +0 = MsgTag_NewWindow(900 + i) and, when there is
// one, its dword 2 (Window_SetDwordA0) the address of the record's +6.
void __cdecl MsgTags_Create();
}
