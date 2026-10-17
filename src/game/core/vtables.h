// The vtable-pointer setters: functions that only store a vtable pointer (the constructors of the abstract interface classes at 0x6fd7a0 .. 0x6fd848 and others, and virtual
// bases whose constructor sets the pointer at +0 or +0x60). Reimplementation of 0x405520, 0x429b50, 0x429b80, 0x4a8bf0, 0x4bc420, 0x4c75f0, 0x4d19f0, 0x4d5260, 0x4db7c0, 0x4dd310, 0x4e11f0, 0x4e1200, 0x4e1210, 0x4e5b90 and others. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x617920 / 0x617ad0 / 0x6338b0 (thiscall): the vtable pointer +0 = 0x703748 / 0x703770 / 0x703770;
// the last also sets +4 = 1 and returns the object.
void __fastcall Vtbl703748_Set(u8 *object);

void __fastcall Vtbl703770_Set(u8 *object);

// 0x4f0140 / 0x5cde30 (thiscall): through the pointer 8 / 4 before the object, a vtable pointer, the dword at +4 of that is an
// offset to a virtual base: the dword 8 / 4 before the object plus that offset is set to 0x7099dc / 0x70995c. Returns that
// vtable pointer.
u32 __fastcall VirtualBase_SetVtbl7099dc(u8 *object);

u32 __fastcall VirtualBase_SetVtbl70995c(u8 *object);

// 0x504740 / 0x429b80 (thiscall): the vtable pointer at +0 / +0x60 = 0x6fd800 / 0x6fdce8. Undefined return.
void __fastcall Vtbl6fd800_Set(u8 *object);

void __fastcall Vtbl6fdce8_At60_Set(u8 *object);

// 0x5cea30 (thiscall): like VirtualBase_SetVtbl7099dc, with 0x70994c (8 bytes before the object).
u32 __fastcall VirtualBase_SetVtbl70994c(u8 *object);

// 0x405520: the vtable pointer +0 = 0x6fd6c8.
void __fastcall Vtbl6fd6c8_Set_405520(u8 *o);

// 0x429b50: the vtable pointer +0 = 0x6fdce8.
void __fastcall Vtbl6fdce8_Set_429b50(u8 *o);

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

// 0x4e11f0: the vtable pointer +0 = 0x6fd7c8.
void __fastcall Vtbl6fd7c8_Set_4e11f0(u8 *o);

// 0x4e1200: the vtable pointer +0 = 0x6fd7d0.
void __fastcall Vtbl6fd7d0_Set_4e1200(u8 *o);

// 0x4e1210: the vtable pointer +0 = 0x6fd7d8.
void __fastcall Vtbl6fd7d8_Set_4e1210(u8 *o);

// 0x4e5b90: the vtable pointer +0 = 0x700620.
void __fastcall Vtbl700620_Set_4e5b90(u8 *o);

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

// 0x503fe0: the vtable pointer +0 = 0x6fd808.
void __fastcall Vtbl6fd808_Set_503fe0(u8 *o);

// 0x506190: the vtable pointer +0 = 0x700868.
void __fastcall Vtbl700868_Set_506190(u8 *o);

// 0x509350: the vtable pointer +0 = 0x6fd820.
void __fastcall Vtbl6fd820_Set_509350(u8 *o);

// 0x50b970: the vtable pointer +0 = 0x6fd848.
void __fastcall Vtbl6fd848_Set_50b970(u8 *o);

// 0x512c30: the vtable pointer +0 = 0x6fd7a0.
void __fastcall Vtbl6fd7a0_Set_512c30(u8 *o);

// 0x512c40: the vtable pointer +0 = 0x6fd7a8.
void __fastcall Vtbl6fd7a8_Set_512c40(u8 *o);

// 0x565230: the vtable pointer +0 = 0x701260.
void __fastcall Vtbl701260_Set_565230(u8 *o);

// 0x591970: the vtable pointer +0 = 0x7026e8.
void __fastcall Vtbl7026e8_Set_591970(u8 *o);

// 0x59b860: the vtable pointer +0 = 0x6fd828.
void __fastcall Vtbl6fd828_Set_59b860(u8 *o);

// 0x6a9180: the vtable pointer +0 = 0x709b44.
void __fastcall Vtbl709b44_Set_6a9180(u8 *o);

// 0x6ad000: the vtable pointer +0 = 0x70a0a8.
void __fastcall Vtbl70a0a8_Set_6ad000(u8 *o);

// 0x6add30: the vtable pointer +0 = 0x70a078.
void __fastcall Vtbl70a078_Set_6add30(u8 *o);

// 0x6b4a50: the vtable pointer +0 = 0x70a078.
void __fastcall Vtbl70a078_Set_6b4a50(u8 *o);

// 0x6b4a90: the vtable pointer +0 = 0x70a258.
void __fastcall Vtbl70a258_Set_6b4a90(u8 *o);

// 0x6c7090: the vtable pointer +0 = 0x70a778.
void __fastcall Vtbl70a778_Set_6c7090(u8 *o);
}  // extern "C"
