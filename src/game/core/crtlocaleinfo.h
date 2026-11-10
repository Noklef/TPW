// The MSVC6 C library's locale information calls: __crtGetLocaleInfoW, __crtGetLocaleInfoA and __getlocaleinfo.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x691680: GetLocaleInfoW where it works (the choice kept in 0xfb1944, none: 0), else GetLocaleInfoA and MultiByteToWideChar on code_page (0: the locale's, 0xfb1450). Another kept choice is returned.
s32 __cdecl CRT_GetLocaleInfoW(u32 locale, u32 type, u16 *buffer, s32 size, u32 code_page);
// 0x6917b0: GetLocaleInfoA where GetLocaleInfoW does not work (the choice kept in 0xfb1948), else GetLocaleInfoW and WideCharToMultiByte.
s32 __cdecl CRT_GetLocaleInfoA(u32 locale, u32 type, char *buffer, s32 size, u32 code_page);
// 0x691380: kind 1: the text, in a new block at *out: from a 0x80-byte frame buffer, or a block when ERROR_INSUFFICIENT_BUFFER. Kind 0: up to 4 digits (from the static 0xfb1938) into the byte at out, stopping at a non-digit. 0, or -1 on failure and for other kinds.
s32 __cdecl CRT_Getlocaleinfo(s32 kind, u32 locale, u32 type, void *out);
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u16[4], CRT_Getlocaleinfo_buffer_fb1938, 0x00fb1938, 0x8);
GAME_DATA(u32, CRT_GetLocaleInfoW_use_fb1944, 0x00fb1944, 0x4);
GAME_DATA(u32, CRT_GetLocaleInfoA_use_fb1948, 0x00fb1948, 0x4);
