// Constructors and initialisers that only store constants and arguments into fields and return the object (or store a value through a pointer). Reimplementation of 0x402f30, 0x421510, 0x47f910, 0x48c9e0, 0x4cf030, 0x4d0f70, 0x4d3ed0, 0x4e0210, 0x4ef830, 0x4f0150, 0x4f1890, 0x4f1bd0, 0x513290, 0x52c6c0 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x402f30: sets +0x8, +0x0, +0xc; returns the object.
u8 * __fastcall Init_402f30(u8 *o);

// 0x421510: sets +0x0, +0x8, +0x4; returns the object.
u8 * __fastcall Init_421510(u8 *o);

// 0x4cf030: sets +0x4, +0x11, +0x0, +0x8, +0x10, +0xc; returns the object.
u8 * __fastcall Init_4cf030(u8 *o);

// 0x4d0f70: sets +0x0, +0x20, +0x24, +0x28, +0x2c; returns the object.
u8 * __fastcall Init_4d0f70(u8 *o);

// 0x4d3ed0: sets +0x0, +0x4, +0x8; returns the object.
u8 * __fastcall Init_4d3ed0(u8 *o);

// 0x4ef830: sets +0x24, +0x0; returns the object.
u8 * __fastcall Init_4ef830(u8 *o);

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

// 0x6b4a60: sets +0x0; returns the object.
u8 * __fastcall Init_6b4a60(u8 *o);

// 0x6b9c50: sets +0x0, +0x4, +0x40, +0x48, +0x4c, +0x68; returns the object.
u8 * __fastcall Init_6b9c50(u8 *o);

// 0x6c47b0: sets +0x0, +0x4; returns the object.
u8 * __fastcall Init_6c47b0(u8 *o);

// 0x6c4fb0: sets +0x4, +0x8, +0xc, +0x0, +0x14, +0x18; returns the object.
u8 * __fastcall Init_6c4fb0(u8 *o);

// 0x6c4fd0: sets +0x0, +0x4, +0x8; returns the object.
u8 * __fastcall Init_6c4fd0(u8 *o, void *, u32 v0);

// 0x6c6d30: sets +0x4, +0x8, +0xc, +0x0; returns the object.
u8 * __fastcall Init_6c6d30(u8 *o);

// 0x6c9260: sets +0x243c, +0x0; returns the object.
u8 * __fastcall Init_6c9260(u8 *o);

// 0x6d6c50: sets +0x4, +0x8, +0xc, +0x10, +0x0; returns the object.
u8 * __fastcall Init_6d6c50(u8 *o);

// 0x47f910: stores +0x0 = a value, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0, +0x14 = 0x0, +0x18 = 0x0, +0x1c = 0x0, +0x20 = 0x0, +0x24 = 0x0, +0x28 = 0x0, +0x2c = 0x0.
u32 __fastcall Fn_47f910(u8 *o, void *, u32 v0);

// 0x48c9e0: stores +0x0 = 0x6ff1f0, 0x7c2658 = 0x0, +0x10 = 0x0, +0x18 = 0x0, +0x14 = 0x0, +0x1c = 0x0.
u32 __fastcall Fn_48c9e0(u8 *o);

// 0x4e0210: stores +0x5e = 0x0, +0x60 = 0x0.
u32 __fastcall Fn_4e0210(u8 *o);

// 0x4f0150: stores +0x0 = a value, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_4f0150(u8 *o, void *, u32 v0);

// 0x52c6c0: stores +0x0 = a value, +0x0 = a value, +0x0 = a value, +0x0 = a value.
u32 __cdecl Fn_52c6c0(u32 v0, u32 v1, u32 v2, u32 v3);

// 0x564730: stores +0xc = argument, +0x10 = argument.
u32 __fastcall Fn_564730(u8 *o, void *, u32 v0, u32 v1);

// 0x5d0e00: stores +0xc = argument, +0x10 = argument, +0x1c = argument, +0x20 = argument, +0x2c = argument, +0x30 = argument.
u32 __fastcall Fn_5d0e00(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);

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

// 0x659770: stores +0x0 = 0x7058f0.
u32 __fastcall Fn_659770(u8 *o);

// 0x65a9d0: stores +0x0 = 0x705938.
u32 __fastcall Fn_65a9d0(u8 *o);

// 0x6617d0: stores +0x0 = 0x705938.
u32 __fastcall Fn_6617d0(u8 *o, void *, u32 v0);

// 0x661870: stores +0x0 = 0x7059b0, +0x5 = 0x0, +0x6 = 0x0, +0x4 = 0x1, +0x10 = 0x0.
u32 __fastcall Fn_661870(u8 *o);

// 0x6618a6: stores +0x0 = 0x7059b0.
u32 __fastcall Fn_6618a6(u8 *o);

// 0x661c0b: stores +0x0 = 0x0, +0x4 = 0x0.
u32 __fastcall Fn_661c0b(u8 *o);

// 0x669700: stores +0x0 = 0x0, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_669700(u8 *o);

// 0x669c41: stores +0x4 = 0x0, +0x0 = 0x0.
u32 __fastcall Fn_669c41(u8 *o);

// 0x669fa8: stores +0x0 = 0x705a80, +0x4 = 0x1, +0x6 = 0x0, +0x14 = 0x0, +0x10 = 0x0, +0xc = 0x0.
u32 __fastcall Fn_669fa8(u8 *o);

// 0x669fee: stores +0x0 = 0x705a80.
u32 __fastcall Fn_669fee(u8 *o);

// 0x66bff7: stores +0x0 = 0x705938.
u32 __fastcall Fn_66bff7(u8 *o);

// 0x67a610: stores 0x77eca4 = 0x6805d0, 0x77eca0 = 0x680a30, 0x77eca8 = 0x680660, 0x77ecac = 0x680570, 0x77ecb0 = 0x680640, 0x77ecb4 = 0x680a30.
u32 __cdecl Fn_67a610();

// 0x682440: stores +0x50 = 0x781490, +0x14 = 0x1.
u32 __cdecl Fn_682440(u32 v0);

// 0x68bda0: stores +0x0 = 0x0, +0x4 = 0x0, +0x8 = 0x0.
u32 __cdecl Fn_68bda0(u32 v0);

// 0x6b52e0: stores +0x0 = argument, +0x4 = argument, +0x8 = argument, +0xc = argument, +0x10 = argument, +0x14 = argument.
u32 __fastcall Fn_6b52e0(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);

// 0x6b5310: stores +0x0 = argument, +0x4 = argument, +0x8 = argument, +0xc = argument, +0x10 = argument, +0x14 = argument.
u32 __fastcall Fn_6b5310(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3, u32 v4, u32 v5);

// 0x6b5430: stores +0x0 = argument, +0x4 = 0x0, +0x5 = argument, +0x8 = 0x0.
u32 __fastcall Fn_6b5430(u8 *o, void *, u32 v0, u32 v1);

// 0x6bb220: stores +0x0 = argument, +0x4 = argument, +0x8 = argument.
u32 __fastcall Fn_6bb220(u8 *o, void *, u32 v0, u32 v1, u32 v2);

// 0x6c54c0: stores +0x0 = the object, +0x0 = 0x0.
u32 __fastcall Fn_6c54c0(u8 *o, void *, u32 v0, u32 v1);

// 0x6c58d0: stores +0x0 = argument, +0xc = argument.
u32 __fastcall Fn_6c58d0(u8 *o, void *, u32 v0, u32 v1);

// 0x6c6a00: stores +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x14 = argument, +0x0 = 0x70a678, +0x1c = a value, +0x20 = a value, +0x0 = 0x70aba8.
u32 __fastcall Fn_6c6a00(u8 *o, void *, u32 v0, u32 v1);

// 0x6d1a50: stores +0x2468 = a value, +0x2430 = 0x0, +0x242c = 0x0, +0x2464 = 0x0, +0x2410 = 0x0.
u32 __fastcall Fn_6d1a50(u8 *o);

// 0x6d3b90: stores +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0, +0x14 = 0x0, +0x18 = 0x0.
u32 __fastcall Fn_6d3b90(u8 *o);

// 0x59b550: stores +0x0 = a value, +0xc = a value, +0x10 = a value, +0x14 = a value, +0x4 = a value, +0x8 = a value, +0x15 = a value, +0x16 = a value.
u32 __fastcall Fn_59b550(u8 *o, void *, u32 v0);

// 0x5f0970: stores +0x0 = argument, +0x1 = argument, +0x2 = argument, +0x3 = argument, +0x4 = 0x0, +0x8 = 0x0, +0xc = 0x0, +0x10 = 0x0.
u32 __fastcall Fn_5f0970(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);

// 0x6599ac: stores +0x2c = argument, +0x2e = argument, +0x30 = argument.
u16 __fastcall Fn_6599ac(u8 *o, void *, u32 v0, u32 v1, u32 v2);

// 0x659dac: stores +0x4 = argument, +0x6 = argument, +0x8 = argument, +0xa = argument.
u32 __fastcall Fn_659dac(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);

// 0x6618c7: stores +0x5 = argument, +0x6 = argument.
u8 __fastcall Fn_6618c7(u8 *o, void *, u32 v0, u32 v1);

// 0x66a061: stores +0x8 = argument, +0xa = argument.
u16 __fastcall Fn_66a061(u8 *o, void *, u32 v0, u32 v1);

// 0x66c096: stores +0x4 = argument, +0x6 = argument, +0x8 = argument.
u16 __fastcall Fn_66c096(u8 *o, void *, u32 v0, u32 v1, u32 v2);

// 0x66c1f3: stores +0x4 = argument, +0x6 = argument, +0x8 = argument, +0xa = argument.
u32 __fastcall Fn_66c1f3(u8 *o, void *, u32 v0, u32 v1, u32 v2, u32 v3);

// 0x507800 (thiscall): the 0x20 elements of 0x14 bytes from the start are constructed (CRT_VectorCtorIterator with 0x507080, which clears +0, +0xc and the bytes +9 .. +0xb) and the dword
// at +0x294, the byte at +0x298, the dword at +0x299 and the byte at +0x29d cleared. Returns the object.
u8 *__fastcall Class507800_Ctor(u8 *object);

// 0x507840 (thiscall): the 0x20 elements of 0x14 bytes are destroyed (CRT_VectorDtorIterator; the element destructor the original passes is the empty debug print).
void __fastcall Class507800_Dtor(u8 *object);

// 0x6181c0 (thiscall; the SEH frame left out): the vtable +0 = 0x703748 (the base's), +4 = 0x703744, +8 and +0xc cleared, Class703c14_Ctor on +0x10, +0x54 = 1, +0x58 .. +0x68 cleared and the
// final vtable +0 = 0x7037e8. Returns the object.
u8 *__fastcall Class7037e8_Ctor(u8 *object);

// 0x47eab0 (thiscall; the SEH frame left out): Fn_661870 on the object, the bytes +0x24 .. +0x28 and the dwords +0x34, +0x38 cleared, SurfaceInfo_Ctor on +0x3c, then +0 = 0x6fefc8,
// the byte +4 and the dwords +8, +0xc, +0x64 cleared. Returns the object.
u8 *__fastcall Class6fefc8_Ctor(u8 *object);

// 0x47eb40 (thiscall destructor; the SEH frame left out): +0 = 0x6fefc8; when the byte +7 and the dword +0x64 are set the surface at +0x14 is released
// (Surface_Release, its result written over a local), then Fn_6618a6. Returns what Fn_6618a6 does (the object).
u32 __fastcall Class6fefc8_Dtor(u8 *object);

// 0x6175c0 (thiscall): Class7037e8_Ctor, then +0x6c, +0x70, +0x74, +0x78 and +0x80 cleared, +0x84 = 0x703730 and the vtable +0 = 0x703710. Returns the object.
u8 *__fastcall Class703710_Ctor(u8 *object);
}  // extern "C"
