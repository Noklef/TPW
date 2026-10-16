// Tiny functions whose whole body is one of a few shapes, written out by tools/tinygen.py from the
// instruction listing (0x401000-0x7f0000). Named by what they do and where they are; what they are for
// is not known yet. A setter returns the value it was given, as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x402f30: sets +0x8, +0x0, +0xc; returns the object.
u8 * __fastcall Init_402f30(u8 *o);
// 0x4053b0: sets +0xa, +0xb, +0x9; returns the object.
u8 * __fastcall CheatSystem_Construct(u8 *o);
// 0x421510: sets +0x0, +0x8, +0x4; returns the object.
u8 * __fastcall Init_421510(u8 *o);
// 0x4cf030: sets +0x4, +0x11, +0x0, +0x8, +0x10, +0xc; returns the object.
u8 * __fastcall Init_4cf030(u8 *o);
// 0x4d0f70: sets +0x0, +0x20, +0x24, +0x28, +0x2c; returns the object.
u8 * __fastcall Init_4d0f70(u8 *o);
// 0x4d3ed0: sets +0x0, +0x4, +0x8; returns the object.
u8 * __fastcall Init_4d3ed0(u8 *o);
// 0x4e0370: 1 when the dword at 0x19c is 0x1, else 0.
u32 __fastcall Is19c_Eq1_4e0370(const u8 *o);
// 0x4e0380: 1 when the dword at 0x19c is 0x4, else 0.
u32 __fastcall Is19c_Eq4_4e0380(const u8 *o);
// 0x4e03e0: 1 when the dword at 0x19c is 0x2, else 0.
u32 __fastcall Is19c_Eq2_4e03e0(const u8 *o);
// 0x4e0440: 1 when the dword at 0x68 is 0, else 0.
u32 __fastcall Is68_Zero_4e0440(const u8 *o);
// 0x4ef830: sets +0x24, +0x0; returns the object.
u8 * __fastcall Init_4ef830(u8 *o);
// 0x4f06a0: returns the word 0xffff.
u16 __stdcall WordConstffff_4f06a0(u32 v0);
// 0x4f06b0: returns the word 0xffff.
u16 __cdecl WordConstffff_4f06b0();
// 0x4f1890: sets +0x4, +0x0; returns the object.
u8 * __fastcall Init_4f1890(u8 *o, void *, u32 v0);
// 0x4f1bd0: sets +0x24, +0x0; returns the object.
u8 * __fastcall Init_4f1bd0(u8 *o);
// 0x513290: sets +0x0, +0x4, +0xc; returns the object.
u8 * __fastcall Init_513290(u8 *o);
// 0x5ac510: sets +0x4, +0x8, +0xc, +0x0; returns the object.
u8 * __fastcall Init_5ac510(u8 *o);
// 0x5cfd80: sets +0x24, +0x0; returns the object.
u8 * __fastcall Init_5cfd80(u8 *o);
// 0x5d36c0: sets +0x4, +0x0; returns the object.
u8 * __fastcall Init_5d36c0(u8 *o);
// 0x5ec210: sets +0x0, +0x4, +0x8, +0xc; returns the object.
u8 * __fastcall Init_5ec210(u8 *o);
// 0x691ee0: sets +0x0; returns the object.
u8 * __fastcall Init_691ee0(u8 *o, void *, u32 v0);
// 0x692970: sets +0x0; returns the object.
u8 * __fastcall Init_692970(u8 *o, void *, u32 v0);
// 0x69fef0: sets +0x4, +0x0; returns the object.
u8 * __fastcall Init_69fef0(u8 *o, void *, u32 v0);
// 0x6a8cd0: *argument = the dword at 0x20; returns the pointer.
u32 __fastcall CopyTo_20_6a8cd0(u8 *o, void *, u32 v0);
// 0x6a8ce0: *argument = the dword at 0x24; returns the pointer.
u32 __fastcall CopyTo_24_6a8ce0(u8 *o, void *, u32 v0);
// 0x6b4a00: *first argument = 0xffffffff; returns that pointer.
u32 __stdcall StoreThrough_ffffffff_6b4a00(u32 v0, u32 v1, u32 v2);
// 0x6b4a60: sets +0x0; returns the object.
u8 * __fastcall Init_6b4a60(u8 *o);
// 0x6b9c50: sets +0x0, +0x4, +0x40, +0x48, +0x4c, +0x68; returns the object.
u8 * __fastcall Init_6b9c50(u8 *o);
// 0x6b9c70: +0x50 = value; the flags at +0x4c |= 0x4; returns the flags.
u32 __fastcall SetThenFlag50_4c_6b9c70(u8 *o, void *, u32 v0);
// 0x6b9cb0: +0x60 = value; the flags at +0x4c |= 0x20; returns the flags.
u32 __fastcall SetThenFlag60_4c_6b9cb0(u8 *o, void *, u32 v0);
// 0x6b9cf0: +0x40 = value; the flags at +0x4c |= 0x80; returns the flags.
u32 __fastcall SetThenFlag40_4c_6b9cf0(u8 *o, void *, u32 v0);
// 0x6b9d10: +0x44 = value; the flags at +0x4c |= 0x100; returns the flags.
u32 __fastcall SetThenFlag44_4c_6b9d10(u8 *o, void *, u32 v0);
// 0x6b9d30: +0x68 = value; the flags at +0x4c |= 0x200; returns the flags.
u32 __fastcall SetThenFlag68_4c_6b9d30(u8 *o, void *, u32 v0);
// 0x6c47b0: sets +0x0, +0x4; returns the object.
u8 * __fastcall Init_6c47b0(u8 *o);
// 0x6c4fb0: sets +0x4, +0x8, +0xc, +0x0, +0x14, +0x18; returns the object.
u8 * __fastcall Init_6c4fb0(u8 *o);
// 0x6c4fd0: sets +0x0, +0x4, +0x8; returns the object.
u8 * __fastcall Init_6c4fd0(u8 *o, void *, u32 v0);
// 0x6c5090: +0x48 = value; the flags at +0x14 |= 0x1000; returns the flags.
u32 __fastcall SetThenFlag48_14_6c5090(u8 *o, void *, u32 v0);
// 0x6c50b0: +0x8 = value; the flags at +0x14 |= 0x4; returns the flags.
u32 __fastcall SetThenFlag8_14_6c50b0(u8 *o, void *, u32 v0);
// 0x6c50d0: +0x9c = value; the flags at +0x14 |= 0x80000; returns the flags.
u32 __fastcall SetThenFlag9c_14_6c50d0(u8 *o, void *, u32 v0);
// 0x6c50f0: +0x4c = value; the flags at +0x14 |= 0x800; returns the flags.
u32 __fastcall SetThenFlag4c_14_6c50f0(u8 *o, void *, u32 v0);
// 0x6c5110: +0x44 = value; the flags at +0x14 |= 0x40; returns the flags.
u32 __fastcall SetThenFlag44_14_6c5110(u8 *o, void *, u32 v0);
// 0x6c5130: +0x28 = value; the flags at +0x14 |= 0x2; returns the flags.
u32 __fastcall SetThenFlag28_14_6c5130(u8 *o, void *, u32 v0);
// 0x6c5150: +0x2c = value; the flags at +0x14 |= 0x400; returns the flags.
u32 __fastcall SetThenFlag2c_14_6c5150(u8 *o, void *, u32 v0);
// 0x6c5170: +0x24 = value; the flags at +0x14 |= 0x20; returns the flags.
u32 __fastcall SetThenFlag24_14_6c5170(u8 *o, void *, u32 v0);
// 0x6c51d0: the flags at +0x14 |= 0x10000; +0x5c = value; returns it.
u32 __fastcall FlagThenSet14_5c_6c51d0(u8 *o, void *, u32 v0);
// 0x6c5220: the flags at +0x14 |= 0x200000; +0x58 = value; returns it.
u32 __fastcall FlagThenSet14_58_6c5220(u8 *o, void *, u32 v0);
// 0x6c6d30: sets +0x4, +0x8, +0xc, +0x0; returns the object.
u8 * __fastcall Init_6c6d30(u8 *o);
// 0x6c9260: sets +0x243c, +0x0; returns the object.
u8 * __fastcall Init_6c9260(u8 *o);
// 0x6d2790: *first argument = 0xffffffff; returns that pointer.
u32 __stdcall StoreThrough_ffffffff_6d2790(u32 v0, u32 v1, u32 v2, u32 v3, u32 v4);
// 0x6d27a0: *first argument = 0xffffffff; returns that pointer.
u32 __stdcall StoreThrough_ffffffff_6d27a0(u32 v0);
// 0x6d29e0: *first argument = 0xffffffff; returns that pointer.
u32 __stdcall StoreThrough_ffffffff_6d29e0(u32 v0, u32 v1, u32 v2, u32 v3);
// 0x6d6c50: sets +0x4, +0x8, +0xc, +0x10, +0x0; returns the object.
u8 * __fastcall Init_6d6c50(u8 *o);
}
