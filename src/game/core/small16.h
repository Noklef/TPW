// A tracked handle that expires: reimplementation of 0x4e0220. Named by what it does until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4e0220 (thiscall; out, pops 4): the word through out = the handle at +0x5e (0 for none). While the game's clock (the dword at +0x1da70c of the object
// [0x80239c]) is past the stamp at +0x60 plus 100 the handle is kept only if the thing it names (the table at 0x7cfb90) passes Person_HasWord (small9) with
// this object's own handle (the word at +0); otherwise +0x5e and +0x60 are cleared and 0 is stored. Returns out.
u16 *__fastcall Obj_GetTrackedHandle(u8 *object, void *, u16 *out);

}  // extern "C"
