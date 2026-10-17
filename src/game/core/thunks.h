// Entry points that only call another ported function (the second and third entry of the same constructor or accessor). Reimplementation of 0x402df0, 0x40f220, 0x454540, 0x481590, 0x4815a0, 0x4815b0, 0x492020, 0x492060, 0x4fa460, 0x5f5640, 0x5f5650, 0x691ed0, 0x691f20, 0x692100 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x402df0: calls Obj_Get30.
u32 __fastcall Wrap_Obj_Get30_402df0(u8 *o);

// 0x40f220: calls Flags7a1a14_Update.
u32 __cdecl Wrap_Flags7a1a14_Update_40f220();

// 0x454540: calls Dword_ClearBit0.
u32 __cdecl Wrap_Dword_ClearBit0_454540();

// 0x481590: calls Global7c2650_Get.
u32 __cdecl Wrap_Global7c2650_Get_481590();

// 0x4815a0: calls Global7c2654_Get.
u32 __cdecl Wrap_Global7c2654_Get_4815a0();

// 0x4815b0: calls Global7c2654_Set.
u32 __stdcall Wrap_Global7c2654_Set_4815b0(u32 v0);

// 0x5f5640: calls Obj_Get64Plus10.
u32 __cdecl Wrap_Obj_Get64Plus10_5f5640();

// 0x5f5650: calls Owner64_Copy4At34.
u32 __cdecl Wrap_Owner64_Copy4At34_5f5650(u32 v0);

// 0x691ed0: calls Init_691ee0.
u8 * __cdecl Wrap_Init_691ee0_691ed0();

// 0x691f20: calls Init_691ee0.
u8 * __cdecl Wrap_Init_691ee0_691f20();

// 0x692100: calls Init_691ee0.
u8 * __cdecl Wrap_Init_691ee0_692100();

// 0x692960: calls Init_692970.
u8 * __cdecl Wrap_Init_692970_692960();

// 0x6929b0: calls Init_692970.
u8 * __cdecl Wrap_Init_692970_6929b0();

// 0x6929f0: calls Init_692970.
u8 * __cdecl Wrap_Init_692970_6929f0();

// 0x6baa10: calls Globalfb2708_Set_6d1bc0.
u32 __stdcall Wrap_Globalfb2708_Set_6d1bc0_6baa10(u32 v0);

// 0x6c4900: calls Init_6c4fb0.
u8 * __cdecl Wrap_Init_6c4fb0_6c4900();

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

// 0x6b0140: calls Wrap_Init_6b4a60_6b0710.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b0710_6b0140();

// 0x6b0180: calls Wrap_Init_6b4a60_6b1de0.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b1de0_6b0180();

// 0x6b01c0: calls Wrap_Init_6b4a60_6b2a10.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b2a10_6b01c0();

// 0x6b0200: calls Wrap_Init_6b4a60_6b3c00.
u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b3c00_6b0200();
}  // extern "C"
