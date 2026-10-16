// Base64 text: reimplementation of 0x5c92f0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5c92f0 (cdecl; dst, src, length): the source bytes written as base64 text with the table at 0x773d80,
// '=' padding, a CR LF after every 76 characters and a terminating zero. Nothing at all (not even the
// terminator) for a null destination or source; a length of 0 or below gives just the terminator.
void __cdecl Base64_Encode(char *dst, const u8 *src, s32 length);
}
