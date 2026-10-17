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
}  // extern "C"
