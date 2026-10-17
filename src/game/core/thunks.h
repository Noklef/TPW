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

// 0x42a0c0 (thiscall): this lowered by the dword before it (a virtual base offset), then MeshPlus60_Ready.
u32 __fastcall Wrap_MeshPlus60_Ready_42a0c0(u8 *self);

// 0x42a0d0 (thiscall): this lowered by the dword before it (a virtual base offset), then Stub_One.
u32 __fastcall Wrap_Stub_One_42a0d0(void *self);

// 0x42a0e0 (thiscall): this lowered by the dword before it (a virtual base offset), then Plus28_IfKind2.
u8 *__fastcall Wrap_Plus28_IfKind2_42a0e0(u8 *self);

// 0x491f80 (thiscall): this lowered by the dword before it (a virtual base offset), then SpriteQueue_Ready.
u32 __fastcall Wrap_SpriteQueue_Ready_491f80(u8 *self);

// 0x491f90 (this is not used): calls Const3_6b71c0.
u32 __fastcall Wrap_Const3_6b71c0_491f90(u8 *);

// 0x564b10 (this is not used): calls Const_MinusOne_564a50.
s32 __fastcall Wrap_Const_MinusOne_564a50_564b10(u8 *);

// 0x5650a0 (thiscall): this lowered by the dword before it (a virtual base offset), then Getm10_564ee0.
u32 __fastcall Wrap_Getm10_564ee0_5650a0(const u8 *self);

// 0x569cf0 (this is not used; 1 stack arguments, popped by the callee): calls Const1_569d00.
u32 __stdcall Wrap_Const1_569d00_569cf0(u32 a0);

// 0x569d80 (this is not used): calls Const1_424ec0.
u32 __fastcall Wrap_Const1_424ec0_569d80(u8 *);

// 0x56a690 (this is not used; 1 stack arguments, popped by the callee): calls Rasteriser_ChooseSpan.
u32 __stdcall Wrap_Rasteriser_ChooseSpan_56a690(u32 a0);

// 0x56a6a0 (this is not used; 2 stack arguments, popped by the callee): calls Setting569f30_Set.
u32 __stdcall Wrap_Setting569f30_Set_56a6a0(u32 a0, u32 a1);

// 0x56a6d0 (this is not used; 3 stack arguments, popped by the callee): calls Const1_56a430.
u32 __stdcall Wrap_Const1_56a430_56a6d0(u32 a0, u32 a1, u32 a2);

// 0x56a6e0 (this is not used): calls Rasteriser_ClearMmx.
u32 __fastcall Wrap_Rasteriser_ClearMmx_56a6e0(u8 *);

// 0x56a710 (this is not used; 4 stack arguments, popped by the callee): calls Const1_56a440.
u32 __stdcall Wrap_Const1_56a440_56a710(u32 a0, u32 a1, u32 a2, u32 a3);

// 0x56a730 (this is not used; 3 stack arguments, popped by the callee): calls SoftRenderer_PixelAt.
u32 __stdcall Wrap_SoftRenderer_PixelAt_56a730(s32 a0, s32 a1, u32 *a2);

// 0x5738a0 (this is not used): calls Zero_5cec60.
u32 __fastcall Wrap_Zero_5cec60_5738a0(u8 *);

// 0x5738b0 (this is not used): calls Global879a24_Get4c4.
u32 __fastcall Wrap_Global879a24_Get4c4_5738b0(u8 *);

// 0x573ef0 (thiscall): this lowered by the dword before it (a virtual base offset), then SoftRenderer_TextureMemory.
u32 __fastcall Wrap_SoftRenderer_TextureMemory_573ef0(u8 *self);

// 0x57ac70 (thiscall): this lowered by the dword before it (a virtual base offset), then ThingMsg02_Type.
u32 __fastcall Wrap_ThingMsg02_Type_57ac70(const u8 *self);

// 0x57ac80 (thiscall): this lowered by the dword before it (a virtual base offset), then Obj_GetBefore14c.
u32 __fastcall Wrap_Obj_GetBefore14c_57ac80(const u8 *self);

// 0x57ca70 (this is not used): calls Const4_6bd790.
u32 __fastcall Wrap_Const4_6bd790_57ca70(u8 *);

// 0x58c2b0 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceA_Field38.
u32 __fastcall Wrap_TexSurfaceA_Field38_58c2b0(const u8 *self);

// 0x58c2d0 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceA_Area.
u32 __fastcall Wrap_TexSurfaceA_Area_58c2d0(const u8 *self);

// 0x58c320 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceA_Field10.
u32 __fastcall Wrap_TexSurfaceA_Field10_58c320(const u8 *self);

// 0x58c900 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceB_FieldC.
u32 __fastcall Wrap_TexSurfaceB_FieldC_58c900(const u8 *self);

// 0x58c910 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceB_Area.
u32 __fastcall Wrap_TexSurfaceB_Area_58c910(const u8 *self);

// 0x58c920 (this is not used): calls Zero_5d60c0.
u32 __fastcall Wrap_Zero_5d60c0_58c920(u8 *);

// 0x5987c0 (thiscall): this lowered by the dword before it (a virtual base offset), then Obj_IncBefore4c.
u32 __fastcall Wrap_Obj_IncBefore4c_5987c0(u8 *self);

// 0x5987d0 (thiscall): this lowered by the dword before it (a virtual base offset), then WaveletCodec_GetRefCount.
u32 __fastcall Wrap_WaveletCodec_GetRefCount_5987d0(const u8 *self);

}  // extern "C"
