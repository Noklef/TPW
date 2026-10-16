// Methods that hand their argument to a member's: reimplementation of 0x5d5da0, 0x5d8a70, 0x5da170,
// 0x5da440, 0x5dd490, 0x5dd4a0 (PtrList_Append on the list at +0x18, +0xc, +0x58, +4, +8, +0x6c) and
// 0x618600 (Obj_Copy4At34 on the object at +0x64).
//
// The first six leave PtrList_Append's EAX behind as their result: the new node when the list was empty
// (its tail, +8), else the new count (+0xc).
#pragma once

#include "../../../re/types.h"

extern "C" {

u32 __fastcall Owner18_Append(u8 *owner, void *, u32 item);  // 0x5d5da0
u32 __fastcall OwnerC_Append(u8 *owner, void *, u32 item);   // 0x5d8a70
u32 __fastcall Owner58_Append(u8 *owner, void *, u32 item);  // 0x5da170
u32 __fastcall Owner4_Append(u8 *owner, void *, u32 item);   // 0x5da440
u32 __fastcall Owner8_Append(u8 *owner, void *, u32 item);   // 0x5dd490
u32 __fastcall Owner6c_Append(u8 *owner, void *, u32 item);  // 0x5dd4a0

// 0x618600 (thiscall; source, pops 4): Obj_Copy4At34 on the object at +0x64 with the source.
u32 __fastcall Owner64_Copy4At34(u8 *owner, void *, const u32 *source);
}
