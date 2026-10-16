// More small methods: the head of a list popped, a name lookup, the release of a shared array, lower-bound searches of three trees, and some releases of globals:
// reimplementation of 0x5f46a0, 0x5bd230, 0x5b9fd0, 0x5ba290, 0x410ac0, 0x47d460, 0x5b30b0, 0x5633a0, 0x564140, 0x51fd20 and 0x5c4ce0. The STL lock is left out.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5f46a0 (thiscall on a list: +4 head, +8 tail, +0xc count; nodes {object, next, prev}): the head node is unlinked and freed. Returns the object it held (0 when the list was
// empty).
u8 *__fastcall List_PopHeadObject(u8 *list);

// 0x5bd230 (thiscall; name, pops 4): over the nodes of the list at +0x2c -> +4 whose object has a name at +0x14: the dword at +8 of the last object whose name equals the
// given one (case-insensitively) - 1 when none does.
u32 __fastcall Names_FindLastValue(u8 *object, void *, const char *name);

// 0x5b9fd0 / 0x5ba290 (thiscall): the shared record at +0x30 (when there is one) loses a use; at 0 its array (elements of 0x66 / 0x10 bytes, destroyed by the function the dword
// at 0x6fd504 / 0x6fd50c holds) and the array's block are freed, +4 is cleared and the record freed. +0x30 is cleared.
void __fastcall Shared_Release66(u8 *object);
void __fastcall Shared_Release10(u8 *object);

// 0x410ac0 / 0x47d460 / 0x5b30b0 (thiscall on a tree object whose +4 is the head node; key pointer, pops 4): the lower bound of the key (a word / dword / word, unsigned; at
// +0xc of the nodes, left child at +0, right child at +8, the sentinel the dword at 0x785de4 / 0x7b4a54 / 0xf7c970 holds): the first node not below the key, the head node
// when there is none.
u8 *__fastcall Tree16a_LowerBound(u8 *tree, void *, const u16 *key);
u8 *__fastcall Tree32_LowerBound(u8 *tree, void *, const u32 *key);
u8 *__fastcall Tree16b_LowerBound(u8 *tree, void *, const u16 *key);

// 0x5633a0 (cdecl; returns 1 or 0): when the object the dword at 0x879a24 points to exists its seven COM members are released (twice, as the original does), it is freed and
// the dword cleared.
u32 __cdecl Global879a24_Free();

// 0x564140 (thiscall; returns 1): Obj_ReleaseSevenComMembers on the object when the dword at 0x8bd524 is 1.
u32 __fastcall Obj_ReleaseSevenIfMode1(u8 *object);

// 0x51fd20 (cdecl): when the dword at 0x816d40 is not 0: the dword at 0x80ced0 is cleared, the blocks the dwords at 0x80cef8 and 0x816d38 point to are freed (Mem_FreeChecked)
// and the dword at 0x816d40 cleared.
void __cdecl Globals816d40_Free();

// 0x5c4ce0 (thiscall): the five strings at +0, +0x14, +0x28, +0x3c and +0x50 are constructed empty. Returns the object.
u8 *__fastcall Strings5_Ctor(u8 *object);

}  // extern "C"
