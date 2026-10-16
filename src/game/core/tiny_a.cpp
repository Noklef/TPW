#include "tiny_a.h"

#include <cstdint>

namespace {

template <typename T> T &at(const void *p, u32 offset) { return *reinterpret_cast<T *>((uintptr_t)p + offset); }
template <typename T> T &global(u32 address) { return *reinterpret_cast<T *>((uintptr_t)address); }

}  // namespace

extern "C" {

void __fastcall Vtbl6fd6c8_Set_405520(u8 *o) {
    at<u32>(o, 0) = 0x006fd6c8;
}

u8 __fastcall GetBytea_405560(const u8 *o) {
    return at<u8>(o, 0xa);
}

u32 __fastcall Get0_4056e0(const u8 *o) {
    return at<u32>(o, 0x0);
}

u32 __fastcall Get4f0_4145e0(const u8 *o) {
    return at<u32>(o, 0x4f0);
}

u32 __fastcall Get34_41ce80(const u8 *o) {
    return at<u32>(o, 0x34);
}

u32 __fastcall Get38_41ce90(const u8 *o) {
    return at<u32>(o, 0x38);
}

u32 __fastcall StreamBase_GetSource(const u8 *o) {
    return at<u32>(o, 0x4);
}

u32 __fastcall Set4_4217f0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4) = v0;
    return v0;
}

u32 __cdecl Const1_424ec0() {
    return 0x1;
}

void __fastcall Vtbl6fdce8_Set_429b50(u8 *o) {
    at<u32>(o, 0) = 0x006fdce8;
}

u32 __cdecl Global7b4b88_Get_47eaa0() {
    return global<u32>(0x007b4b88);
}

u32 __fastcall Set4c_48f690(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x4c) = v0;
    return v0;
}

void __fastcall Vtbl6fd810_Set_4a8bf0(u8 *o) {
    at<u32>(o, 0) = 0x006fd810;
}

void __fastcall Vtbl6fefb8_Set_4bc420(u8 *o) {
    at<u32>(o, 0) = 0x006fefb8;
}

void __fastcall Vtbl6fd838_Set_4c75f0(u8 *o) {
    at<u32>(o, 0) = 0x006fd838;
}

void __fastcall Vtbl6fd830_Set_4d19f0(u8 *o) {
    at<u32>(o, 0) = 0x006fd830;
}

void __fastcall Vtbl6fd7b0_Set_4d5260(u8 *o) {
    at<u32>(o, 0) = 0x006fd7b0;
}

void __fastcall Vtbl6fd7c0_Set_4db7c0(u8 *o) {
    at<u32>(o, 0) = 0x006fd7c0;
}

void __fastcall Vtbl6fd7e0_Set_4dd310(u8 *o) {
    at<u32>(o, 0) = 0x006fd7e0;
}

u32 __fastcall Get64_4e03d0(const u8 *o) {
    return at<u32>(o, 0x64);
}

void __fastcall Vtbl6fd7c8_Set_4e11f0(u8 *o) {
    at<u32>(o, 0) = 0x006fd7c8;
}

void __fastcall Vtbl6fd7d0_Set_4e1200(u8 *o) {
    at<u32>(o, 0) = 0x006fd7d0;
}

void __fastcall Vtbl6fd7d8_Set_4e1210(u8 *o) {
    at<u32>(o, 0) = 0x006fd7d8;
}

u8 __fastcall GetByte190_4e21b0(const u8 *o) {
    return at<u8>(o, 0x190);
}

u32 __fastcall Set198_4e21e0(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x198) = v0;
    return v0;
}

u8 __fastcall GetByte198_4e21f0(const u8 *o) {
    return at<u8>(o, 0x198);
}

void __fastcall Vtbl700620_Set_4e5b90(u8 *o) {
    at<u32>(o, 0) = 0x00700620;
}

u32 __cdecl Const7ffffffd_4f6080() {
    return 0x7ffffffd;
}

void __fastcall Vtbl6fd7e8_Set_4f8570(u8 *o) {
    at<u32>(o, 0) = 0x006fd7e8;
}

void __fastcall Vtbl6fd7f0_Set_4f8580(u8 *o) {
    at<u32>(o, 0) = 0x006fd7f0;
}

void __fastcall Vtbl6fd7f8_Set_4f8590(u8 *o) {
    at<u32>(o, 0) = 0x006fd7f8;
}

void __fastcall Vtbl6fd840_Set_4fb320(u8 *o) {
    at<u32>(o, 0) = 0x006fd840;
}

void __fastcall Vtbl7007a8_Set_4ff4e0(u8 *o) {
    at<u32>(o, 0) = 0x007007a8;
}

u32 __cdecl Conste_4ff4f0() {
    return 0xe;
}

void __fastcall Vtbl6fd808_Set_503fe0(u8 *o) {
    at<u32>(o, 0) = 0x006fd808;
}

void __fastcall Vtbl700868_Set_506190(u8 *o) {
    at<u32>(o, 0) = 0x00700868;
}

u32 __cdecl Constf_5061a0() {
    return 0xf;
}

void __fastcall Vtbl6fd820_Set_509350(u8 *o) {
    at<u32>(o, 0) = 0x006fd820;
}

void __fastcall Vtbl6fd848_Set_50b970(u8 *o) {
    at<u32>(o, 0) = 0x006fd848;
}

u32 __stdcall Const1_50c1f0(u32 v0) {
    return 0x1;
}

void __fastcall Vtbl6fd7a0_Set_512c30(u8 *o) {
    at<u32>(o, 0) = 0x006fd7a0;
}

void __fastcall Vtbl6fd7a8_Set_512c40(u8 *o) {
    at<u32>(o, 0) = 0x006fd7a8;
}

u32 __cdecl Global785584_Get_519410() {
    return global<u32>(0x00785584);
}

u32 __cdecl Global785588_Get_519420() {
    return global<u32>(0x00785588);
}

u32 __cdecl Global78558c_Get_519430() {
    return global<u32>(0x0078558c);
}

u32 __cdecl Global785590_Get_519440() {
    return global<u32>(0x00785590);
}

u32 __fastcall Get1da710_51a280(const u8 *o) {
    return at<u32>(o, 0x1da710);
}

u32 __fastcall Get8_51c050(const u8 *o) {
    return at<u32>(o, 0x8);
}

u32 __cdecl Global81d7a4_Get_52f1f0() {
    return global<u32>(0x0081d7a4);
}

u32 __cdecl Global81ae2c_Get_52f840() {
    return global<u32>(0x0081ae2c);
}

u32 __cdecl Global81af64_Get_52f850() {
    return global<u32>(0x0081af64);
}

u32 __cdecl Global81b6dc_Set_52fb10(u32 v0) {
    global<u32>(0x0081b6dc) = v0;
    return v0;
}

u32 __cdecl Global81b6dc_Get_52fb20() {
    return global<u32>(0x0081b6dc);
}

u32 __cdecl Global81ea14_Set_52fb30(u32 v0) {
    global<u32>(0x0081ea14) = v0;
    return v0;
}

u32 __cdecl Global81ea14_Get_52fb40() {
    return global<u32>(0x0081ea14);
}

u32 __cdecl Global81aebc_Set_52fb50(u32 v0) {
    global<u32>(0x0081aebc) = v0;
    return v0;
}

u32 __cdecl Global81aebc_Get_52fb60() {
    return global<u32>(0x0081aebc);
}

u32 __cdecl Global81b43c_Set_52fb70(u32 v0) {
    global<u32>(0x0081b43c) = v0;
    return v0;
}

u32 __cdecl Global81b43c_Get_52fb80() {
    return global<u32>(0x0081b43c);
}

u32 __cdecl Global81af4c_Set_52fb90(u32 v0) {
    global<u32>(0x0081af4c) = v0;
    return v0;
}

u32 __cdecl Global81af4c_Get_52fbc0() {
    return global<u32>(0x0081af4c);
}

u32 __cdecl Global820a8c_Get_52fbe0() {
    return global<u32>(0x00820a8c);
}

u32 __cdecl Global879088_Set_5508b0(u32 v0) {
    global<u32>(0x00879088) = v0;
    return v0;
}

u32 __fastcall Getm10_564ee0(const u8 *o) {
    return at<u32>(o, (u32)-16);
}

void __fastcall Vtbl701260_Set_565230(u8 *o) {
    at<u32>(o, 0) = 0x00701260;
}

u32 __stdcall Const1_569d00(u32 v0) {
    return 0x1;
}

u32 __stdcall Const1_56a430(u32 v0, u32 v1, u32 v2) {
    return 0x1;
}

u32 __stdcall Const1_56a440(u32 v0, u32 v1, u32 v2, u32 v3) {
    return 0x1;
}

u8 * __fastcall Partc4_56dd60(u8 *o) {
    return o + 0xc4;
}

void __fastcall Vtbl7026e8_Set_591970(u8 *o) {
    at<u32>(o, 0) = 0x007026e8;
}

u32 __fastcall WaveletCodec_GetRefCount(const u8 *o) {
    return at<u32>(o, (u32)-76);
}

u32 __fastcall Getc_59b540(const u8 *o) {
    return at<u32>(o, 0xc);
}

void __fastcall Vtbl6fd828_Set_59b860(u8 *o) {
    at<u32>(o, 0) = 0x006fd828;
}

u8 * __fastcall BalanceFile_GetValues(u8 *o) {
    return o + 0x8;
}

u32 __cdecl Zero_5cec60() {
    return 0;
}

u32 __cdecl Zero_5d60c0() {
    return 0;
}

u32 __fastcall Get4_5dc650(const u8 *o) {
    return at<u32>(o, 0x4);
}

void __fastcall Set70_To0_5dc850(u8 *o) {
    at<u32>(o, 0x70) = 0x0;
}

void __fastcall Set14_To1_5e26f0(u8 *o) {
    at<u32>(o, 0x14) = 0x1;
}

void __fastcall Set4_To1_5e3bb0(u8 *o) {
    at<u32>(o, 0x4) = 0x1;
}

void __fastcall Set4_To0_5e3bc0(u8 *o) {
    at<u32>(o, 0x4) = 0x0;
}

u32 __cdecl Zero_66f4d0() {
    return 0;
}

u32 __cdecl Global781fc8_Get_691530() {
    return global<u32>(0x00781fc8);
}

u8 __fastcall GetBytec_695af0(const u8 *o) {
    return at<u8>(o, 0xc);
}

u8 __fastcall GetByted_695b00(const u8 *o) {
    return at<u8>(o, 0xd);
}

void __cdecl Stub_699ca0() {}

u16 __fastcall GetWordc_69c8e0(const u8 *o) {
    return at<u16>(o, 0xc);
}

u16 __fastcall GetWorde_69c8f0(const u8 *o) {
    return at<u16>(o, 0xe);
}

u32 __fastcall Get1c_6a8cc0(const u8 *o) {
    return at<u32>(o, 0x1c);
}

void __fastcall Vtbl709b44_Set_6a9180(u8 *o) {
    at<u32>(o, 0) = 0x00709b44;
}

void __fastcall Vtbl70a0a8_Set_6ad000(u8 *o) {
    at<u32>(o, 0) = 0x0070a0a8;
}

void __fastcall Vtbl70a078_Set_6add30(u8 *o) {
    at<u32>(o, 0) = 0x0070a078;
}

u32 __fastcall Get88_6adf80(const u8 *o) {
    return at<u32>(o, 0x88);
}

void __fastcall Vtbl70a078_Set_6b4a50(u8 *o) {
    at<u32>(o, 0) = 0x0070a078;
}

void __fastcall Vtbl70a258_Set_6b4a90(u8 *o) {
    at<u32>(o, 0) = 0x0070a258;
}

void __stdcall Stub_Ret8_6b70d0(u32, u32) {}

u32 __cdecl Const3_6b71c0() {
    return 0x3;
}

u32 __fastcall Set2c_6b9220(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x2c) = v0;
    return v0;
}

u32 __stdcall Const1_6b9bd0(u32 v0, u32 v1) {
    return 0x1;
}

u32 __fastcall Setc_6baa00(u8 *o, void *, u32 v0) {
    at<u32>(o, 0xc) = v0;
    return v0;
}

u32 __fastcall Set14_6bb190(u8 *o, void *, u32 v0) {
    at<u32>(o, 0x14) = v0;
    return v0;
}

u32 __fastcall Get14_6bb1a0(const u8 *o) {
    return at<u32>(o, 0x14);
}

void __fastcall Set14_To0_6bb1e0(u8 *o) {
    at<u32>(o, 0x14) = 0x0;
}

u32 __cdecl Const4_6bd790() {
    return 0x4;
}

u32 __stdcall Zero_6c54a0(u32 v0) {
    return 0;
}

u32 __stdcall Zero_6c54b0(u32 v0, u32 v1, u32 v2, u32 v3) {
    return 0;
}

u8 * __fastcall Part14_6c5590(u8 *o) {
    return o + 0x14;
}

void __fastcall Vtbl70a778_Set_6c7090(u8 *o) {
    at<u32>(o, 0) = 0x0070a778;
}

u32 __cdecl Globalfb2708_Set_6d1bc0(u32 v0) {
    global<u32>(0x00fb2708) = v0;
    return v0;
}

}
