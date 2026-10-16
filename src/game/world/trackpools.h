// The track rides' two record pools: the buoys (0x400 records of 0x1c bytes from [0x877b78], [0x877b7c] counting the ones in use) and the
// collision nodes (0x800 records of 0x20 bytes from [0x877b70], [0x877b74] counting them): reimplementation of 0x54b130, 0x54b170, 0x54b1e0 and
// 0x54b210. Bit 0 of a record's first byte marks it in use. The "Out of ..." messages the original sends to its empty debug printf are left out.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x54b130 (cdecl): the first buoy not in use, its dword at +4 set to its index; 0 for none (or no pool).
u8 *__cdecl Buoy_FindFree();

// 0x54b170 (cdecl; a, b): that buoy taken: the count up, +0 = 1, +8 = a, +0xc = b and +0x10-+0x1b cleared (+4 is its index). 0 for none.
u8 *__cdecl Buoy_Alloc(u32 a, u32 b);

// 0x54b1e0 (cdecl): the first collision node not in use; 0 for none (or no pool).
u8 *__cdecl CollNode_FindFree();

// 0x54b210 (cdecl; a, b, c): that node taken: the count up, +0 = 0x1f, +4 = a, +8 = b, +0xc and +0x10 = c, +0x14-+0x1f cleared. 0 for none.
u8 *__cdecl CollNode_Alloc(u32 a, u32 b, u32 c);

}  // extern "C"
