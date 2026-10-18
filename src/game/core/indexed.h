// Small getters and setters by index or through a global: reimplementation of 0x41a960, 0x41a970, 0x4145d0, 0x4e0aa0,
// 0x5099a0, 0x509990 and 0x5ed430. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41a960 (thiscall): *out = the dword at 0x1da70c of what the dword at 0x80239c points to. Returns that pointer.
u32 __fastcall Out_SetFromGame1da70c(u32 *out);

// 0x41a970 (thiscall): that dword minus the dword at +0.
u32 __fastcall Game1da70c_MinusAt0(const u32 *object);

// 0x41a990 (thiscall; name unconfirmed): that dword shifted right by 2 minus the dword at +0 shifted right by 2.
u32 __fastcall Game1da70c_MinusAt0Quarters(const u32 *object);

// 0x4145d0 (thiscall; index, pops 4): the dword at 0x4e0 + 4 * index.
u32 __fastcall Obj_GetIndexed4e0(const u8 *object, void *, u32 index);

// 0x4e0aa0 (thiscall; value, pops 4): whether the low 16 bits of the value equal the word at +0x6c.
u32 __fastcall Obj_Word6cEquals(const u8 *object, void *, u32 value);

// 0x5099a0 / 0x509990 (thiscall; index, pops 4): the dword at 0x28 + 12 * index is cleared (returns 3 * index) / returned.
u32 __fastcall Obj_ClearIndexed28(u8 *object, void *, u32 index);
u32 __fastcall Obj_GetIndexed28(const u8 *object, void *, u32 index);

// 0x5ed430 (thiscall): the number of 20-byte elements between the pointers at +4 and +8; 0 when the first is 0.
s32 __fastcall Vector20_Count(const u8 *object);

// 0x4f04c0 (thiscall; index, pops 4): the address of the index-th element of 36 bytes of the array at +4.
u8 *__fastcall Array36_At(const u8 *object, void *, u32 index);
}  // extern "C"
