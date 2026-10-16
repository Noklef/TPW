// Small mapping functions (switches over a kind or a mode): reimplementation of 0x4d3220, 0x4d3390, 0x4d3500, 0x4d3670, 0x485ec0, 0x506490 and 0x5064f0. Named by their
// addresses until the callers are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4d3220 / 0x4d3390 / 0x4d3500 / 0x4d3670 (thiscall on an object): the id for the kind in the dword at +0 (1 to 0x21): 0xf7 / 0x118 / 0x139 / 0x15a plus the same scrambled
// offsets 0 to 0x20; 0x17f for 0 and anything above 0x21.
u32 __fastcall Kind_ToIdA(const u32 *object);
u32 __fastcall Kind_ToIdB(const u32 *object);
u32 __fastcall Kind_ToIdC(const u32 *object);
u32 __fastcall Kind_ToIdD(const u32 *object);

// 0x485ec0 (cdecl; x): 0 for 0 and 1, 1 for 2, 2 for 3, 3 for 4; 9 for anything else.
u32 __cdecl Index_ToCode485ec0(u32 x);

// 0x506490 (thiscall on an object): the byte at +2 as 4 to 8 gives 1, 0, 2, 3, 4; anything else 5 (the debug message of the original is gone).
u32 __fastcall Byte2_ToIndex(const u8 *object);

// 0x5064f0 (thiscall on an object): the dword at +0x19c as 0 to 7 gives 0, 1, 3, 3, 4, 4, 2, 5; anything else 2.
u32 __fastcall Mode19c_ToIndex(const u8 *object);

}  // extern "C"
