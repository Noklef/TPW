// The object's message handler (the part of Object_HandleMessage 0x4dd320): reimplementation. Named by what it does.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4dd320 (thiscall on the object; message, pops 4): the message's number comes from its slot 0. For 0xb (the end of the day) each of the object's six history rings
// (temp dword at +0x1a8, +0x230, +0x2b8, +0x70, +0xf8, +0x340, in this order; then the data from +4, the current index at +0x7c, the length at +0x80 and the wrapped byte at +0x84)
// is advanced: the index plus 1 (back to 0, with the byte set to 1, when not below the length, signed), the temp stored at that index and cleared. For 0x1b with the
// object's assigned staff member (the word at +0x5e) as the word at +8 of the message, that word and the dword at +0x60 are cleared (a debug print is left out). Returns
// 0 in those two cases, else the message number.
u32 __fastcall Object_HandleDayMessage(u8 *object, void *, u8 *message);

// 0x4ddb90 (thiscall on the object; person handle, pops 4): the person goes to the end of the queue of persons whose head is the word at +0x3c (each person's next is the
// word at +0x228 of its thing, its previous the word at +0x22a): the head when the queue is empty, else the last one's next; its previous becomes the last (0 for none) and
// its next 0.
void __fastcall Object_QueueAddPerson(u8 *object, void *, u32 person);

// 0x4ddd20 (thiscall on the object; person handle, pops 4; name unconfirmed): element 0 of the object's ride record (Rides_FindById of the dword at +0x24) = 0 when it
// is the person; then the person leaves the queue: the one ahead (+0x22a) gets its next, the one behind (+0x228) its previous (the head at +0x3c when none is ahead),
// and both its words are cleared. The debug checks are not reproduced.
void __fastcall Object_QueueRemovePerson(u8 *object, void *, u32 person);

// 0x4ddf50 (thiscall; person handle, pops 4): the person's place in the queue, 0 for the head; -1 (after a printed message) when it is not found (the chain ends at a
// person whose Visitor_IsState11To14 is false: the walk goes on from 0). A handle whose thing slot is empty is stepped over forever.
s32 __fastcall Object_QueuePosition(u8 *object, void *, u32 person);

// 0x4de130 (thiscall on the object; out word, pops 4): the id at +0x3a when it is set; else the cell Object_NeighbourCellId gives is followed through Map_FindFacingNeighbour
// (up to 1000 steps, counted at +0x40, the last cell found kept at +0x3a). Not reaching the end of the chain in time gives 0 (a message is printed, left out).
void __fastcall Object_BackOfQueue(u8 *object, void *, u16 *out);

// 0x4de110 (thiscall on the object; out word, pops 4): Object_BackOfQueue; returns out.
u16 *__fastcall Object_BackOfQueueTo(u8 *object, void *, u16 *out);

// 0x4ddf40 (thiscall on the object): Object_QueuePosition of the person whose handle word is 0 (the stack word it passes is the object's pointer with its low word cleared (sic)).
s32 __fastcall Object_QueuePositionOfNone(u8 *object);

// 0x4dda20 (thiscall): whether the queue of persons (Object_QueuePosition for the person 0, i.e. its length) is shorter than the dword at +0x40 times 4 (unsigned).
u32 __fastcall Object_QueueHasRoom(u8 *object);

// 0x4dd920 (thiscall; name unconfirmed): whether the object takes people into its queue: flag 4 of +0x32, the dword at +0x19c neither 1 nor 4, the dword at +0x68 set,
// a back of the queue (Object_BackOfQueue, not 0) and room (Object_QueuePosition of the person 0 below the dword at +0x40 times 4, unsigned); and by the definition's
// kind (+0x9c of ObjectDefs_Find of the id at +0xe, found anew for each test): for 3 Object_LinkOtherOpen, for 1 and 2 the dword at +0x2c set.
u32 __fastcall Object_QueueOpen(u8 *object);

// 0x4de4a0 (thiscall; name unconfirmed): whether the object's entrance meets the path. With flag 8 of +0x32: the cell of its back of the queue (Object_BackOfQueue;
// 0 gives 0) in the cells at [0x8023a0] (0x44 bytes, the id less 1), whose neighbour bits (MapCell_Neighbours) differ from its direction (MapCell_Direction). Without:
// its own cell (the word at +0x36) and the bit its angle (+0x10) faces (0: 0x10, 0x5a: 4, 0xb4: 1, 0x10e: 0x40); the cell one step towards the opposite bit
// (Nibble_Scale of it, Map_StepCellInDirection) is a type 1 cell with the bit among its neighbours, and its own cell has the opposite one. Another angle leaves the
// original's bit a stale stack byte (the port's is 0; the game's angles are quarter turns).
u32 __fastcall Object_EntranceOnPath(u8 *object);

// 0x4de840 (thiscall; place, out cell word, out x and y bytes; pops 0x10; x87; name unconfirmed): where a person at a place in the queue stands. From the cell
// Object_NeighbourCellId gives, Map_FindFacingNeighbour is followed while the place is 4 or more (less 4 each step) and the cell is not 0; the cell goes to *cell. Along =
// __ftol(place * 0.25 * 255) (the low byte), across = World_Rand % 28 + 0x72. Along up to 0x80: by the cell's direction (MapCell_Direction) 1 (x, y) = (across,
// along), 4 (0xff - along, across), 0x10 (across, 0xff - along), 0x40 (along, across); another writes neither (a debug print is left out). Above 0x80: the direction of
// the cell after it (Map_FindFacingNeighbour), or at the queue's end the opposite of the first of 1, 4, 0x10, 0x40 among the cell's neighbour bits (asked twice) whose
// step (Map_StepCellInDirection; a null one is read) is a type 1 cell, else the cell's own direction; then as above but 4 gives (along + 0x80, across). Returns 0.
u32 __fastcall Object_QueueSpot(u8 *object, void *, u32 place, u16 *cell, u8 *x, u8 *y);

// 0x4dec30 (thiscall; place, out cell word, out x and y bytes; pops 0x10; x87; name unconfirmed): Object_QueueSpot's along and across (in that order: __ftol of
// place * 0.25 * 255, then World_Rand), the back of the queue (Object_BackOfQueue) to *cell, then Object_BackOfQueue asked again (sic: its cell unused), and the
// first switch of Object_QueueSpot on the direction of the object's own cell (the word at +0x36). Returns 0.
u32 __fastcall Object_EntranceQueueSpot(u8 *object, void *, u32 place, u16 *cell, u8 *x, u8 *y);

// 0x4de7e0 (thiscall; the same arguments, pops 0x10; name unconfirmed; called by 0x501214): Object_QueueSpot for an object whose byte +0x32 has
// bit 3 set, else Object_EntranceQueueSpot (the debug print of whether the place is below 4 left out). Returns its result.
u32 __fastcall Object_AnyQueueSpot(u8 *object, void *, u32 place, u16 *cell, u8 *x, u8 *y);

// 0x4df290 (thiscall; name unconfirmed): whether the object can be opened: the dword at +0x19c is not 1, 4 or 2, the dword at +0x64 is 0, Object_EntranceOnPath, and
// for a definition (ObjectDefs_Find of the id at +0xe) of kind 3 (+0x9c) Object_LinkOtherOpen.
u32 __fastcall Object_CanOpen(u8 *object);
}  // extern "C"
