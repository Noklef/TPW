// The C++ library's time_put (MSVC6): do_put of char and wchar_t. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6ab160, 0x6a4540 (pops 0x1c): time_put<char> / <wchar_t>::do_put(dest { failed byte, streambuf }, ios (unused), tm, format, modifier): "!%" then the
// modifier (if any) and the format, through _Strftime with the facet's names (+8) into a local string grown by 16, 32, 64... until it answers; what it
// wrote, less the leading '!', to dest (a wide one widened by btowc first). No streambuf or an end: failed set. Returns dest at out.
u32 *__fastcall TimePut_DoPut(u8 *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, const s32 *tm, u32 format, u32 modifier);
u32 *__fastcall WTimePut_DoPut(u8 *self, void *, u32 *out, u32 failed, u8 *buffer, u8 *ios, const s32 *tm, u32 format, u32 modifier);
}
