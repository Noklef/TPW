// The C library's multibyte strings (MSVC6's mbspbrk.c, mbsrchr.c, mbctolower.c), over the code page's tables: _mbctype at 0xfb1668 (bit 4: a lead byte), _mbcasemap at 0xfb1770, __mbcodepage 0xfb1870.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6895b0: _mbctolower: over 0xff, the two bytes (first the high one) mapped by LCMapString (flags 0x100) when the high one is a lead byte; a byte of an upper case (bit 0x10) by _mbcasemap. Else as it is.
u32 __cdecl CRT_mbctolower(u32 c);

// 0x689250: _mbsicmp: no code page: _stricmp. Else, under lock 0x19, characters through LCMapString (flags 0x200) for lead bytes (one that ends the text: as an end; a map of other than 1 or 2
// bytes: 0x7fffffff) or _mbcasemap for a byte of an upper case (bit 0x10). -1, 0 or 1.
s32 __cdecl CRT_mbsicmp(const char *a, const char *b);

// 0x689e90: getSystemCP: -2 GetOEMCP's, -3 GetACP's, -4 the code page of [0xfb1450]; 0xfb1884 set for those, cleared for the code page itself, answered.
u32 __cdecl CRT_getSystemCP(u32 code_page);
// 0x689ee0: CPtoLCID: 932, 936, 949, 950's locales, else 0.
u32 __cdecl CRT_CPtoLCID(u32 code_page);
// 0x689f40: setSBCS: the table 0xfb1668 (0x101 bytes) and six globals cleared. Answers 0.
u32 __cdecl CRT_setSBCS();
// 0x689f80: setSBUpLow: with GetCPInfo of the code page [0xfb1870] (1): the bytes (lead byte ranges as blanks, 0 as a blank) through __crtGetStringTypeA and __crtLCMapStringA
// (lower 0x100, upper 0x200; locale [0xfb1874]): an upper case byte gets bit 0x10 and its lower case in _mbcasemap 0xfb1770, a lower case one 0x20 and its upper case, others 0.
// Without: the same for A-Z and a-z.
void __cdecl CRT_setSBUpLow();
// 0x689c60: _setmbcp: under lock 0x19 the code page getSystemCP gives: the one in use: 0; 0: setSBCS. One of the five in the table 0x781590 (0x30 bytes each): its four
// lists of ranges given the bits 0x781588, its three words; else GetCPInfo's lead byte ranges (4) and, with them, 8 for bytes 1-0xfe; no info: setSBCS when [0xfb1884], else -1.
// Then setSBUpLow, 0.
s32 __cdecl CRT_setmbcp(s32 page);
// 0x68a160: __initmbctable: _setmbcp(-3) (the ANSI code page), its answer.
s32 __cdecl CRT_initmbctable();
// 0x689720: x_ismbbtype: 1 when the byte's entry in 0xfb1669 meets mask, or its _ctype word (0x77ea9a) meets ctype_mask; else 0.
u32 __cdecl CRT_ismbbtype(u32 c, u32 ctype_mask, u32 mask);
u32 __cdecl CRT_ismbblead(u32 c);   // 0x689700

// 0x689650: _mbspbrk: no code page: strpbrk. Else, under lock 0x19, the first character of the text in the set. A set that ends in a lead byte stops at the first character looked at (sic).
char *__cdecl CRT_mbspbrk(const char *text, const char *set);

// 0x689400: _mbsrchr: no code page: strrchr. Else, under lock 0x19, the last character c of the text. A lead byte before the end counts as found at the end, when nothing else was (sic).
char *__cdecl CRT_mbsrchr(const char *text, u32 c);
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u8[0xf8], gCRT_setmbcp_781588, 0x00781588, 0xf8);   // (__rgctypeflag, 4 bytes of padding, then 5 code pages of 0x30 bytes)
GAME_DATA(u8[0x101], CRT_mbctype, 0x00fb1668, 0x101);
GAME_DATA(u8[0x100], CRT_mbcasemap, 0x00fb1770, 0x100);
GAME_DATA(u32, CRT_mbcodepage, 0x00fb1870, 0x4);
GAME_DATA(u32, CRT_mblcid, 0x00fb1874, 0x4);
GAME_DATA(u32[3], CRT_mbulinfo, 0x00fb1878, 0xc);
GAME_DATA(u32, CRT_mb_system_set_fb1884, 0x00fb1884, 0x4);
GAME_DATA(u32, CRT_ismbcodepage, 0x00fb3b84, 0x4);
