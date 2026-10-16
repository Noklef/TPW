// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x492020: calls Fn_669fa8.
u8 * __fastcall Wrap_Fn_669fa8_492020(u8 *o);
// 0x492060: calls Fn_669fee.
u32 __fastcall Wrap_Fn_669fee_492060(u8 *o);
// 0x4fa460: calls Set4_4217f0.
u32 __fastcall Wrap_Set4_4217f0_4fa460(u8 *o);
// 0x6b0470: calls Vtbl70a078_Set_6b4a50.
void __fastcall Wrap_Vtbl70a078_Set_6b4a50_6b0470(u8 *o);
// 0x6b0710: calls Init_6b4a60.
u8 * __fastcall Wrap_Init_6b4a60_6b0710(u8 *o);
// 0x6b0750: calls Vtbl70a258_Set_6b4a90.
void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b0750(u8 *o);
// 0x6b1de0: calls Init_6b4a60.
u8 * __fastcall Wrap_Init_6b4a60_6b1de0(u8 *o);
// 0x6b1e20: calls Vtbl70a258_Set_6b4a90.
void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b1e20(u8 *o);
// 0x6b2a10: calls Init_6b4a60.
u8 * __fastcall Wrap_Init_6b4a60_6b2a10(u8 *o);
// 0x6b2a50: calls Vtbl70a258_Set_6b4a90.
void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b2a50(u8 *o);
// 0x6b3c00: calls Init_6b4a60.
u8 * __fastcall Wrap_Init_6b4a60_6b3c00(u8 *o);
// 0x6b3c40: calls Vtbl70a258_Set_6b4a90.
void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b3c40(u8 *o);
}
