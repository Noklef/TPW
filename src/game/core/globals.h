// Getters, setters and releases of the game's globals. Reimplementation of 0x47eaa0, 0x4b0aa0, 0x4f21a0, 0x4f7970, 0x4fe980, 0x519410, 0x519420, 0x519430, 0x519440, 0x51fd20, 0x52f1f0, 0x52f840, 0x52f850, 0x52f860 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x57aca0 / 0x57cb30 (cdecl): the dword at 0x8bb628 / 0x8bba78 = 1.0f. Both return 1.0f's bits.
u32 __cdecl Global8bb628_SetOne();

u32 __cdecl Global8bba78_SetOne();

// 0x57cb70 (cdecl): 0x8bcb8c = 1.0f and the dwords at 0x8bcb80, 0x8bcb84 and 0x8bcb88 = 1. Returns 1.
u32 __cdecl Globals8bcb80_SetOne();

// 0x4f21a0 (thiscall on a dword variable): the variable is given the next id (the dword at 0xfb1dd4 raised) when it is 0. Returns it.
u32 __fastcall Id_Get(u32 *id);

// 0x4f7970 / 0x5d15d0 (cdecl): the counted object the dword at 0x7ced44 / 0xf827dc points to loses a use (as Counted_Release) and is deleted (slot 0 with 1) when it has none left;
// the dword is cleared. (A null dword faults.)
void __cdecl Global7ced44_Release();

void __cdecl Globalf827dc_Release();

// 0x5633a0 (cdecl; returns 1 or 0): when the object the dword at 0x879a24 points to exists its seven COM members are released (twice, as the original does), it is freed and
// the dword cleared.
u32 __cdecl Global879a24_Free();

// 0x51fd20 (cdecl): when the dword at 0x816d40 is not 0: the dword at 0x80ced0 is cleared, the blocks the dwords at 0x80cef8 and 0x816d38 point to are freed (Mem_FreeChecked)
// and the dword at 0x816d40 cleared.
void __cdecl Globals816d40_Free();

// 0x4b0aa0 (thiscall): the global game object (the dword at 0x7c2658) is given the value at +0x28 (Set198_4e21e0), then the signed byte at +0x24 times 50 (in the low word of
// the argument; the upper half is the value just returned, as the original leaves it) as its quality of goods, then the value at +0x2c (Obj_Set194).
void __fastcall Obj_ApplyToGlobalObject(u8 *object);

// 0x56a750 (cdecl): the 64 pairs of dwords from 0x879ad0 and the dword at 0x879cd0 are cleared.
void __cdecl Table879ad0_Clear();

// 0x52f860 (cdecl; value): whether the dword at 0x81ae2c equals the value.
u32 __cdecl Global81ae2c_Equals(u32 value);

// 0x52fab0 / 0x52faf0 (cdecl): the dword at 0x81b0cc / 0x81ecac when the dword at 0x8187f8 is 0, else 0.
u32 __cdecl Global81b0cc_IfNoFlag();

u32 __cdecl Global81ecac_IfNoFlag();

// 0x52fba0 (cdecl; amount): the dword at 0x81af4c += the amount. Returns the amount.
u32 __cdecl Global81af4c_Add(u32 amount);

// 0x573860 (cdecl): the dword at 0x4c4 of what the dword at 0x879a24 points to.
u32 __cdecl Global879a24_Get4c4();

// 0x5d5db0 (cdecl): the dword at 0x786b80 = 1 and the dword at +0x14 of what the dword at 0xf82884 points to = 3. Returns that
// pointer.
u32 __cdecl Global786b80_SetAndMode3();

// 0x4fe980 (stdcall; index): the byte at 0x785058 / 0x78505c / 0x785060 for 0 / 1 / 2, otherwise 0 (the original reports that to its empty
// debug printf).
u8 __stdcall Bytes785058_Get(u32 index);

// 0x47eaa0: the dword at 0x7b4b88.
u32 __cdecl Global7b4b88_Get_47eaa0();

// 0x4b4480 (cdecl): the dword at 0x7cc2f0 (0, 1 or 2: set by the window messages of 0x4b37xx-0x4b3fxx).
u32 __cdecl Global7cc2f0_Get_4b4480();

// 0x519410: the dword at 0x785584.
u32 __cdecl Global785584_Get_519410();

// 0x519420: the dword at 0x785588.
u32 __cdecl Global785588_Get_519420();

// 0x519430: the dword at 0x78558c.
u32 __cdecl Global78558c_Get_519430();

// 0x519440: the dword at 0x785590.
u32 __cdecl Global785590_Get_519440();

// 0x52f1f0: the dword at 0x81d7a4.
u32 __cdecl Global81d7a4_Get_52f1f0();

// 0x52f840: the dword at 0x81ae2c.
u32 __cdecl Global81ae2c_Get_52f840();

// 0x52f850: the dword at 0x81af64.
u32 __cdecl Global81af64_Get_52f850();

// 0x52fb10: 0x81b6dc = value; returns it.
u32 __cdecl Global81b6dc_Set_52fb10(u32 v0);

// 0x52fb20: the dword at 0x81b6dc.
u32 __cdecl Global81b6dc_Get_52fb20();

// 0x52fb30: 0x81ea14 = value; returns it.
u32 __cdecl Global81ea14_Set_52fb30(u32 v0);

// 0x52fb40: the dword at 0x81ea14.
u32 __cdecl Global81ea14_Get_52fb40();

// 0x52fb50: 0x81aebc = value; returns it.
u32 __cdecl Global81aebc_Set_52fb50(u32 v0);

// 0x52fb60: the dword at 0x81aebc.
u32 __cdecl Global81aebc_Get_52fb60();

// 0x52fb70: 0x81b43c = value; returns it.
u32 __cdecl Global81b43c_Set_52fb70(u32 v0);

// 0x52fb80: the dword at 0x81b43c.
u32 __cdecl Global81b43c_Get_52fb80();

// 0x52fb90: 0x81af4c = value; returns it.
u32 __cdecl Global81af4c_Set_52fb90(u32 v0);

// 0x52fbc0: the dword at 0x81af4c.
u32 __cdecl Global81af4c_Get_52fbc0();

// 0x52fbe0: the dword at 0x820a8c.
u32 __cdecl Global820a8c_Get_52fbe0();

// 0x5508b0: 0x879088 = value; returns it.
u32 __cdecl Global879088_Set_5508b0(u32 v0);

// 0x691530: the dword at 0x781fc8.
u32 __cdecl Global781fc8_Get_691530();

// 0x6d1bc0: 0xfb2708 = value; returns it.
u32 __cdecl Globalfb2708_Set_6d1bc0(u32 v0);

// 0x561a00: frees the block the dword at 0x879a20 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal879a20_561a00();

// 0x56b760: frees the block the dword at 0x879d54 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal879d54_56b760();

// 0x5740e0: frees the block the dword at 0x87a248 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal87a248_5740e0();

// 0x580500: frees the block the dword at 0x8bcbb8 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal8bcbb8_580500();

// 0x58b040: frees the block the dword at 0x8c47d4 points to (if any), clears it and returns 1 (0 when there was none).
u32 __cdecl FreeGlobal8c47d4_58b040();
}  // extern "C"
