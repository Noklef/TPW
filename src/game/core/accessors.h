// Tiny getters and setters of globals and object fields, and do-nothing virtuals: reimplementation of
// 0x4030c0, 0x423720, 0x423730, 0x423b50, 0x477190, 0x47ebb0, 0x486b80, 0x48c920-0x48c950, 0x498a70,
// 0x4a2a90, 0x4df280, 0x4e1910, 0x4e19f0-0x4e1a20, 0x4e2180, 0x4e2770, 0x4f7c50, 0x51b3c0, 0x51b3d0,
// 0x527930-0x527950 and 0x53c8c0. They take their names from what they do (the global's address, the
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
u8 __cdecl Global7c24d0_GetByte();              // 0x486b80: the byte (AL; the rest of EAX is the caller's)
}
