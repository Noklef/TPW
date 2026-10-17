#include "thunks.h"

#include <cstdint>

#include "accessors.h"
#include "forwarders.h"
#include "globals.h"
#include "initializers.h"
#include "misc.h"
#include "vtables.h"
#include "../gfx/texsurface.h"
#include "../render/rasteriser.h"
#include "../render/softrender.h"
#include "../render/spritequeue.h"
#include "../world/thingmsg.h"
#include "selectors.h"
#include "trivial.h"

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

u32 __fastcall Wrap_ThingMsg02_Type_57ac70(const u8 *self) {
    return ThingMsg02_Type(Adjust(self));
}

u32 __fastcall Wrap_Obj_GetBefore14c_57ac80(const u8 *self) {
    return Obj_GetBefore14c(Adjust(self));
}

u32 __fastcall Wrap_Const4_6bd790_57ca70(u8 *) {
    return Const4_6bd790();
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

u32 __fastcall Wrap_Obj_IncBefore4c_5987c0(u8 *self) {
    return Obj_IncBefore4c(Adjust(self));
}

u32 __fastcall Wrap_WaveletCodec_GetRefCount_5987d0(const u8 *self) {
    return WaveletCodec_GetRefCount(Adjust(self));
}

}  // extern "C"
