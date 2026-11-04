// The C++ library's num_put::do_put of a pointer, of char and of wchar_t (MSVC6).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x694e60 / 0x69bb70 (pops 0x18; this not read): sprintf "%p", then the text padded with fill to the stream's width (+0x14 of ios; on the right when its flags & 0x1c0 are 0x40), through a stream buffer iterator { failed, buffer }, returned at out. Width cleared. A character that cannot be put: failed's low byte set. Wide: each byte by btowc.
u32 *__fastcall NumPut_DoPutPointer(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
u32 *__fastcall NumPutW_DoPutPointer(void *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, u32 fill, u32 value);
}
