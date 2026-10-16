// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x409cd0: stores +0x0 = argument.
u32 __fastcall Fn_409cd0(u8 *o, void *, u32 v0);
// 0x4d2a60: stores +0x0 = a value.
u32 __fastcall Fn_4d2a60(u8 *o, void *, u32 v0);
// 0x4e0df0: stores +0x0 = a value.
u32 __fastcall Fn_4e0df0(u8 *o, void *, u32 v0);
// 0x59b550: stores +0x0 = a value, +0xc = a value, +0x10 = a value, +0x14 = a value, +0x4 = a value, +0x8 = a value, +0x15 = a value, +0x16 = a value.
u32 __fastcall Fn_59b550(u8 *o, void *, u32 v0);
// 0x5f0970: stores +0x0 = argument, +0x1 = argument, +0x2 = argument, +0x3 = argument, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0.
u32 __fastcall Fn_5f0970(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);
// 0x657ddd: returns a field or global.
u16 __fastcall Fn_657ddd(const u8 *o);
// 0x659710: returns a field or global.
u8 __fastcall Fn_659710(const u8 *o);
// 0x659730: returns a field or global.
u16 __fastcall Fn_659730(const u8 *o);
// 0x659790: returns a field or global.
u16 __fastcall Fn_659790(const u8 *o);
// 0x659994: stores +0x8 = argument.
u32 __fastcall Fn_659994(u8 *o, void *, u32 v0);
// 0x6599ac: stores +0x2c = argument, +0x2e = argument, +0x30 = argument.
u16 __fastcall Fn_6599ac(u8 *o, void *, u32 v0, u32 v1, u32 v2);
// 0x6599da: stores +0x32 = argument.
u32 __fastcall Fn_6599da(u8 *o, void *, u32 v0);
// 0x659a40: stores +0xe = argument.
u32 __fastcall Fn_659a40(u8 *o, void *, u32 v0);
// 0x659dac: stores +0x4 = argument, +0x6 = argument, +0x8 = argument, +0xa = argument.
u32 __fastcall Fn_659dac(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);
// 0x65f330: returns a field or global.
u16 __fastcall Fn_65f330(const u8 *o);
// 0x6618c7: stores +0x5 = argument, +0x6 = argument.
u8 __fastcall Fn_6618c7(u8 *o, void *, u32 v0, u32 v1);
// 0x66190a: returns a field or global.
u8 __fastcall Fn_66190a(const u8 *o);
// 0x668564: stores +0x161 = argument.
u32 __fastcall Fn_668564(u8 *o, void *, u32 v0);
// 0x668800: returns a field or global.
u8 __fastcall Fn_668800(const u8 *o);
// 0x66a061: stores +0x8 = argument, +0xa = argument.
u16 __fastcall Fn_66a061(u8 *o, void *, u32 v0, u32 v1);
// 0x66b454: returns a field or global.
u16 __fastcall Fn_66b454(const u8 *o);
// 0x66b523: stores +0x14c = argument.
u32 __fastcall Fn_66b523(u8 *o, void *, u32 v0);
// 0x66c096: stores +0x4 = argument, +0x6 = argument, +0x8 = argument.
u16 __fastcall Fn_66c096(u8 *o, void *, u32 v0, u32 v1, u32 v2);
// 0x66c1f3: stores +0x4 = argument, +0x6 = argument, +0x8 = argument, +0xa = argument.
u32 __fastcall Fn_66c1f3(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);
// 0x6d2770: stores +0x0 = 0x0.
u8 __stdcall Fn_6d2770(u32 v0);
}
