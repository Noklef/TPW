// The message centre (module MESS: MessageCentre_Save 0x40fc80, MessageCentre_Load 0x40fd60): 29 slots of 0x14 bytes from +0, each a dword and a std::map (tree B, core/stdtree) at +4.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x40f970 (thiscall on a slot; name unconfirmed): the destructor of one of the message centre's slots, a dword and then a std::map of tree B at +4 (StdTree_DtorB).
void __fastcall MessageSlot_Dtor(u8 *slot);

// 0x409250 (thiscall on the message centre; name unconfirmed): its 29 slots of 0x14 bytes from +0 destroyed, the last first (CRT_VectorDtorIterator with MessageSlot_Dtor).
void __fastcall MessageCentre_DtorSlots(u8 *centre);

// 0x40f9e0 (thiscall on the message centre; thing, slot, pops 8; name unconfirmed): 0 for a slot of 29 or more (signed); else the thing's handle
// (Thing_GetHandle) inserted into that slot's map (StdMapB_Insert). Returns whether it was inserted (in AL only).
u8 __fastcall MessageCentre_AddThing(u8 *centre, void *, const u8 *thing, s32 slot);

// 0x40fa40 (thiscall on the message centre; thing, pops 4; name unconfirmed): for each of the 29 slots the thing's handle (Thing_GetHandle) is looked up
// in the slot's map (Tree16a_LowerBound; found when that is not the head and the handle is not below its key, StdTreeB_KeyLess) and that node erased
// (StdTree_EraseB). Returns the number erased.
u32 __fastcall MessageCentre_RemoveThing(u8 *centre, void *, const u8 *thing);

// 0x40fc80 (thiscall on the message centre; a save chunk whose dword +0 is the stream, pops 4): writes the slot count 29, then for each slot its map's size
// (4 bytes) and the keys of its nodes in order (2 bytes each, from the first node through StdTree_IncB to the head) with Stream_Write; a short key write
// is only asserted (left out). Returns 0.
u32 __fastcall MessageCentre_Save(u8 *centre, void *, const u8 *chunk);

// 0x40fd60 (thiscall on the message centre; a load chunk whose dword +0 is the stream, pops 4): every slot map emptied (StdTree_EraseRangeB from the
// first node to the head); then Stream_Read of the slot count (4 bytes, ignored: the result of that read is the buffer the counts are read into), and
// for each of the 29 slots a count (4 bytes) and that many 2-byte keys inserted into the slot's map (StdMapB_Insert). A short read leaves the
// buffer's old bytes: the count buffer starts as the first read's result, the key buffer as the erase's out (the last slot's head); the short-key
// assert is left out. Returns -1 at the first key already in its map, else 0.
s32 __fastcall MessageCentre_Load(u8 *centre, void *, const u8 *chunk);
}  // extern "C"
