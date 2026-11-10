// The C library's category init routines that setlocale calls (MSVC6's initmon.c and initctype.c).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x68d6b0: _get_lc_lconv: the monetary texts (+0xc to +0x24, new blocks; the grouping fixed) and digits (+0x28 to +0x2f) of the
// LC_MONETARY locale [0xfb18cc] into lconv: the results ORed; -1 for a null lconv.
s32 __cdecl CRT_get_lc_lconv(u32 *lconv);
// 0x68d5c0: __init_monetary: the "C" locale ([0xfb1444] 0): the static lconv 0x781f98; otherwise a new one (calloc) from _get_lc_lconv.
// Its first three fields come from the current [0x781fc8], which then points to it; the one owned before [0xfb18ac] freed. 1: failed.
s32 __cdecl CRT_init_monetary();
// 0x68d8b0: __init_ctype: the "C" locale ([0xfb1440] 0): _pctype and _pwctype back to the static table 0x77ea9a; otherwise new tables
// from GetStringTypeA and W over the code page [0xfb1450] (asked for when 0), lead bytes 0x8000, __mb_cur_max [0x77e7dc] its max size.
// 1: failed (the blocks freed).
s32 __cdecl CRT_init_ctype();
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u32 *, CRT_monetary_owned_fb18ac, 0x00fb18ac, 0x4);
GAME_DATA(void *, CRT_ctype_owned_fb18b0, 0x00fb18b0, 0x4);
GAME_DATA(void *, CRT_ctype_wide_owned_fb18b4, 0x00fb18b4, 0x4);
