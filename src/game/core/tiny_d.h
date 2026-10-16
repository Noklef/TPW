// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47f910: stores +0x0 = a value, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0, +0x14 = 0x0, +0x18 = 0x0, +0x1c = 0x0, +0x20 = 0x0, +0x24 = 0x0, +0x28 = 0x0, +0x2c = 0x0.
u32 __fastcall Fn_47f910(u8 *o, void *, u32 v0);
// 0x48c9e0: stores +0x0 = 0x6ff1f0, 0x7c2658 = 0x0, +0x10 = 0x0, +0x18 = 0x0, +0x14 = 0x0, +0x1c = 0x0.
u32 __fastcall Fn_48c9e0(u8 *o);
// 0x4e0210: stores +0x5e = 0x0, +0x60 = 0x0.
u32 __fastcall Fn_4e0210(u8 *o);
// 0x4e0de0: stores +0x30 = argument.
u16 __fastcall Fn_4e0de0(u8 *o, void *, u32 v0);
// 0x4f0150: stores +0x0 = a value, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_4f0150(u8 *o, void *, u32 v0);
// 0x4f6010: returns a constant.
u8 __cdecl Fn_4f6010();
// 0x509ae0: stores +0x60 = argument.
u16 __fastcall Fn_509ae0(u8 *o, void *, u32 v0);
// 0x50b080: stores +0x0 = argument.
u16 __fastcall Fn_50b080(u8 *o, void *, u32 v0);
// 0x52c2a0: stores 0x818c54 = 0x0.
void __cdecl Fn_52c2a0();
// 0x52c6c0: stores +0x0 = a value, +0x0 = a value, +0x0 = a value, +0x0 = a value.
u32 __cdecl Fn_52c6c0(u32 v0, u32 v1, u32 v2, u32 v3);
// 0x52fbd0: stores 0x820a8c = 0x0.
void __cdecl Fn_52fbd0();
// 0x564730: stores +0xc = argument, +0x10 = argument.
u32 __fastcall Fn_564730(u8 *o, void *, u32 v0, u32 v1);
// 0x56b620: returns a field or global.
u32 __stdcall Fn_56b620(u32 v0);
// 0x5ce530: stores +0x0 = a value.
u32 __fastcall Fn_5ce530(u8 *o, void *, u32 v0);
// 0x5cec50: returns a field or global.
u32 __fastcall Fn_5cec50(u8 *o, void *, u32 v0, u32 v1);
// 0x5d0e00: stores +0xc = argument, +0x10 = argument, +0x1c = argument, +0x20 = argument, +0x2c = argument, +0x30 = argument.
u32 __fastcall Fn_5d0e00(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);
// 0x5d1400: returns a constant.
u8 __cdecl Fn_5d1400();
// 0x5d1420: stores +0x0 = argument, +0x0 = argument.
u32 __stdcall Fn_5d1420(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5, u32 v6);
// 0x5d29b0: stores +0x0 = 0x700620, 0xf827ec = 0x774178, 0xf827f0 = 0x7741e0.
u32 __fastcall Fn_5d29b0(u8 *o);
// 0x5d5f70: stores +0x18 = 0x0, +0x1c = 0x0, +0x20 = 0x0, +0x28 = 0x0.
u32 __fastcall Fn_5d5f70(u8 *o);
// 0x5e1bb0: stores +0x28 = argument, +0x2c = argument, +0x30 = argument.
u32 __fastcall Fn_5e1bb0(u8 *o, void *, u32 v0, u32 v1, u32 v2);
// 0x5ebe30: stores +0x0 = argument, +0x4 = argument, +0x8 = argument.
u32 __fastcall Fn_5ebe30(u8 *o, void *, u32 v0, u32 v1, u32 v2);
// 0x5ebef0: stores +0x0 = 0x0, +0x1 = 0x0, +0x2 = 0x0, +0x3 = 0x0, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0.
u32 __fastcall Fn_5ebef0(u8 *o);
// 0x5eda40: stores 0xf86988 = argument, 0xf8698c = argument, 0xf86990 = argument, 0xf86994 = argument.
u32 __cdecl Fn_5eda40(u32 v0, u32 v1);
// 0x64a450: stores +0x0 = a value, +0x0 = a value, +0x0 = 0x77a0a0, +0x0 = 0x77b0a0.
u32 __cdecl Fn_64a450(u32 v0, u32 v1, u32 v2, u32 v3);
// 0x64dd60: stores +0x0 = 0x64ddec, +0x4 = 0x64de4f, +0x8 = 0x64de13, +0xc = 0x64dead, +0x10 = 0x64dff8, +0x68 = 0x0, +0x6c = 0x0, +0x14 = 0x0, +0x70 = 0x7054b8, +0x74 = 0x77, +0x78 = 0x0, +0x7c = 0x0, +0x80 = 0x0.
u32 __cdecl Fn_64dd60(u32 v0);
// 0x657d30: stores +0x0 = 0x7058d8, +0xc = 0x0, +0x4 = 0x0, +0x6 = 0x2, +0x8 = 0x2.
u32 __fastcall Fn_657d30(u8 *o);
// 0x6590c3: stores 0xfaa5d4 = argument.
u32 __cdecl Fn_6590c3(u32 v0);
// 0x6590ea: stores 0xfaa5fc = argument.
u32 __cdecl Fn_6590ea(u32 v0);
// 0x6593ff: returns a field or global.
u32 __cdecl Fn_6593ff();
// 0x6595a8: returns a field or global.
u16 __cdecl Fn_6595a8();
// 0x659770: stores +0x0 = 0x7058f0.
u32 __fastcall Fn_659770(u8 *o);
// 0x659ce0: stores +0x10 = 0x1.
u32 __fastcall Fn_659ce0(u8 *o);
// 0x65a9d0: stores +0x0 = 0x705938.
u32 __fastcall Fn_65a9d0(u8 *o);
// 0x65aa80: returns a field or global.
u32 __fastcall Fn_65aa80(u8 *o, void *, u32 v0);
// 0x65b850: returns a field or global.
u32 __fastcall Fn_65b850(const u8 *o);
// 0x65bc70: returns a field or global.
u32 __fastcall Fn_65bc70(const u8 *o);
// 0x65d547: returns a field or global.
u32 __fastcall Fn_65d547(u8 *o, void *, u32 v0);
// 0x65d55a: returns a field or global.
u32 __fastcall Fn_65d55a(const u8 *o);
// 0x65d56b: returns a field or global.
u32 __fastcall Fn_65d56b(const u8 *o);
// 0x65d57c: returns a field or global.
u32 __fastcall Fn_65d57c(const u8 *o);
// 0x65d7bc: returns a field or global.
u32 __fastcall Fn_65d7bc(const u8 *o);
// 0x6617d0: stores +0x0 = 0x705938.
u32 __fastcall Fn_6617d0(u8 *o, void *, u32 v0);
// 0x6617f0: stores +0x12 = 0x1.
u32 __fastcall Fn_6617f0(u8 *o);
// 0x661830: stores +0x90 = argument.
u32 __fastcall Fn_661830(u8 *o, void *, u32 v0);
// 0x661850: stores +0x94 = argument.
u32 __fastcall Fn_661850(u8 *o, void *, u32 v0);
// 0x661870: stores +0x0 = 0x7059b0, +0x5 = 0x0, +0x6 = 0x0, +0x4 = 0x1, +0x10 = 0x0.
u32 __fastcall Fn_661870(u8 *o);
// 0x6618a6: stores +0x0 = 0x7059b0.
u32 __fastcall Fn_6618a6(u8 *o);
// 0x6618e6: stores +0x4 = 0x1.
u32 __fastcall Fn_6618e6(u8 *o);
// 0x6618f8: stores +0x4 = 0x0.
u32 __fastcall Fn_6618f8(u8 *o);
// 0x661c0b: stores +0x0 = 0x0, +0x4 = 0x0.
u32 __fastcall Fn_661c0b(u8 *o);
// 0x661f9b: stores +0x14 = 0x1.
u32 __cdecl Fn_661f9b(u32 v0);
// 0x662086: returns a field or global.
u32 __fastcall Fn_662086(const u8 *o);
// 0x662097: stores +0x10 = 0x0.
u32 __fastcall Fn_662097(u8 *o);
// 0x6620cc: stores +0x8 = argument.
u32 __fastcall Fn_6620cc(u8 *o, void *, u32 v0);
// 0x662411: stores 0xfaa638 = 0x1.
void __cdecl Fn_662411();
// 0x667801: stores +0x140 = argument.
u32 __fastcall Fn_667801(u8 *o, void *, u32 v0);
// 0x66781a: stores +0x144 = argument.
u32 __fastcall Fn_66781a(u8 *o, void *, u32 v0);
// 0x668d84: returns a field or global.
u32 __fastcall Fn_668d84(const u8 *o);
// 0x6693d0: stores +0x140 = argument.
u32 __fastcall Fn_6693d0(u8 *o, void *, u32 v0);
// 0x6693f0: stores +0x13c = argument.
u32 __fastcall Fn_6693f0(u8 *o, void *, u32 v0);
// 0x669700: stores +0x0 = 0x0, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_669700(u8 *o);
// 0x669c30: returns a field or global.
u32 __fastcall Fn_669c30(const u8 *o);
// 0x669c41: stores +0x4 = 0x0, +0x0 = 0x0.
u32 __fastcall Fn_669c41(u8 *o);
// 0x669f80: stores +0x6 = 0x1.
u32 __fastcall Fn_669f80(u8 *o);
// 0x669f94: stores +0x6 = 0x0.
u32 __fastcall Fn_669f94(u8 *o);
// 0x669fa8: stores +0x0 = 0x705a80, +0x4 = 0x1, +0x6 = 0x0, +0x14 = 0x0, +0x10 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_669fa8(u8 *o);
// 0x669fee: stores +0x0 = 0x705a80.
u32 __fastcall Fn_669fee(u8 *o);
// 0x66a002: stores +0x4 = 0x1.
u32 __fastcall Fn_66a002(u8 *o);
// 0x66a281: returns a field or global.
u32 __fastcall Fn_66a281(const u8 *o);
// 0x66a6c7: returns a field or global.
u32 __fastcall Fn_66a6c7(const u8 *o);
// 0x66b469: returns a field or global.
u32 __fastcall Fn_66b469(const u8 *o);
// 0x66b53e: stores +0x148 = argument.
u32 __fastcall Fn_66b53e(u8 *o, void *, u32 v0);
// 0x66bff7: stores +0x0 = 0x705938.
u32 __fastcall Fn_66bff7(u8 *o);
// 0x66c922: returns a field or global.
u32 __fastcall Fn_66c922(const u8 *o);
// 0x66cc7c: returns a field or global.
u32 __fastcall Fn_66cc7c(const u8 *o);
// 0x66d950: returns a field or global.
u32 __fastcall Fn_66d950(const u8 *o);
// 0x66dad2: returns a field or global.
u32 __fastcall Fn_66dad2(const u8 *o);
// 0x67a610: stores 0x77eca4 = 0x6805d0, 0x77eca0 = 0x680a30, 0x77eca8 = 0x680660, 0x77ecac = 0x680570, 0x77ecb0 = 0x680640, 0x77ecb4 = 0x680a30.
u32 __cdecl Fn_67a610();
// 0x682440: stores +0x50 = 0x781490, +0x14 = 0x1.
u32 __cdecl Fn_682440(u32 v0);
// 0x68bda0: stores +0x0 = 0x0, +0x4 = 0x0, +0x8 = 0x0.
u32 __cdecl Fn_68bda0(u32 v0);
// 0x6a65e0: returns a constant.
u8 __cdecl Fn_6a65e0();
// 0x6b52e0: stores +0x0 = argument, +0x4 = argument, +0x8 = argument, +0xc = argument, +0x10 = argument, +0x14 = argument.
u32 __fastcall Fn_6b52e0(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);
// 0x6b5310: stores +0x0 = argument, +0x4 = argument, +0x8 = argument, +0xc = argument, +0x10 = argument, +0x14 = argument.
u32 __fastcall Fn_6b5310(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);
// 0x6b5430: stores +0x0 = argument, +0x4 = 0x0, +0x5 = argument, +0x8 = 0x0.
u32 __fastcall Fn_6b5430(u8 *o, void *, u32 v0, u32 v1);
// 0x6bb220: stores +0x0 = argument, +0x4 = argument, +0x8 = argument.
u32 __fastcall Fn_6bb220(u8 *o, void *, u32 v0, u32 v1, u32 v2);
// 0x6c0200: stores +0x0 = 0x0.
u32 __stdcall Fn_6c0200(u32 v0, u32 v1);
// 0x6c3900: returns a field or global.
u32 __cdecl Fn_6c3900(u32 v0);
// 0x6c54c0: stores +0x0 = the object, +0x0 = 0x0.
u32 __fastcall Fn_6c54c0(u8 *o, void *, u32 v0, u32 v1);
// 0x6c58d0: stores +0x0 = argument, +0xc = argument.
u32 __fastcall Fn_6c58d0(u8 *o, void *, u32 v0, u32 v1);
// 0x6c6a00: stores +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x14 = argument, +0x0 = 0x70a678, +0x1c = a value, +0x20 = a value, +0x0 = 0x70aba8.
u32 __fastcall Fn_6c6a00(u8 *o, void *, u32 v0, u32 v1);
// 0x6d1a50: stores +0x2468 = a value, +0x2430 = 0x0, +0x242c = 0x0, +0x2464 = 0x0, +0x2410 = 0x0.
u32 __fastcall Fn_6d1a50(u8 *o);
// 0x6d2780: stores +0x0 = 0x0.
u32 __stdcall Fn_6d2780(u32 v0, u32 v1);
// 0x6d3b90: stores +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0, +0x14 = 0x0, +0x18 = 0x0.
u32 __fastcall Fn_6d3b90(u8 *o);
}
