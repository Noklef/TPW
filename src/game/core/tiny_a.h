// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x405520: the vtable pointer +0 = 0x6fd6c8.
void __fastcall Vtbl6fd6c8_Set_405520(u8 *o);
// 0x405560: the byte at 0xa.
u8 __fastcall GetBytea_405560(const u8 *o);
// 0x4056e0: the dword at 0x0.
u32 __fastcall Get0_4056e0(const u8 *o);
// 0x4145e0: the dword at 0x4f0.
u32 __fastcall Get4f0_4145e0(const u8 *o);
// 0x41ce80: the dword at 0x34.
u32 __fastcall Get34_41ce80(const u8 *o);
// 0x41ce90: the dword at 0x38.
u32 __fastcall Get38_41ce90(const u8 *o);
// 0x4217e0: the dword at 0x4.
u32 __fastcall StreamBase_GetSource(const u8 *o);
// 0x4217f0: 0x4 = value; returns it.
u32 __fastcall Set4_4217f0(u8 *o, void *, u32 v0);
// 0x424ec0: returns 0x1.
u32 __cdecl Const1_424ec0();
// 0x429b50: the vtable pointer +0 = 0x6fdce8.
void __fastcall Vtbl6fdce8_Set_429b50(u8 *o);
// 0x47eaa0: the dword at 0x7b4b88.
u32 __cdecl Global7b4b88_Get_47eaa0();
// 0x48f690: 0x4c = value; returns it.
u32 __fastcall Set4c_48f690(u8 *o, void *, u32 v0);
// 0x4a8bf0: the vtable pointer +0 = 0x6fd810.
void __fastcall Vtbl6fd810_Set_4a8bf0(u8 *o);
// 0x4bc420: the vtable pointer +0 = 0x6fefb8.
void __fastcall Vtbl6fefb8_Set_4bc420(u8 *o);
// 0x4c75f0: the vtable pointer +0 = 0x6fd838.
void __fastcall Vtbl6fd838_Set_4c75f0(u8 *o);
// 0x4d19f0: the vtable pointer +0 = 0x6fd830.
void __fastcall Vtbl6fd830_Set_4d19f0(u8 *o);
// 0x4d5260: the vtable pointer +0 = 0x6fd7b0.
void __fastcall Vtbl6fd7b0_Set_4d5260(u8 *o);
// 0x4db7c0: the vtable pointer +0 = 0x6fd7c0.
void __fastcall Vtbl6fd7c0_Set_4db7c0(u8 *o);
// 0x4dd310: the vtable pointer +0 = 0x6fd7e0.
void __fastcall Vtbl6fd7e0_Set_4dd310(u8 *o);
// 0x4e03d0: the dword at 0x64.
u32 __fastcall Get64_4e03d0(const u8 *o);
// 0x4e11f0: the vtable pointer +0 = 0x6fd7c8.
void __fastcall Vtbl6fd7c8_Set_4e11f0(u8 *o);
// 0x4e1200: the vtable pointer +0 = 0x6fd7d0.
void __fastcall Vtbl6fd7d0_Set_4e1200(u8 *o);
// 0x4e1210: the vtable pointer +0 = 0x6fd7d8.
void __fastcall Vtbl6fd7d8_Set_4e1210(u8 *o);
// 0x4e21b0: the byte at 0x190.
u8 __fastcall GetByte190_4e21b0(const u8 *o);
// 0x4e21e0: 0x198 = value; returns it.
u32 __fastcall Set198_4e21e0(u8 *o, void *, u32 v0);
// 0x4e21f0: the byte at 0x198.
u8 __fastcall GetByte198_4e21f0(const u8 *o);
// 0x4e5b90: the vtable pointer +0 = 0x700620.
void __fastcall Vtbl700620_Set_4e5b90(u8 *o);
// 0x4f6080: returns 0x7ffffffd.
u32 __cdecl Const7ffffffd_4f6080();
// 0x4f8570: the vtable pointer +0 = 0x6fd7e8.
void __fastcall Vtbl6fd7e8_Set_4f8570(u8 *o);
// 0x4f8580: the vtable pointer +0 = 0x6fd7f0.
void __fastcall Vtbl6fd7f0_Set_4f8580(u8 *o);
// 0x4f8590: the vtable pointer +0 = 0x6fd7f8.
void __fastcall Vtbl6fd7f8_Set_4f8590(u8 *o);
// 0x4fb320: the vtable pointer +0 = 0x6fd840.
void __fastcall Vtbl6fd840_Set_4fb320(u8 *o);
// 0x4ff4e0: the vtable pointer +0 = 0x7007a8.
void __fastcall Vtbl7007a8_Set_4ff4e0(u8 *o);
// 0x4ff4f0: returns 0xe.
u32 __cdecl Conste_4ff4f0();
// 0x503fe0: the vtable pointer +0 = 0x6fd808.
void __fastcall Vtbl6fd808_Set_503fe0(u8 *o);
// 0x506190: the vtable pointer +0 = 0x700868.
void __fastcall Vtbl700868_Set_506190(u8 *o);
// 0x5061a0: returns 0xf.
u32 __cdecl Constf_5061a0();
// 0x509350: the vtable pointer +0 = 0x6fd820.
void __fastcall Vtbl6fd820_Set_509350(u8 *o);
// 0x50b970: the vtable pointer +0 = 0x6fd848.
void __fastcall Vtbl6fd848_Set_50b970(u8 *o);
// 0x50c1f0: returns 0x1.
u32 __stdcall Const1_50c1f0(u32 v0);
// 0x512c30: the vtable pointer +0 = 0x6fd7a0.
void __fastcall Vtbl6fd7a0_Set_512c30(u8 *o);
// 0x512c40: the vtable pointer +0 = 0x6fd7a8.
void __fastcall Vtbl6fd7a8_Set_512c40(u8 *o);
// 0x519410: the dword at 0x785584.
u32 __cdecl Global785584_Get_519410();
// 0x519420: the dword at 0x785588.
u32 __cdecl Global785588_Get_519420();
// 0x519430: the dword at 0x78558c.
u32 __cdecl Global78558c_Get_519430();
// 0x519440: the dword at 0x785590.
u32 __cdecl Global785590_Get_519440();
// 0x51a280: the dword at 0x1da710.
u32 __fastcall Get1da710_51a280(const u8 *o);
// 0x51c050: the dword at 0x8.
u32 __fastcall Get8_51c050(const u8 *o);
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
// 0x564ee0: the dword at -0x10.
u32 __fastcall Getm10_564ee0(const u8 *o);
// 0x565230: the vtable pointer +0 = 0x701260.
void __fastcall Vtbl701260_Set_565230(u8 *o);
// 0x569d00: returns 0x1.
u32 __stdcall Const1_569d00(u32 v0);
// 0x56a430: returns 0x1.
u32 __stdcall Const1_56a430(u32 v0, u32 v1, u32 v2);
// 0x56a440: returns 0x1.
u32 __stdcall Const1_56a440(u32 v0, u32 v1, u32 v2, u32 v3);
// 0x56dd60: the address of the part at 0xc4.
u8 * __fastcall Partc4_56dd60(u8 *o);
// 0x591970: the vtable pointer +0 = 0x7026e8.
void __fastcall Vtbl7026e8_Set_591970(u8 *o);
// 0x591a70: the dword at -0x4c.
u32 __fastcall WaveletCodec_GetRefCount(const u8 *o);
// 0x59b540: the dword at 0xc.
u32 __fastcall Getc_59b540(const u8 *o);
// 0x59b860: the vtable pointer +0 = 0x6fd828.
void __fastcall Vtbl6fd828_Set_59b860(u8 *o);
// 0x5b0d60: the address of the part at 0x8.
u8 * __fastcall BalanceFile_GetValues(u8 *o);
// 0x5cec60: returns 0.
u32 __cdecl Zero_5cec60();
// 0x5d60c0: returns 0.
u32 __cdecl Zero_5d60c0();
// 0x5dc650: the dword at 0x4.
u32 __fastcall Get4_5dc650(const u8 *o);
// 0x5dc850: 0x70 = 0x0.
void __fastcall Set70_To0_5dc850(u8 *o);
// 0x5e26f0: 0x14 = 0x1.
void __fastcall Set14_To1_5e26f0(u8 *o);
// 0x5e3bb0: 0x4 = 0x1.
void __fastcall Set4_To1_5e3bb0(u8 *o);
// 0x5e3bc0: 0x4 = 0x0.
void __fastcall Set4_To0_5e3bc0(u8 *o);
// 0x66f4d0: returns 0.
u32 __cdecl Zero_66f4d0();
// 0x691530: the dword at 0x781fc8.
u32 __cdecl Global781fc8_Get_691530();
// 0x695af0: the byte at 0xc.
u8 __fastcall GetBytec_695af0(const u8 *o);
// 0x695b00: the byte at 0xd.
u8 __fastcall GetByted_695b00(const u8 *o);
// 0x699ca0: does nothing.
void __cdecl Stub_699ca0();
// 0x69c8e0: the word at 0xc.
u16 __fastcall GetWordc_69c8e0(const u8 *o);
// 0x69c8f0: the word at 0xe.
u16 __fastcall GetWorde_69c8f0(const u8 *o);
// 0x6a8cc0: the dword at 0x1c.
u32 __fastcall Get1c_6a8cc0(const u8 *o);
// 0x6a9180: the vtable pointer +0 = 0x709b44.
void __fastcall Vtbl709b44_Set_6a9180(u8 *o);
// 0x6ad000: the vtable pointer +0 = 0x70a0a8.
void __fastcall Vtbl70a0a8_Set_6ad000(u8 *o);
// 0x6add30: the vtable pointer +0 = 0x70a078.
void __fastcall Vtbl70a078_Set_6add30(u8 *o);
// 0x6adf80: the dword at 0x88.
u32 __fastcall Get88_6adf80(const u8 *o);
// 0x6b4a50: the vtable pointer +0 = 0x70a078.
void __fastcall Vtbl70a078_Set_6b4a50(u8 *o);
// 0x6b4a90: the vtable pointer +0 = 0x70a258.
void __fastcall Vtbl70a258_Set_6b4a90(u8 *o);
// 0x6b70d0: does nothing (pops 8 bytes).
void __stdcall Stub_Ret8_6b70d0(u32 v0, u32 v1);
// 0x6b71c0: returns 0x3.
u32 __cdecl Const3_6b71c0();
// 0x6b9220: 0x2c = value; returns it.
u32 __fastcall Set2c_6b9220(u8 *o, void *, u32 v0);
// 0x6b9bd0: returns 0x1.
u32 __stdcall Const1_6b9bd0(u32 v0, u32 v1);
// 0x6baa00: 0xc = value; returns it.
u32 __fastcall Setc_6baa00(u8 *o, void *, u32 v0);
// 0x6bb190: 0x14 = value; returns it.
u32 __fastcall Set14_6bb190(u8 *o, void *, u32 v0);
// 0x6bb1a0: the dword at 0x14.
u32 __fastcall Get14_6bb1a0(const u8 *o);
// 0x6bb1e0: 0x14 = 0x0.
void __fastcall Set14_To0_6bb1e0(u8 *o);
// 0x6bd790: returns 0x4.
u32 __cdecl Const4_6bd790();
// 0x6c54a0: returns 0.
u32 __stdcall Zero_6c54a0(u32 v0);
// 0x6c54b0: returns 0.
u32 __stdcall Zero_6c54b0(u32 v0, u32 v1, u32 v2, u32 v3);
// 0x6c5590: the address of the part at 0x14.
u8 * __fastcall Part14_6c5590(u8 *o);
// 0x6c7090: the vtable pointer +0 = 0x70a778.
void __fastcall Vtbl70a778_Set_6c7090(u8 *o);
// 0x6d1bc0: 0xfb2708 = value; returns it.
u32 __cdecl Globalfb2708_Set_6d1bc0(u32 v0);
}
