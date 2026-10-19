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

// 0x491f10 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then SpriteSet_DeletingDtor.
u8 *__fastcall Wrap_SpriteSet_DeletingDtor_491f10(u8 *self, void *, u32 flags);

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

// 0x573f00 (thiscall): this lowered by the dword before it (a virtual base offset), then SoftRenderer_NewSurfaceB.
u8 *__fastcall Wrap_SoftRenderer_NewSurfaceB_573f00(u8 *self);

// 0x58c8e0 (thiscall; width, height, two unused; pops 0x10): this lowered by the dword before it, then TexSurfaceB_AllocPixels.
u32 __fastcall Wrap_TexSurfaceB_AllocPixels_58c8e0(u8 *self, void *, u32 width, u32 height, u32 c, u32 d);

// 0x58c930 (thiscall): this lowered by the dword before it, then TexSurfaceB_Blit.
u32 __fastcall Wrap_TexSurfaceB_Blit_58c930(u8 *self, void *, u8 *texture, s32 x, s32 y);

// 0x58c8f0 (thiscall): this lowered by the dword before it, then TexSurfaceB_FreePixels.
u32 __fastcall Wrap_TexSurfaceB_FreePixels_58c8f0(u8 *self);

// 0x57ac70 (thiscall): this lowered by the dword before it (a virtual base offset), then ThingMsg02_Type.
u32 __fastcall Wrap_ThingMsg02_Type_57ac70(const u8 *self);

// 0x57ac80 (thiscall): this lowered by the dword before it (a virtual base offset), then Obj_GetBefore14c.
u32 __fastcall Wrap_Obj_GetBefore14c_57ac80(const u8 *self);

// 0x57ca70 (this is not used): calls Const4_6bd790.
u32 __fastcall Wrap_Const4_6bd790_57ca70(u8 *);

// 0x58c2a0 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceA_ReleaseSurfaces.
u32 __fastcall Wrap_TexSurfaceA_ReleaseSurfaces_58c2a0(u8 *self);
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

// 0x58c940 (thiscall): this lowered by the dword before it (a virtual base offset), then TexSurfaceB_Zero, its float on ST0.
float __fastcall Wrap_TexSurfaceB_Zero_58c940(const u8 *self);

// 0x58c950 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then TexSurfaceB_DeletingDtor.
u8 *__fastcall Wrap_TexSurfaceB_DeletingDtor_58c950(u8 *self, void *, u32 flags);

// 0x590a70 (thiscall): jumps to Texture_Release, the destructor the texture arrays are given.
u32 __fastcall Wrap_Texture_Release_590a70(u8 *self);

// 0x57abe0 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then
// LiveObject7019f8_DeletingDtor (core/liveobjects; the class's vtable 0x7019f8 slot 0).
u8 *__fastcall Wrap_LiveObject7019f8_DeletingDtor_57abe0(u8 *self, void *, u32 flags);

// 0x572100 (thiscall): jumps to TerrainTexCache_Dtor (terrain/texcache).
u32 __fastcall Wrap_TerrainTexCache_Dtor_572100(u8 *self);

// 0x5987c0 (thiscall): this lowered by the dword before it (a virtual base offset), then Obj_IncBefore4c.
u32 __fastcall Wrap_Obj_IncBefore4c_5987c0(u8 *self);

// 0x5987d0 (thiscall): this lowered by the dword before it (a virtual base offset), then WaveletCodec_GetRefCount.
u32 __fastcall Wrap_WaveletCodec_GetRefCount_5987d0(const u8 *self);

// 0x42a080 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then Deleter_Minus60_42a090.
u8 *__fastcall Wrap_Deleter_Minus60_42a090_42a080(u8 *self, void *, u32 a2);

// 0x42a070 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then Deleter_Minus8_458970.
u8 *__fastcall Wrap_Deleter_Minus8_458970_42a070(u8 *self, void *, u32 a2);

// 0x564af0 (thiscall; 2 stack arguments): this lowered by the dword before it (a virtual base offset), then VBase_CallSlot8.
u32 __fastcall Wrap_VBase_CallSlot8_564af0(u8 *self, void *, u32 a2, u32 a3);

// 0x569c70 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then Deleter_Minus2c_569c80.
u8 *__fastcall Wrap_Deleter_Minus2c_569c80_569c70(u8 *self, void *, u32 a2);

// 0x56a650 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then Deleter_Minus8_56a660.
u8 *__fastcall Wrap_Deleter_Minus8_56a660_56a650(u8 *self, void *, u32 a2);

// 0x579010 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then VBase_Deleter_7018d0.
u8 *__fastcall Wrap_VBase_Deleter_7018d0_579010(u8 *self, void *, u32 a2);

// 0x5790b0 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then VBase_Deleter_701920.
u8 *__fastcall Wrap_VBase_Deleter_701920_5790b0(u8 *self, void *, u32 a2);

// 0x579140 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then VBase_Deleter_7018f8.
u8 *__fastcall Wrap_VBase_Deleter_7018f8_579140(u8 *self, void *, u32 a2);

// 0x579060, 0x579070, 0x579080, 0x5790a0 (cdecl; this the first stack argument): this lowered by the dword before it (a virtual
// base offset; the argument rewritten in place), then Transform_Vertices, Transform_BlockedPoints, Transform_ProjectToScreen and
// Transform_Concatenate (src/game/math/transform), which take it unused.
void __cdecl Wrap_Transform_Vertices_579060(u8 *self, const float *matrix, float *out, const u8 *in, s32 out_stride, s32 in_stride,
                                            u32 count, s32 mode);
void __cdecl Wrap_Transform_BlockedPoints_579070(u8 *self, const float *matrix, float *out, const u8 *in, s32 out_stride, u32 first,
                                                 u32 count);
double __cdecl Wrap_Transform_ProjectToScreen_579080(u8 *self, float *vertices, s32 stride, u32 count, s32 mode);
void __cdecl Wrap_Transform_Concatenate_5790a0(u8 *self, const float *a, const float *b, float *out);

// 0x58c360 (thiscall; 1 stack arguments): this lowered by the dword before it (a virtual base offset), then Class702198_DeletingDtor.
u8 *__fastcall Wrap_Class702198_DeletingDtor_58c360(u8 *self, void *, u32 a2);

// 0x598790 (thiscall; 14 stack arguments): this lowered by the dword before it (a virtual base offset), then WaveletCodec_DecodeScaled.
u32 __fastcall Wrap_WaveletCodec_DecodeScaled_598790(u8 *self, void *, s32 a2, u32 a3, u32 a4, u32 a5, u32 a6, u32 a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12, u32 a13, u32 a14, u32 a15);

// 0x5987a0 (thiscall; 11 stack arguments): this lowered by the dword before it (a virtual base offset), then WaveletCodec_Decode.
u32 __fastcall Wrap_WaveletCodec_Decode_5987a0(u8 *self, void *, s32 a2, u32 a3, u32 a4, u32 a5, u32 a6, u32 a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12);

// 0x5987e0 (thiscall): this lowered by the dword before it (a virtual base offset), then Refcounted_Release.
s32 __fastcall Wrap_Refcounted_Release_5987e0(u8 *self);

// 0x409240: calls Class507800_Dtor.
void __fastcall Wrap_Class507800_Dtor_409240(u8 *self);

// 0x41cc10: calls Handle_Dtor.
void __fastcall Wrap_Handle_Dtor_41cc10(u8 *self);

// 0x42a000: lowers this by the dword before it, then raises it by 0x64 and calls MeshPlus60_Ready.
u32 __fastcall Wrap_MeshPlus60_Ready_42a000(u8 *self);

// 0x42a030: lowers this by the dword before it, then raises it by 0x64 and calls Stub_One.
u32 __fastcall Wrap_Stub_One_42a030(void *self);

// 0x42a040: lowers this by the dword before it, then raises it by 0x64 and calls Plus28_IfKind2.
u8 *__fastcall Wrap_Plus28_IfKind2_42a040(u8 *self);

// 0x45d810: calls StringList_Dtor on the object at 0xfb34e0.
void __cdecl Wrap_StringList_Dtor_45d810();

// 0x45f940: calls String_Dtor.
void __fastcall Wrap_String_Dtor_45f940(u8 *self);

// 0x47e790: calls ScreenSurfaces_Dtor on the object at 0x7b4ac0.
void __cdecl Wrap_ScreenSurfaces_Dtor_47e790();

// 0x489c80: calls Time_GetMs.
u32 __cdecl Wrap_Time_GetMs_489c80();

// 0x492460: calls Wrap_Fn_669fee_492060 on the object at 0x7ca0c0.
u32 __cdecl Wrap_Wrap_Fn_669fee_492060_492460();

// 0x492740: calls SurfaceInfo_Ctor on the object at 0x7ca0e8.
u8 *__cdecl Wrap_SurfaceInfo_Ctor_492740();

// 0x49a0a0: calls Gauge_Init on the object at 0x7ca250.
u8 *__cdecl Wrap_Gauge_Init_49a0a0();

// 0x4a46f0: calls Gauge_Init on the object at 0x7cb318.
u8 *__cdecl Wrap_Gauge_Init_4a46f0();

// 0x4c07c0: calls Class70293c_Dtor on the object at 0x7cc988.
void __cdecl Wrap_Class70293c_Dtor_4c07c0();

// 0x51af80: calls SurfaceInfo_Ctor on the object at 0x8023a8.
u8 *__cdecl Wrap_SurfaceInfo_Ctor_51af80();

// 0x532660: calls FileStream_Dtor on the object at 0x81b0e8.
void __cdecl Wrap_FileStream_Dtor_532660();

// 0x5ac500: calls ListDtor7027fc_5ac590 on the object at 0xf7b560.
void __cdecl Wrap_ListDtor7027fc_5ac590_5ac500();

// 0x5c4c50: calls Strings5_Dtor on the object at 0xf816c0.
void __cdecl Wrap_Strings5_Dtor_5c4c50();

// 0x5c74a0: calls WString_Dtor.
void __fastcall Wrap_WString_Dtor_5c74a0(u8 *self);

// 0x5f5560: calls Class703710_Ctor on the object at 0xfa6e40.
u8 *__cdecl Wrap_Class703710_Ctor_5f5560();

// 0x5f5f80: calls Time_Init.
void __cdecl Wrap_Time_Init_5f5f80();

// 0x5f8a40: calls SystemInfo_Ctor on the object at 0xfa7620.
u8 *__cdecl Wrap_SystemInfo_Ctor_5f8a40();

// 0x5fcce0: calls PixelFormat_InitArgb8888Unused.
void __cdecl Wrap_PixelFormat_InitArgb8888Unused_5fcce0();

// 0x5fcd30: calls PixelFormat_InitArgb1555.
void __cdecl Wrap_PixelFormat_InitArgb1555_5fcd30();

// 0x5fcd80: calls PixelFormat_InitRgb555.
void __cdecl Wrap_PixelFormat_InitRgb555_5fcd80();

// 0x5fcdd0: calls PixelFormat_InitRgb565.
void __cdecl Wrap_PixelFormat_InitRgb565_5fcdd0();

// 0x5fce20: calls PixelFormat_InitArgb4444.
void __cdecl Wrap_PixelFormat_InitArgb4444_5fce20();

// 0x5fce70: calls PixelFormat_InitRgb888.
void __cdecl Wrap_PixelFormat_InitRgb888_5fce70();

// 0x5fcec0: calls PixelFormat_InitBgr888.
void __cdecl Wrap_PixelFormat_InitBgr888_5fcec0();

// 0x5fcf10: calls PixelFormat_InitArgb8888.
void __cdecl Wrap_PixelFormat_InitArgb8888_5fcf10();

// 0x5fcf60: calls PixelFormat_InitAbgr8888.
void __cdecl Wrap_PixelFormat_InitAbgr8888_5fcf60();

// 0x5fcfb0: calls PixelFormat_InitXrgb8888.
void __cdecl Wrap_PixelFormat_InitXrgb8888_5fcfb0();

// 0x5fd000: calls PixelFormat_InitXbgr8888.
void __cdecl Wrap_PixelFormat_InitXbgr8888_5fd000();

// 0x617360: calls CanvasDriver8_Init.
void __cdecl Wrap_CanvasDriver8_Init_617360();

// 0x617380: calls CanvasDriver16_Init.
void __cdecl Wrap_CanvasDriver16_Init_617380();

// 0x6173a0: calls CanvasDriver24_Init.
void __cdecl Wrap_CanvasDriver24_Init_6173a0();

// 0x6173c0: calls CanvasDriver32_Init.
void __cdecl Wrap_CanvasDriver32_Init_6173c0();

// 0x618ca0: calls Canvas_Ctor on the object at 0xfa8258.
u8 *__cdecl Wrap_Canvas_Ctor_618ca0();

// 0x6300b0: calls PixelConversion_Dtor.
void __fastcall Wrap_PixelConversion_Dtor_6300b0(u8 *self);

// 0x6b0160: calls Wrap_Vtbl70a258_Set_6b4a90_6b0750 on the object at 0xfb1efc.
void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b0750_6b0160();

// 0x6b01a0: calls Wrap_Vtbl70a258_Set_6b4a90_6b1e20 on the object at 0xfb1ef0.
void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b1e20_6b01a0();

// 0x6b01e0: calls Wrap_Vtbl70a258_Set_6b4a90_6b2a50 on the object at 0xfb1ef4.
void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b2a50_6b01e0();

// 0x6b0220: calls Wrap_Vtbl70a258_Set_6b4a90_6b3c40 on the object at 0xfb1ef8.
void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b3c40_6b0220();

// 0x6bf320: calls Handle_Dtor with this raised by 0x4.
void __fastcall Wrap_Handle_Dtor_6bf320(u8 *self);

// 0x6c1230: calls String_Dtor with this raised by 0x4.
void __fastcall Wrap_String_Dtor_6c1230(u8 *self);

// 0x6c5ae0: calls Semaphore_Ctor on the object at 0xfb205c.
u8 *__cdecl Wrap_Semaphore_Ctor_6c5ae0();

// 0x6d8990: calls StringList_Dtor on the object at 0xfb34e0.
void __cdecl Wrap_StringList_Dtor_6d8990();

// 0x481410: jumps to Panel7b9940_Detach.
u32 __cdecl Wrap_Panel7b9940_Detach_481410();

// 0x564a80 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then RleImage_DeletingDtor.
u8 *__fastcall Wrap_RleImage_DeletingDtor_564a80(u8 *self, void *, u32 flags);

// 0x564b00 (thiscall; dest; pops 4): this lowered by the dword before it (a virtual base offset), then RleImage_DrawScaled.
u32 __fastcall Wrap_RleImage_DrawScaled_564b00(const u8 *self, void *, const u32 *dest);

// 0x564ff0 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then WctImage_DeletingDtor.
u8 *__fastcall Wrap_WctImage_DeletingDtor_564ff0(u8 *self, void *, u32 flags);

// 0x565080 (thiscall; surface, flip; pops 8): this lowered by the dword before it (a virtual base offset), then Texture_DecodeWctInto.
u32 __fastcall Wrap_Texture_DecodeWctInto_565080(const u8 *self, void *, u32 *surface, u32 flip);

// 0x565090 (thiscall; pixels; pops 4): this lowered by the dword before it (a virtual base offset), then Texture_DecodeWct.
u32 __fastcall Wrap_Texture_DecodeWct_565090(const u8 *self, void *, u8 *const *pixels);

// 0x569cc0 (thiscall; flags; pops 4): this lowered by the dword before it (a virtual base offset), then D3dProbe_SetFlags.
u32 __fastcall Wrap_D3dProbe_SetFlags_569cc0(u8 *self, void *, u32 flags);

// 0x5738c0 (thiscall): this lowered by the dword before it (a virtual base offset), then Renderer_NewSurfaceA.
u8 *__fastcall Wrap_Renderer_NewSurfaceA_5738c0(void *self);
// 0x4813f0: jumps to 0x48a290 (ui/panel7b9940).
void __cdecl Wrap_Panel7b9940_Close_4813f0();
// 0x481400: jumps to Panel7b9940_Show (ui/panel7b9940).
u32 __cdecl Wrap_Panel7b9940_Show_481400();
// 0x4f8ad0 (thiscall): jumps to Nav_Dtor (world/nav).
void __fastcall Wrap_Nav_Dtor_4f8ad0(u8 *nav);
// 0x510150 (thiscall): jumps to Nav_Update (world/nav).
void __fastcall Wrap_Nav_Update_510150(u8 *nav);
// 0x510180 (thiscall): jumps to Nav_PathProgress (world/nav).
s32 __fastcall Wrap_Nav_PathProgress_510180(const u8 *nav);
// 0x4815c0: jumps to KeyMap_AsyncModifiers (core/keymap).
u32 __cdecl Wrap_KeyMap_AsyncModifiers_4815c0();
}  // extern "C"
