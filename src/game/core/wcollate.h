// The C++ library's collate<wchar_t>::do_transform (MSVC6; its name from its shape and vtable slot, unconfirmed).
#pragma once

#include "../../../re/types.h"
#include "stdwstring.h"

extern "C" {

// 0x69ffe0 (pops 0xc; this not read): the wide string at out made from the characters begin to end, by way of a local string. The original's out allocator byte is one uninitialized byte of its stack: here 0.
StdWString *__fastcall CollateW_DoTransform(void *self, void *, StdWString *out, const u16 *begin, const u16 *end);
}
