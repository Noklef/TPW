// The C++ library's collate<char>::do_transform (MSVC6; its name from its shape and vtable slot 2 of 0x709fc4, unconfirmed). wcollate.h has the wchar_t one.
#pragma once

#include "../../../re/types.h"
#include "stdstring.h"

extern "C" {

// 0x6a7080 (pops 0xc; this not read): the string at out made from the characters first to last, by way of a local string grown with zeros until they fit. out's allocator byte is the
// local's, which the original sets from the low byte of first's stack slot.
StdString *__fastcall Collate_DoTransform(void *self, void *, StdString *out, const char *first, const char *last);
}
