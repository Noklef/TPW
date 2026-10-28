// The message centre (module MESS: MessageCentre_Save 0x40fc80, MessageCentre_Load 0x40fd60): 29 slots of 0x14 bytes from +0, each a dword and a std::map (tree B, core/stdtree) at +4.
// Slot k's map holds the handles of the things that listen to messages of type k (MessageCentre_Post hands them each message).
#pragma once

#include "../../../re/types.h"

extern "C" {

// What MessageCentre_Post calls in the game that is not yet ported, so that it can run outside it (the benches point it at a recorder).
struct MessageCentreEnv {
    u32(__fastcall *handleMessage)(u8 *thing, void *, u8 *msg);   // Thing_HandleMessage 0x50b550 (agent C's; thiscall, pops 4)
};
// MessageCentre_Post's calls go to env (nullptr: the game's).
void MessageCentre_SetEnv(const MessageCentreEnv *env);

// 0x40f970 (thiscall on a slot; name unconfirmed): the destructor of one of the message centre's slots, a dword and then a std::map of tree B at +4 (StdTree_DtorB).
void __fastcall MessageSlot_Dtor(u8 *slot);
// 0x405460 (thiscall on a slot; name unconfirmed): the constructor of one of the message centre's slots: the map's tree (StdTree_CtorB) at +4, its multi flag +0xc = 0
// and its two allocator / comparator bytes +4 and +5 = 0 (the original copies an uninitialised stack byte there; the STL lock it takes is left out). Returns the slot.
u8 *__fastcall MessageSlot_Ctor(u8 *slot);
// 0x415e00 (cdecl; name unconfirmed): the message centre (the global 0x788d3c), made on the first call: 0x244 bytes (Mem_Alloc) whose 29 slots are constructed
// (CRT_VectorCtorIterator with MessageSlot_Ctor / MessageSlot_Dtor); a failed allocation stores and returns 0.
u8 *__cdecl MessageCentre_Instance();

// 0x4a8bd0 (thiscall; name unconfirmed): the 29 slots of 0x14 bytes from +0 constructed in place (CRT_VectorCtorIterator with MessageSlot_Ctor / MessageSlot_Dtor),
// as Instance does for its block; returns the centre. Called from 0x4a8860, 0x4dd139, 0x50b7e2 and 0x51a068.
u8 *__fastcall MessageCentre_CtorSlots(u8 *centre);

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

// 0x40fb10 (thiscall on the message centre; msg, pops 4; name invented; agent B's hub of the 0x463060 closure): the message's type is its
// slot 0. For type 0x1b (a thing gone) the thing it names (the word +8, in the thing table 0x7cfb90) has its handle (Thing_GetHandle, read
// again for each slot) erased from every slot's map where Tree16a_Find finds it (StdTree_EraseB; the count kept in the original is unused,
// its assert on [0x7cfb90] left out). Then each thing in the map of the type's slot (no bound check, sic), in key order (its first node,
// StdTree_IncB, until the head, re-read each time), gets the message: the env's Thing_HandleMessage on the thing table's entry of the
// node's key. Returns 1.
u32 __fastcall MessageCentre_Post(u8 *centre, void *, u8 *msg);
}  // extern "C"
