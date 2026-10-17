// Tiny getters and setters of globals and object fields, and do-nothing virtuals: reimplementation of
// 0x4030c0, 0x423720, 0x423730, 0x423b50, 0x477190, 0x47ebb0, 0x486b80, 0x48c920-0x48c950, 0x498a70,
// 0x4a2a90, 0x4df280, 0x4e1910, 0x4e19f0-0x4e1a20, 0x4e2180, 0x4e2770, 0x4f7c50, 0x51b3c0, 0x51b3d0,
// 0x527930-0x527950, 0x53c8c0, 0x4861d0, 0x4d8c00-0x4d8c50, 0x42a010, 0x42a050, 0x48c960, 0x492e40
// and 0x4a0810. They take their names from what they do (the global's address, the
// field's offset) until the owners are known; a setter's return value is the value it was given,
// as the code leaves it in EAX.
#pragma once

#include "../../../re/types.h"

extern "C" {

// Fields of an object (thiscall).
u32 __fastcall Obj_Get24(const u8 *object);                           // 0x423720
u32 __fastcall Obj_Get2c(const u8 *object);                           // 0x423730
u32 __fastcall Obj_Get30(const u8 *object);                           // 0x4030c0
u32 __fastcall Obj_SetFirst(u8 *object, void *, u32 value);           // 0x423b50: +0 = value
u32 __fastcall Obj_Get188(const u8 *object);                          // 0x4e1a10
u32 __fastcall Obj_Set188(u8 *object, void *, u32 value);             // 0x4e1a20
u32 __fastcall Obj_Get194(const u8 *object);                          // 0x4e1a00
u32 __fastcall Obj_Set194(u8 *object, void *, u32 value);             // 0x4e2770
u8 __fastcall Obj_GetByte18c(const u8 *object);                       // 0x4e2180: the byte (AL; the rest of EAX is the caller's)
void __fastcall Obj_Inc2b8(u8 *object);                               // 0x4e19f0: +0x2b8 incremented
void __fastcall Vtbl6fd818_Set(u8 *object);                           // 0x4df280: +0 = 0x6fd818
u32 __fastcall Obj_SetByte7(u8 *object, void *, u32 ignored, u32 value);  // 0x47ebb0: the byte +7 = the second argument

// Virtuals that do nothing; the callee pops the arguments.
void __stdcall Stub_Ret16(u32, u32, u32, u32);  // 0x477190
void __stdcall Stub_Ret4b(u32);                 // 0x4f7c50

// Globals.
u32 __cdecl Global7ca1cc_Get();                 // 0x498a70
u32 __cdecl Global7cb2e8_Get();                 // 0x4a2a90
u32 __cdecl Global7c2650_Get();                 // 0x48c920
u32 __cdecl Global7c2654_Get();                 // 0x48c930
u32 __cdecl Global7c2654_Set(u32 value);        // 0x48c940
u32 __cdecl Global7c2650_Set(u32 value);        // 0x48c950
u32 __cdecl Global8186ac_Get();                 // 0x527930
u32 __cdecl Global816d48_Set(u32 value);        // 0x527940
u32 __cdecl Global816d48_Get();                 // 0x527950
u32 __cdecl Global80239c_Get();                 // 0x4e1910
u32 __cdecl Global802ba8_SetMasked(u32 value);  // 0x51b3c0: = value & 0x3ff; returns it
void __cdecl Global802bc0_Set1();               // 0x51b3d0
void __cdecl Global871ee0_Clear();              // 0x53c8c0
u8 __cdecl Global7c24d0_GetByte();

// 0x4861d0 (cdecl; i): the dword i of the table at 0x74fb50.
u32 __cdecl Table74fb50_Get(u32 i);

// 0x4d8c50: the counter at 0x7cdb98 incremented; returns it.
u32 __cdecl Counter7cdb98_Inc();

// 0x4d8c00 (cdecl; v): 0 stays as it is; below 0x10 (by the low byte) the low byte times 16;
// otherwise the low byte over 16.
u32 __cdecl Nibble_Scale(u32 v);

// 0x4d8c20 (cdecl; a, b): the low byte of b shifted left by the log2 of a's low byte (0 for
// 0 and 1), or, when that is above 0x80, divided by 256.
u32 __cdecl Byte_ShiftLog2(u32 a, u32 b);

// 0x42a010 (thiscall on a mesh's +0x60; returns 1): when the flags at +0x50 (this - 0x10) lack
// 0x10000000 the mesh joins the list at 0x879dec through its +0x58 (as Meshes_Ready does).
u32 __fastcall MeshPlus60_Ready(u8 *sub);

// 0x42a050 (thiscall on an object's +0x28): the object (this - 0x28) when the low nibble of the
// dword at this - 0x10 is 2, else 0.
u8 *__fastcall Plus28_IfKind2(u8 *sub);

// 0x48c960: 0x7c264c, 0x7c2650 and 0x7c2654 = the dwords at 0x7b05a8, 0x7b05ac and 0x7b05b0;
// returns the last.
u32 __cdecl Globals7c264c_Load();

// 0x492e40 (thiscall): vtable 0x492d80, +4 = +0xc = +0x10 = 0, +8 = -1. Returns the object.
u8 *__fastcall Class492d80_Ctor(u8 *object);

// 0x4a0810 (cdecl; which, v): 1, 2, 3 store v in 0x7cb28c, 0x7cb2a0, 0x7cb290; returns 0 for 1 and 2,
// v for 3, else which - 3 (EAX as the code leaves it).
s32 __cdecl Global7cb28c_Select(s32 which, u32 v);              // 0x486b80: the byte (AL; the rest of EAX is the caller's)

// 0x6185f0 (thiscall): the dword at +0x64, plus 0x10.
u32 __fastcall Obj_Get64Plus10(const u8 *object);

u32 __fastcall Obj_Get3c(const u8 *object);

// 0x6339e0 (thiscall; source, pops 4): +0x14 = 0, the word +0xe = the source's word at +0xc,
// +0x10 = the source's dword at +0x10, the byte +0xc = 3. Returns that dword.
u32 __fastcall Obj_Take(u8 *object, void *, const u8 *source);

// 0x633b60 (thiscall; value): +0x10 = value; returns it.
u32 __fastcall Obj_Set10(u8 *object, void *, u32 value);

// 0x6339a0 (thiscall; source, pops 4): +0x14 = the source's dword at +0x20; the byte +0xd = its low
// byte at +0x10; the word +0xe = the new +0x14's bit 3 (0 or 8); the byte +0xc = 1; +0x10 = the
// source's words at +0x14 and +0x18 as low and high halves. Returns the source.
const u8 *__fastcall Obj_TakeWords(u8 *object, void *, const u8 *source);

// 0x6344c0 (thiscall; source, pops 4): 4 dwords at +0x34 from the source (nothing for a null
// source). Returns the source's fourth dword, or 0 for a null source.
u32 __fastcall Obj_Copy4At34(u8 *object, void *, const u32 *source);

// 0x48f650 (thiscall; a, b, pops 8): without its word +0x44 set, or a null a or b: 0. Else *a =
// the word at +0xa8, *b = the word at +0xaa (zero-extended) and the dword at +0xa0 is returned.
u32 __fastcall Obj_GetWordPair(const u8 *object, void *, u32 *a, u32 *b);

// 0x57ac90 (thiscall): the dword 0x14c before the object (the object is a member 0x14c into its owner).
u32 __fastcall Obj_GetBefore14c(const u8 *object);

// 0x57cb10 (thiscall; a, b, c, pops 12): +0x14 = a, +0x18 = b, +0x10 = c. Returns b.
u32 __fastcall Obj_Set14_18_10(u8 *object, void *, u32 a, u32 b, u32 c);

// 0x5832e0 (thiscall; colour, pops 4): +0x8b4 = the colour and the floats at +0x8b8, +0x8bc, +0x8c0 and +0x8c4 = its
// bytes from the top (each left in place: 0xff000000, 0xff0000, 0xff00, 0xff). Returns the lowest byte.
u32 __fastcall Obj_SetColour8b4(u8 *object, void *, u32 colour);

// 0x4f1520 (thiscall; a, pops 4): +0 = a, +8 and +0xc from the dwords at 0x7099f0 and 0x7099f4, +0x10 from the dword at 0xfb34b0. Returns the object.
u8 *__fastcall Obj_Init4f1520(u8 *object, void *, u32 a);

// 0x4f0880 / 0x4f08a0 (stdcall; the object and 3 / 7 more dwords that are not used): Obj_Init4f1520 with -1 on the object. Returns it.
u8 *__stdcall Obj_InitMinus1A(u8 *object, u32, u32, u32);

u8 *__stdcall Obj_InitMinus1B(u8 *object, u32, u32, u32, u32, u32, u32, u32);

// 0x557d60 (cdecl; source, a, b, c): *a, *b and *c = the source's dwords at +0x30, +0x34 and +0x38. Returns the last.
u32 __cdecl Obj_Read30_34_38(const u8 *source, u32 *a, u32 *b, u32 *c);

// 0x4056f0 (thiscall): the address 4 bytes into the object.
u8 *__fastcall Obj_AddrOf4(u8 *object);

// 0x47a9a0 (thiscall): the dword at +0xc, or 0x6fd6c4 when it is 0.
u32 __fastcall Obj_GetCOr6fd6c4(const u8 *object);

// 0x4d05d0 (thiscall; value, pops 4): the dword at +0x118 = value (the debug print is not reproduced).
void __fastcall Obj_Set118(u8 *object, void *, u32 value);

// 0x591a70: the dword at -0x4c.
u32 __fastcall WaveletCodec_GetRefCount(const u8 *o);

// 0x5b0d60: the address of the part at 0x8.
u8 * __fastcall BalanceFile_GetValues(u8 *o);

// 0x4217e0: the dword at 0x4.
u32 __fastcall StreamBase_GetSource(const u8 *o);

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

// 0x4217f0: 0x4 = value; returns it.
u32 __fastcall Set4_4217f0(u8 *o, void *, u32 v0);

// 0x48f690: 0x4c = value; returns it.
u32 __fastcall Set4c_48f690(u8 *o, void *, u32 v0);

// 0x4e03d0: the dword at 0x64.
u32 __fastcall Get64_4e03d0(const u8 *o);

// 0x4e21b0: the byte at 0x190.
u8 __fastcall GetByte190_4e21b0(const u8 *o);

// 0x4e21e0: 0x198 = value; returns it.
u32 __fastcall Set198_4e21e0(u8 *o, void *, u32 v0);

// 0x4e21f0: the byte at 0x198.
u8 __fastcall GetByte198_4e21f0(const u8 *o);

// 0x51a280: the dword at 0x1da710.
u32 __fastcall Get1da710_51a280(const u8 *o);

// 0x51c050: the dword at 0x8.
u32 __fastcall Get8_51c050(const u8 *o);

// 0x564ee0: the dword at -0x10.
u32 __fastcall Getm10_564ee0(const u8 *o);

// 0x56dd60: the address of the part at 0xc4.
u8 * __fastcall Partc4_56dd60(u8 *o);

// 0x59b540: the dword at 0xc.
u32 __fastcall Getc_59b540(const u8 *o);

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

// 0x695af0: the byte at 0xc.
u8 __fastcall GetBytec_695af0(const u8 *o);

// 0x695b00: the byte at 0xd.
u8 __fastcall GetByted_695b00(const u8 *o);

// 0x69c8e0: the word at 0xc.
u16 __fastcall GetWordc_69c8e0(const u8 *o);

// 0x69c8f0: the word at 0xe.
u16 __fastcall GetWorde_69c8f0(const u8 *o);

// 0x6a8cc0: the dword at 0x1c.
u32 __fastcall Get1c_6a8cc0(const u8 *o);

// 0x6adf80: the dword at 0x88.
u32 __fastcall Get88_6adf80(const u8 *o);

// 0x6b9220: 0x2c = value; returns it.
u32 __fastcall Set2c_6b9220(u8 *o, void *, u32 v0);

// 0x6baa00: 0xc = value; returns it.
u32 __fastcall Setc_6baa00(u8 *o, void *, u32 v0);

// 0x6bb190: 0x14 = value; returns it.
u32 __fastcall Set14_6bb190(u8 *o, void *, u32 v0);

// 0x6bb1a0: the dword at 0x14.
u32 __fastcall Get14_6bb1a0(const u8 *o);

// 0x6bb1e0: 0x14 = 0x0.
void __fastcall Set14_To0_6bb1e0(u8 *o);

// 0x6c5590: the address of the part at 0x14.
u8 * __fastcall Part14_6c5590(u8 *o);

// 0x4e0de0: stores +0x30 = argument.
u16 __fastcall Fn_4e0de0(u8 *o, void *, u32 v0);

// 0x4f6010: returns a constant.
u8 __cdecl Fn_4f6010();

// 0x509ae0: stores +0x60 = argument.
u16 __fastcall Fn_509ae0(u8 *o, void *, u32 v0);

// 0x50b080: stores +0x0 = argument.
u16 __fastcall Fn_50b080(u8 *o, void *, u32 v0);

// 0x52c2a0: stores 0x818c54 = 0x0.
void __cdecl Fn_52c2a0();

// 0x52fbd0: stores 0x820a8c = 0x0.
void __cdecl Fn_52fbd0();

// 0x56b620: returns a field or global.
u32 __stdcall Fn_56b620(u32 v0);

// 0x5ce530: stores +0x0 = a value.
u32 __fastcall Fn_5ce530(u8 *o, void *, u32 v0);

// 0x5cec50: returns a field or global.
u32 __fastcall Fn_5cec50(u8 *o, void *, u32 v0, u32 v1);

// 0x5d1400: returns a constant.
u8 __cdecl Fn_5d1400();

// 0x6590c3: stores 0xfaa5d4 = argument.
u32 __cdecl Fn_6590c3(u32 v0);

// 0x6590ea: stores 0xfaa5fc = argument.
u32 __cdecl Fn_6590ea(u32 v0);

// 0x6593ff: returns a field or global.
u32 __cdecl Fn_6593ff();

// 0x6595a8: returns a field or global.
u16 __cdecl Fn_6595a8();

// 0x659ce0: stores +0x10 = 0x1.
u32 __fastcall Fn_659ce0(u8 *o);

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

// 0x6617f0: stores +0x12 = 0x1.
u32 __fastcall Fn_6617f0(u8 *o);

// 0x661830: stores +0x90 = argument.
u32 __fastcall Fn_661830(u8 *o, void *, u32 v0);

// 0x661850: stores +0x94 = argument.
u32 __fastcall Fn_661850(u8 *o, void *, u32 v0);

// 0x6618e6: stores +0x4 = 0x1.
u32 __fastcall Fn_6618e6(u8 *o);

// 0x6618f8: stores +0x4 = 0x0.
u32 __fastcall Fn_6618f8(u8 *o);

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

// 0x669c30: returns a field or global.
u32 __fastcall Fn_669c30(const u8 *o);

// 0x669f80: stores +0x6 = 0x1.
u32 __fastcall Fn_669f80(u8 *o);

// 0x669f94: stores +0x6 = 0x0.
u32 __fastcall Fn_669f94(u8 *o);

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

// 0x66c922: returns a field or global.
u32 __fastcall Fn_66c922(const u8 *o);

// 0x66cc7c: returns a field or global.
u32 __fastcall Fn_66cc7c(const u8 *o);

// 0x66d950: returns a field or global.
u32 __fastcall Fn_66d950(const u8 *o);

// 0x66dad2: returns a field or global.
u32 __fastcall Fn_66dad2(const u8 *o);

// 0x6a65e0: returns a constant.
u8 __cdecl Fn_6a65e0();

// 0x6c0200: stores +0x0 = 0x0.
u32 __stdcall Fn_6c0200(u32 v0, u32 v1);

// 0x6c3900: returns a field or global.
u32 __cdecl Fn_6c3900(u32 v0);

// 0x6d2780: stores +0x0 = 0x0.
u32 __stdcall Fn_6d2780(u32 v0, u32 v1);

// 0x409cd0: stores +0x0 = argument.
u32 __fastcall Fn_409cd0(u8 *o, void *, u32 v0);

// 0x4d2a60: stores +0x0 = a value.
u32 __fastcall Fn_4d2a60(u8 *o, void *, u32 v0);

// 0x4e0df0: stores +0x0 = a value.
u32 __fastcall Fn_4e0df0(u8 *o, void *, u32 v0);

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

// 0x6599da: stores +0x32 = argument.
u32 __fastcall Fn_6599da(u8 *o, void *, u32 v0);

// 0x659a40: stores +0xe = argument.
u32 __fastcall Fn_659a40(u8 *o, void *, u32 v0);

// 0x65f330: returns a field or global.
u16 __fastcall Fn_65f330(const u8 *o);

// 0x66190a: returns a field or global.
u8 __fastcall Fn_66190a(const u8 *o);

// 0x668564: stores +0x161 = argument.
u32 __fastcall Fn_668564(u8 *o, void *, u32 v0);

// 0x668800: returns a field or global.
u8 __fastcall Fn_668800(const u8 *o);

// 0x66b454: returns a field or global.
u16 __fastcall Fn_66b454(const u8 *o);

// 0x66b523: stores +0x14c = argument.
u32 __fastcall Fn_66b523(u8 *o, void *, u32 v0);

// 0x6d2770: stores +0x0 = 0x0.
u8 __stdcall Fn_6d2770(u32 v0);
}
