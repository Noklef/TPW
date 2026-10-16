// The sign textures of a model: reimplementation of 0x468950. Named by what it does until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x468950 (cdecl; model, out1, out2): over the model's entries (the word at +0x36 of them, 16 bytes each from the pointer at +0x54): the entry that has flags 2 and 8, or is
// named "sign1.tga" (the pointer at +0xc of the entry, compared case blind), sets *out1 = the dword at +0x50 of the model + (the entry's index * 8); flags 2 and 0x10, or the
// name "sign2.tga", the same for *out2. Later entries override earlier ones.
void __cdecl Model_FindSignEntries(u8 *model, u32 *out1, u32 *out2);

}  // extern "C"
