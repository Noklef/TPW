// More methods that hand their members or list entries to virtual methods: reimplementation of 0x5644c0, 0x4ef7b0, 0x4ef790, 0x5640a0, 0x58d380, 0x421690 and 0x576270.
// Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5644c0 (thiscall; returns 1): the dword at 0x879cd4 = the COM call (slot 0x80) of the object at +0x14 with 0.
u32 __fastcall Obj_StoreComSlot80(u8 *object);

// 0x4ef7b0 (thiscall on a counted object): the count at +4 is lowered unless it is 0 or -1; returns the object when the count is then 0, otherwise 0.
// 0x4ef790 (thiscall on a pointer to one): the object that returns (when there is one) is deleted (slot 0 with 1).
u8 *__fastcall Counted_Release(u8 *counted);
void __fastcall CountedRef_Release(u8 **ref);

// 0x5640a0 (thiscall; returns 1): when bit 0 of the byte at +0x780 is set the COM object at +8 (when set) gets its slot 0x4c called; then the COM objects at +0x14,
// +0x10, +0x18, +0xc, +0, +4 and +8 are released (slot 8) when set, and the pointers cleared.
u32 __fastcall Obj_ReleaseSevenComMembers(u8 *object);

// 0x58d380 (thiscall; returns 1): the animated textures are cleared (AnimTextures_Clear); then for each of the six arrays of 256 pointers from the start of the object, as many
// as the signed count at +0x1800 + 4 * index says: the record pointed to (when set) has the object its first dword holds (when set) deleted through slot 0x24 with 1,
// and is freed; the count is cleared.
u32 __fastcall Object_FreeSixPointerArrays(u8 *object);

// 0x421690 (thiscall on {list, cached}; object, pops 4): the node holding the object is unlinked from the list (+4 head, +8 tail, +0xc count) and freed, the cached
// pointer at +4 cleared when it is the object, and the object deleted (slot 0 with 1). Returns 1 when there was such a node, otherwise 0.
u32 __fastcall ListRef_Remove(u8 **ref, void *, u8 *object);

// 0x576270 (thiscall on a pool of nodes {object, next} with the free head at +4 and a counter at +0xc; head variable, object, pops 8): a node is taken from the pool (the
// counter raised even when there is none; 0 returned then) and given the object. 1 when the list already has the object. Otherwise the node is linked into the list
// after the leading entries whose object slot 8 returns 1 (at the front when the object own slot 8 returns 1, or when the first entry does not), and the bit 31 of the
// dword at 0x8bd508 set. Returns 1.
u32 __fastcall Pool_AddSorted(u8 *pool, void *, u8 **head, u8 *object);

}  // extern "C"
