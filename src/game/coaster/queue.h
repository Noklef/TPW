// A coaster's two queues of riders: rings of u32 (peeps) at coaster+0xb0 (the queue) and
// +0xd0 (the boarding queue), coasters looked up by index in the table at 0x790bd0.
// A ring (0x20 bytes): +0 capacity, +4 buffer, +8 limit, +0xc count, +0x10 reserved,
// +0x14 head (the next to take), +0x18 tail (the next free slot), +0x1c a slot the takes
// stop at (boarding only). Its pointers step to the next slot modulo the capacity.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x43b0e0: appends peep to coaster index's queue when count + reserved is below the
// capacity (else nothing).
void __cdecl Coaster_QueuePush(u32 index, u32 peep);

// 0x43b130: appends peep to the coaster's queue when it has room, else to the boarding
// queue when that has room.
void __cdecl CoasterQueue_Add(u8 *coaster, u32 peep);

// 0x43b1f0: the room left before the queue's limit: limit - reserved - count, at least 0.
s32 __cdecl Coaster_QueueRoom(u32 index);

// 0x43b220: takes the next peep from coaster index's boarding queue; 0 when it is empty
// or the head has reached the stop slot.
u32 __cdecl Coaster_BoardingPop(u32 index);

// 0x43b390: moves peeps from the queue to the boarding queue, min(boarding capacity -
// reserved - count, queue count + reserved) times (when positive). Each step takes the
// queue's head (0 when it is empty, with no stop slot check) and, when not 0, appends it
// to the boarding queue if it has room.
// sic: a peep taken when the boarding queue is full is dropped.
void __cdecl CoasterQueue_Advance(u8 *coaster);

// 0x43b480: empties one side of each of the train's cars (from +8, +0xc of them, 0x60 each
// at +0x28): side non-zero the riders at car +0x28 (count +0x30), else +0x2c (+0x34). Each
// rider is appended to the boarding queue when it has room (else dropped, sic), its seat
// part ([car+4]+0x2c) released on the ride's instance (0x7a4610[car+0xc] + 8), the train's
// rider count (+0x10) stepped down and the seat cleared.
void __cdecl CoasterTrain_Unload(u8 *coaster, u8 *train, u32 side);

// 0x43b080: an empty ring of capacity slots (allocated zeroed), its limit 0 and its head,
// tail and buffer the first slot; no stop slot.
void __cdecl CoasterRing_Init(u8 *ring, u32 capacity);

// 0x43b0c0: frees the ring's buffer (the pointer is left as it was).
void __cdecl CoasterRing_Free(u8 *ring);
}
