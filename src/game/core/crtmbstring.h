// The C library's multibyte strings (MSVC6's mbspbrk.c, mbsrchr.c, mbctolower.c), over the code page's tables: _mbctype at 0xfb1668 (bit 4: a lead byte), _mbcasemap at 0xfb1770, __mbcodepage 0xfb1870.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6895b0: _mbctolower: over 0xff, the two bytes (first the high one) mapped by LCMapString (flags 0x100) when the high one is a lead byte; a byte of an upper case (bit 0x10) by _mbcasemap. Else as it is.
u32 __cdecl CRT_mbctolower(u32 c);

// 0x689250: _mbsicmp: no code page: _stricmp. Else, under lock 0x19, characters through LCMapString (flags 0x200) for lead bytes (one that ends the text: as an end; a map of other than 1 or 2
// bytes: 0x7fffffff) or _mbcasemap for a byte of an upper case (bit 0x10). -1, 0 or 1.
s32 __cdecl CRT_mbsicmp(const char *a, const char *b);

// 0x689650: _mbspbrk: no code page: strpbrk. Else, under lock 0x19, the first character of the text in the set. A set that ends in a lead byte stops at the first character looked at (sic).
char *__cdecl CRT_mbspbrk(const char *text, const char *set);

// 0x689400: _mbsrchr: no code page: strrchr. Else, under lock 0x19, the last character c of the text. A lead byte before the end counts as found at the end, when nothing else was (sic).
char *__cdecl CRT_mbsrchr(const char *text, u32 c);
}
