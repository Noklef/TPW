// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
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
}
