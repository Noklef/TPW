// Sweeps over the things that call a function for each object of a kind: reimplementation of 0x482040, 0x482100, 0x4821b0, 0x482270, 0x482340 and 0x482400 (cdecl). Each
// walks the things newest first (the world at [0x7cf83c]), considers the objects (model byte at +2 is 3) that pass a test and calls the callback (cdecl, the object as its
// argument) for each; returns how many it called it for. The tests are on the definition word at +0x284 (Object_DefWord284), the definition (Object_GetDef) and the
// object's flag byte at +0x32.
#pragma once

#include "../../../re/types.h"

typedef void(__cdecl *ObjectCallback)(u8 *object);

extern "C" {

// 0x482040 (kind, callback): the word is 0 and the dword at +0x4ac of the definition is the kind.
s32 __cdecl Objects_ForEachOfKind(u32 kind, ObjectCallback callback);

// 0x482100 (callback): the word is not 0.
s32 __cdecl Objects_ForEachWordSet(ObjectCallback callback);

// 0x4821b0 (callback): the word is not 0 and the dword at +0x288 of the definition is 2.
s32 __cdecl Objects_ForEachUpgrade2(ObjectCallback callback);

// 0x482270 / 0x482340 / 0x482400 (callback): the word is 0, the definition's +0x4ac is 3 and the flag byte has bit 0 / neither of bits 0 and 1 / bit 1.
s32 __cdecl Objects_ForEachKind3Flag1(ObjectCallback callback);
s32 __cdecl Objects_ForEachKind3NoFlags(ObjectCallback callback);
s32 __cdecl Objects_ForEachKind3Flag2(ObjectCallback callback);

}  // extern "C"
