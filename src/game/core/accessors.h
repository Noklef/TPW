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
}
