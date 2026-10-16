// Observer lists, a handle swap, a definition kind check and a ring push: reimplementation of 0x41eaa0, 0x421590, 0x4d91f0, 0x4e2b80 and 0x50c100.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41eaa0 / 0x421590 (thiscall; item, pops 4): the list (vtable 0x6fdb04 / 0x6fdb0c, +4 head, +8 tail, +0xc count) at +0x4c / +0 of the object is made
// (a block of 16 bytes) when there is none; a node of 12 bytes ({item, next, prev}; the node's +4 is the old head) becomes its head, the count rises
// and the dword at +0x34 / +0x30 of the item = the object. Returns 1 (0 when there is no list).
u32 __fastcall ObserverList4c_Add(u8 *object, void *, u8 *item);
u32 __fastcall ObserverList0_Add(u8 *object, void *, u8 *item);

// 0x4d91f0 (thiscall; handle, pops 4): when the word at +0x24 is not 0 the thing it names (in the table at 0x7cfb90) gets the word at +8 = handle; the
// thing the handle names gets the words at +8 = 0 and +0xa = the old word at +0x24, and the word at +0x24 = handle.
void __fastcall Obj_ReplaceHandle24(u8 *object, void *, u32 handle);

// 0x4e2b80 (thiscall): whether the dword at +0x9c of the object definition for the id at +0xe (ObjectDefs_Find, looked up again for each test) is 1, 2
// or 3.
u32 __fastcall Obj_DefKind123(const u8 *object);

// 0x50c100 (thiscall; id, value, pops 8): the ring of 32 word pairs from +4 (the next position at +0x88): unless the last pushed pair has the same id
// and value, the pair (low 16 bits of id, low 16 bits of value) is stored at the position, which moves on (modulo 32).
void __fastcall Ring_PushUnique(u8 *object, void *, u32 id, u32 value);

}  // extern "C"
