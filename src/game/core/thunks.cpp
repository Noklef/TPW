#include "thunks.h"

#include <cstdint>

#include "accessors.h"
#include "forwarders.h"
#include "globals.h"
#include "initializers.h"
#include "misc.h"
#include "vtables.h"
#include "../gfx/texsurface.h"
#include "../gfx/texture.h"
#include "../render/rasteriser.h"
#include "../render/softrender.h"
#include "../render/spritequeue.h"
#include "../world/thingmsg.h"
#include "selectors.h"
#include "trivial.h"
#include "../gfx/wavelet.h"
#include "vdel.h"
#include "vdtor.h"
#include "vobj2.h"
#include "vwrap.h"
#include "../files/drives.h"
#include "../files/file.h"
#include "../files/handles.h"
#include "../files/wadsource.h"
#include "../gfx/canvas.h"
#include "../gfx/color.h"
#include "../gfx/gauge.h"
#include "../gfx/pixconv.h"
#include "../gfx/screensurfaces.h"
#include "../gfx/surface.h"
#include "../math/transform.h"
#include "semaphore.h"
#include "string.h"

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }

template <typename T> T Adjust(T p) { return reinterpret_cast<T>(reinterpret_cast<uintptr_t>(p) - *reinterpret_cast<const u32 *>(reinterpret_cast<uintptr_t>(p) - 4)); }

}  // namespace

extern "C" {

u32 __fastcall Wrap_Obj_Get30_402df0(u8 *o) {
    return Obj_Get30(reinterpret_cast<const u8 *>(o));
}

u32 __cdecl Wrap_Flags7a1a14_Update_40f220() {
    return Flags7a1a14_Update(reinterpret_cast<const u8 *>(reinterpret_cast<u8 *>(0x00786b68)));
}

u32 __cdecl Wrap_Dword_ClearBit0_454540() {
    return Dword_ClearBit0(reinterpret_cast<u32 *>(0x74d008));
}

u32 __cdecl Wrap_Global7c2650_Get_481590() {
    return Global7c2650_Get();
}

u32 __cdecl Wrap_Global7c2654_Get_4815a0() {
    return Global7c2654_Get();
}

u32 __stdcall Wrap_Global7c2654_Set_4815b0(u32 v0) {
    return Global7c2654_Set(v0);
}

u32 __cdecl Wrap_Obj_Get64Plus10_5f5640() {
    return Obj_Get64Plus10(reinterpret_cast<const u8 *>(reinterpret_cast<u8 *>(0x00fa6e40)));
}

u32 __cdecl Wrap_Owner64_Copy4At34_5f5650(u32 v0) {
    return Owner64_Copy4At34(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fa6e40)), nullptr, reinterpret_cast<const u32 *>((uintptr_t)v0));
}

u8 * __cdecl Wrap_Init_691ee0_691ed0() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ad0)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_691ee0_691f20() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb19a8)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_691ee0_692100() {
    return Init_691ee0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1a38)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_692960() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1c70)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_6929b0() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1c18)), nullptr, 0x0);
}

u8 * __cdecl Wrap_Init_692970_6929f0() {
    return Init_692970(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1d00)), nullptr, 0x0);
}

u32 __stdcall Wrap_Globalfb2708_Set_6d1bc0_6baa10(u32 v0) {
    return Globalfb2708_Set_6d1bc0(v0);
}

u8 * __cdecl Wrap_Init_6c4fb0_6c4900() {
    return Init_6c4fb0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb2038)));
}

u8 * __fastcall Wrap_Fn_669fa8_492020(u8 *o) {
    Fn_669fa8(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x6ff598; at<u32>(o, 0x18) = 0x0; return o;
}

u32 __fastcall Wrap_Fn_669fee_492060(u8 *o) {
    at<u32>(o, 0x0) = 0x6ff598; return Fn_669fee(reinterpret_cast<u8 *>(o));
}

u32 __fastcall Wrap_Set4_4217f0_4fa460(u8 *o) {
    return Set4_4217f0(reinterpret_cast<u8 *>((o) + 0xc), nullptr, 0x3);
}

void __fastcall Wrap_Vtbl70a078_Set_6b4a50_6b0470(u8 *o) {
    at<u32>(o, 0x0) = 0x70a158; Vtbl70a078_Set_6b4a50(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b0710(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a188; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b0750(u8 *o) {
    at<u32>(o, 0x0) = 0x70a188; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b1de0(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1a8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b1e20(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1a8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b2a10(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1c8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b2a50(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1c8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __fastcall Wrap_Init_6b4a60_6b3c00(u8 *o) {
    Init_6b4a60(reinterpret_cast<u8 *>(o)); at<u32>(o, 0x0) = 0x70a1e8; return o;
}

void __fastcall Wrap_Vtbl70a258_Set_6b4a90_6b3c40(u8 *o) {
    at<u32>(o, 0x0) = 0x70a1e8; Vtbl70a258_Set_6b4a90(reinterpret_cast<u8 *>(o));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b0710_6b0140() {
    return Wrap_Init_6b4a60_6b0710(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1efc)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b1de0_6b0180() {
    return Wrap_Init_6b4a60_6b1de0(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef0)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b2a10_6b01c0() {
    return Wrap_Init_6b4a60_6b2a10(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef4)));
}

u8 * __cdecl Wrap_Wrap_Init_6b4a60_6b3c00_6b0200() {
    return Wrap_Init_6b4a60_6b3c00(reinterpret_cast<u8 *>(reinterpret_cast<u8 *>(0x00fb1ef8)));
}

u32 __fastcall Wrap_MeshPlus60_Ready_42a0c0(u8 *self) {
    return MeshPlus60_Ready(Adjust(self));
}

u32 __fastcall Wrap_Stub_One_42a0d0(void *self) {
    return Stub_One(Adjust(self));
}

u8 *__fastcall Wrap_Plus28_IfKind2_42a0e0(u8 *self) {
    return Plus28_IfKind2(Adjust(self));
}

u8 *__fastcall Wrap_SpriteSet_DeletingDtor_491f10(u8 *self, void *, u32 flags) {
    return SpriteSet_DeletingDtor(Adjust(self), nullptr, flags);
}

u32 __fastcall Wrap_SpriteQueue_Ready_491f80(u8 *self) {
    return SpriteQueue_Ready(Adjust(self));
}

u32 __fastcall Wrap_Const3_6b71c0_491f90(u8 *) {
    return Const3_6b71c0();
}

s32 __fastcall Wrap_Const_MinusOne_564a50_564b10(u8 *) {
    return Const_MinusOne_564a50();
}

u32 __fastcall Wrap_Getm10_564ee0_5650a0(const u8 *self) {
    return Getm10_564ee0(Adjust(self));
}

u32 __stdcall Wrap_Const1_569d00_569cf0(u32 a0) {
    return Const1_569d00(a0);
}

u32 __fastcall Wrap_Const1_424ec0_569d80(u8 *) {
    return Const1_424ec0();
}

u32 __stdcall Wrap_Rasteriser_ChooseSpan_56a690(u32 a0) {
    return Rasteriser_ChooseSpan(a0);
}

u32 __stdcall Wrap_Setting569f30_Set_56a6a0(u32 a0, u32 a1) {
    return Setting569f30_Set(a0, a1);
}

u32 __stdcall Wrap_Const1_56a430_56a6d0(u32 a0, u32 a1, u32 a2) {
    return Const1_56a430(a0, a1, a2);
}

u32 __fastcall Wrap_Rasteriser_ClearMmx_56a6e0(u8 *) {
    return Rasteriser_ClearMmx();
}

u32 __stdcall Wrap_Const1_56a440_56a710(u32 a0, u32 a1, u32 a2, u32 a3) {
    return Const1_56a440(a0, a1, a2, a3);
}

u32 __stdcall Wrap_SoftRenderer_PixelAt_56a730(s32 a0, s32 a1, u32 *a2) {
    return SoftRenderer_PixelAt(a0, a1, a2);
}

u32 __fastcall Wrap_Zero_5cec60_5738a0(u8 *) {
    return Zero_5cec60();
}

u32 __fastcall Wrap_Global879a24_Get4c4_5738b0(u8 *) {
    return Global879a24_Get4c4();
}

u32 __fastcall Wrap_SoftRenderer_TextureMemory_573ef0(u8 *self) {
    return SoftRenderer_TextureMemory(Adjust(self));
}

u8 *__fastcall Wrap_SoftRenderer_NewSurfaceB_573f00(u8 *self) {
    return SoftRenderer_NewSurfaceB(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceB_AllocPixels_58c8e0(u8 *self, void *, u32 width, u32 height, u32 c, u32 d) {
    return TexSurfaceB_AllocPixels(Adjust(self), nullptr, width, height, c, d);
}

u32 __fastcall Wrap_TexSurfaceB_FreePixels_58c8f0(u8 *self) {
    return TexSurfaceB_FreePixels(Adjust(self));
}

u32 __fastcall Wrap_ThingMsg02_Type_57ac70(const u8 *self) {
    return ThingMsg02_Type(Adjust(self));
}

u32 __fastcall Wrap_Obj_GetBefore14c_57ac80(const u8 *self) {
    return Obj_GetBefore14c(Adjust(self));
}

u32 __fastcall Wrap_Const4_6bd790_57ca70(u8 *) {
    return Const4_6bd790();
}

u32 __fastcall Wrap_TexSurfaceA_ReleaseSurfaces_58c2a0(u8 *self) {
    return TexSurfaceA_ReleaseSurfaces(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceA_Field38_58c2b0(const u8 *self) {
    return TexSurfaceA_Field38(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceA_Area_58c2d0(const u8 *self) {
    return TexSurfaceA_Area(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceA_Field10_58c320(const u8 *self) {
    return TexSurfaceA_Field10(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceB_FieldC_58c900(const u8 *self) {
    return TexSurfaceB_FieldC(Adjust(self));
}

u32 __fastcall Wrap_TexSurfaceB_Area_58c910(const u8 *self) {
    return TexSurfaceB_Area(Adjust(self));
}

u32 __fastcall Wrap_Zero_5d60c0_58c920(u8 *) {
    return Zero_5d60c0();
}

float __fastcall Wrap_TexSurfaceB_Zero_58c940(const u8 *self) {
    return TexSurfaceB_Zero(Adjust(self));
}

u32 __fastcall Wrap_Texture_Release_590a70(u8 *self) { return Texture_Release(self); }

u32 __fastcall Wrap_Obj_IncBefore4c_5987c0(u8 *self) {
    return Obj_IncBefore4c(Adjust(self));
}

u32 __fastcall Wrap_WaveletCodec_GetRefCount_5987d0(const u8 *self) {
    return WaveletCodec_GetRefCount(Adjust(self));
}

u8 *__fastcall Wrap_Deleter_Minus60_42a090_42a080(u8 *self, void *, u32 a2) {
    return Deleter_Minus60_42a090(Adjust(self), nullptr, a2);
}

u32 __fastcall Wrap_VBase_CallSlot8_564af0(u8 *self, void *, u32 a2, u32 a3) {
    return VBase_CallSlot8(Adjust(self), nullptr, a2, a3);
}

u8 *__fastcall Wrap_Deleter_Minus2c_569c80_569c70(u8 *self, void *, u32 a2) {
    return Deleter_Minus2c_569c80(Adjust(self), nullptr, a2);
}

u8 *__fastcall Wrap_Deleter_Minus8_56a660_56a650(u8 *self, void *, u32 a2) {
    return Deleter_Minus8_56a660(Adjust(self), nullptr, a2);
}

u8 *__fastcall Wrap_VBase_Deleter_7018d0_579010(u8 *self, void *, u32 a2) {
    return VBase_Deleter_7018d0(Adjust(self), nullptr, a2);
}

u8 *__fastcall Wrap_VBase_Deleter_701920_5790b0(u8 *self, void *, u32 a2) {
    return VBase_Deleter_701920(Adjust(self), nullptr, a2);
}

u8 *__fastcall Wrap_VBase_Deleter_7018f8_579140(u8 *self, void *, u32 a2) {
    return VBase_Deleter_7018f8(Adjust(self), nullptr, a2);
}

void __cdecl Wrap_Transform_Vertices_579060(u8 *self, const float *matrix, float *out, const u8 *in, s32 out_stride, s32 in_stride,
                                            u32 count, s32 mode) {
    Transform_Vertices(Adjust(self), matrix, out, in, out_stride, in_stride, count, mode);
}

void __cdecl Wrap_Transform_BlockedPoints_579070(u8 *self, const float *matrix, float *out, const u8 *in, s32 out_stride, u32 first,
                                                 u32 count) {
    Transform_BlockedPoints(Adjust(self), matrix, out, in, out_stride, first, count);
}

double __cdecl Wrap_Transform_ProjectToScreen_579080(u8 *self, float *vertices, s32 stride, u32 count, s32 mode) {
    return Transform_ProjectToScreen(Adjust(self), vertices, stride, count, mode);
}

void __cdecl Wrap_Transform_Concatenate_5790a0(u8 *self, const float *a, const float *b, float *out) {
    Transform_Concatenate(Adjust(self), a, b, out);
}

u8 *__fastcall Wrap_Class702198_DeletingDtor_58c360(u8 *self, void *, u32 a2) {
    return Class702198_DeletingDtor(Adjust(self), nullptr, a2);
}

u32 __fastcall Wrap_WaveletCodec_DecodeScaled_598790(u8 *self, void *, s32 a2, u32 a3, u32 a4, u32 a5, u32 a6, u32 a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12, u32 a13, u32 a14, u32 a15) {
    return WaveletCodec_DecodeScaled(Adjust(self), nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15);
}

u32 __fastcall Wrap_WaveletCodec_Decode_5987a0(u8 *self, void *, s32 a2, u32 a3, u32 a4, u32 a5, u32 a6, u32 a7, u32 a8, u32 a9, u32 a10, u32 a11, u32 a12) {
    return WaveletCodec_Decode(Adjust(self), nullptr, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

s32 __fastcall Wrap_Refcounted_Release_5987e0(u8 *self) {
    return Refcounted_Release(Adjust(self));
}

void __fastcall Wrap_Class507800_Dtor_409240(u8 *self) {
    Class507800_Dtor(self);
}

void __fastcall Wrap_Handle_Dtor_41cc10(u8 *self) {
    Handle_Dtor(reinterpret_cast<Handle *>(self));
}

u32 __fastcall Wrap_MeshPlus60_Ready_42a000(u8 *self) {
    return MeshPlus60_Ready(reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(Adjust(self)) + 0x64));
}

u32 __fastcall Wrap_Stub_One_42a030(void *self) {
    return Stub_One(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(Adjust(self)) + 0x64));
}

u8 *__fastcall Wrap_Plus28_IfKind2_42a040(u8 *self) {
    return Plus28_IfKind2(reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(Adjust(self)) + 0x64));
}

void __cdecl Wrap_StringList_Dtor_45d810() {
    StringList_Dtor(reinterpret_cast<StringList *>(0xfb34e0));
}

void __fastcall Wrap_String_Dtor_45f940(u8 *self) {
    String_Dtor(reinterpret_cast<String *>(self));
}

void __cdecl Wrap_ScreenSurfaces_Dtor_47e790() {
    ScreenSurfaces_Dtor(reinterpret_cast<u8 *>(0x7b4ac0));
}

u32 __cdecl Wrap_Time_GetMs_489c80() {
    return Time_GetMs();
}

u32 __cdecl Wrap_Wrap_Fn_669fee_492060_492460() {
    return Wrap_Fn_669fee_492060(reinterpret_cast<u8 *>(0x7ca0c0));
}

u8 *__cdecl Wrap_SurfaceInfo_Ctor_492740() {
    return reinterpret_cast<u8 *>(SurfaceInfo_Ctor(reinterpret_cast<SurfaceInfo *>(0x7ca0e8)));
}

u8 *__cdecl Wrap_Gauge_Init_49a0a0() {
    return Gauge_Init(reinterpret_cast<u8 *>(0x7ca250));
}

u8 *__cdecl Wrap_Gauge_Init_4a46f0() {
    return Gauge_Init(reinterpret_cast<u8 *>(0x7cb318));
}

void __cdecl Wrap_Class70293c_Dtor_4c07c0() {
    Class70293c_Dtor(reinterpret_cast<u8 *>(0x7cc988));
}

u8 *__cdecl Wrap_SurfaceInfo_Ctor_51af80() {
    return reinterpret_cast<u8 *>(SurfaceInfo_Ctor(reinterpret_cast<SurfaceInfo *>(0x8023a8)));
}

void __cdecl Wrap_FileStream_Dtor_532660() {
    FileStream_Dtor(reinterpret_cast<StreamBase *>(0x81b0e8));
}

void __cdecl Wrap_ListDtor7027fc_5ac590_5ac500() {
    ListDtor7027fc_5ac590(reinterpret_cast<u8 *>(0xf7b560));
}

void __cdecl Wrap_Strings5_Dtor_5c4c50() {
    Strings5_Dtor(reinterpret_cast<u8 *>(0xf816c0));
}

void __fastcall Wrap_WString_Dtor_5c74a0(u8 *self) {
    WString_Dtor(reinterpret_cast<WString *>(self));
}

u8 *__cdecl Wrap_Class703710_Ctor_5f5560() {
    return Class703710_Ctor(reinterpret_cast<u8 *>(0xfa6e40));
}

void __cdecl Wrap_Time_Init_5f5f80() {
    Time_Init();
}

u8 *__cdecl Wrap_SystemInfo_Ctor_5f8a40() {
    return reinterpret_cast<u8 *>(SystemInfo_Ctor(reinterpret_cast<SystemInfo *>(0xfa7620)));
}

void __cdecl Wrap_PixelFormat_InitArgb8888Unused_5fcce0() {
    PixelFormat_InitArgb8888Unused();
}

void __cdecl Wrap_PixelFormat_InitArgb1555_5fcd30() {
    PixelFormat_InitArgb1555();
}

void __cdecl Wrap_PixelFormat_InitRgb555_5fcd80() {
    PixelFormat_InitRgb555();
}

void __cdecl Wrap_PixelFormat_InitRgb565_5fcdd0() {
    PixelFormat_InitRgb565();
}

void __cdecl Wrap_PixelFormat_InitArgb4444_5fce20() {
    PixelFormat_InitArgb4444();
}

void __cdecl Wrap_PixelFormat_InitRgb888_5fce70() {
    PixelFormat_InitRgb888();
}

void __cdecl Wrap_PixelFormat_InitBgr888_5fcec0() {
    PixelFormat_InitBgr888();
}

void __cdecl Wrap_PixelFormat_InitArgb8888_5fcf10() {
    PixelFormat_InitArgb8888();
}

void __cdecl Wrap_PixelFormat_InitAbgr8888_5fcf60() {
    PixelFormat_InitAbgr8888();
}

void __cdecl Wrap_PixelFormat_InitXrgb8888_5fcfb0() {
    PixelFormat_InitXrgb8888();
}

void __cdecl Wrap_PixelFormat_InitXbgr8888_5fd000() {
    PixelFormat_InitXbgr8888();
}

void __cdecl Wrap_CanvasDriver8_Init_617360() {
    CanvasDriver8_Init();
}

void __cdecl Wrap_CanvasDriver16_Init_617380() {
    CanvasDriver16_Init();
}

void __cdecl Wrap_CanvasDriver24_Init_6173a0() {
    CanvasDriver24_Init();
}

void __cdecl Wrap_CanvasDriver32_Init_6173c0() {
    CanvasDriver32_Init();
}

u8 *__cdecl Wrap_Canvas_Ctor_618ca0() {
    return reinterpret_cast<u8 *>(Canvas_Ctor(reinterpret_cast<Canvas *>(0xfa8258)));
}

void __fastcall Wrap_PixelConversion_Dtor_6300b0(u8 *self) {
    PixelConversion_Dtor(reinterpret_cast<PixelConversion *>(self));
}

void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b0750_6b0160() {
    Wrap_Vtbl70a258_Set_6b4a90_6b0750(reinterpret_cast<u8 *>(0xfb1efc));
}

void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b1e20_6b01a0() {
    Wrap_Vtbl70a258_Set_6b4a90_6b1e20(reinterpret_cast<u8 *>(0xfb1ef0));
}

void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b2a50_6b01e0() {
    Wrap_Vtbl70a258_Set_6b4a90_6b2a50(reinterpret_cast<u8 *>(0xfb1ef4));
}

void __cdecl Wrap_Wrap_Vtbl70a258_Set_6b4a90_6b3c40_6b0220() {
    Wrap_Vtbl70a258_Set_6b4a90_6b3c40(reinterpret_cast<u8 *>(0xfb1ef8));
}

void __fastcall Wrap_Handle_Dtor_6bf320(u8 *self) {
    Handle_Dtor(reinterpret_cast<Handle *>(reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(self) + 0x4)));
}

void __fastcall Wrap_String_Dtor_6c1230(u8 *self) {
    String_Dtor(reinterpret_cast<String *>(reinterpret_cast<u8 *>(reinterpret_cast<uintptr_t>(self) + 0x4)));
}

u8 *__cdecl Wrap_Semaphore_Ctor_6c5ae0() {
    return reinterpret_cast<u8 *>(Semaphore_Ctor(reinterpret_cast<Semaphore *>(0xfb205c)));
}

void __cdecl Wrap_StringList_Dtor_6d8990() {
    StringList_Dtor(reinterpret_cast<StringList *>(0xfb34e0));
}
}  // extern "C"
