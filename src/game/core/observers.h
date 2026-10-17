// Observer lists: adding an item to the list at +0 or +0x4c of an object. Reimplementation of 0x41eaa0, 0x421590, 0x421680. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x41eaa0 / 0x421590 (thiscall; item, pops 4): the list (vtable 0x6fdb04 / 0x6fdb0c, +4 head, +8 tail, +0xc count) at +0x4c / +0 of the object is made
// (a block of 16 bytes) when there is none; a node of 12 bytes ({item, next, prev}; the node's +4 is the old head) becomes its head, the count rises
// and the dword at +0x34 / +0x30 of the item = the object. Returns 1 (0 when there is no list).
u32 __fastcall ObserverList4c_Add(u8 *object, void *, u8 *item);

u32 __fastcall ObserverList0_Add(u8 *object, void *, u8 *item);

// 0x421680 (thiscall; item, pops 4): +8 = item, then ObserverList0_Add of the object with it. Returns what that returns.
u32 __fastcall Obj_SetAt8AndAdd(u8 *object, void *, u8 *item);
}  // extern "C"
