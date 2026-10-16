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

// 0x4ddf50 (thiscall; person handle, pops 4): the person's place in the queue, 0 for the head; -1 (after a printed message) when it is not found (the chain ends at a
// person whose Visitor_IsState11To14 is false: the walk goes on from 0). A handle whose thing slot is empty is stepped over forever.
s32 __fastcall Object_QueuePosition(u8 *object, void *, u32 person);

}  // extern "C"
