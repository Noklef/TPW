// The park's thing table: reimplementation of 0x515f30-0x516310, 0x50b350 and the handle
// queue 0x51ad40/0x51ad90.
//
// Every thing in the park (rides, shops, people, scenery, ...) has a slot in one table of
// 0x2800 20-byte slots (gThingSlots, 0x7cfb90); a thing is named by its slot index, a u16
// handle that the thing keeps at +0 (0: none, slot 0 is never used). The slots in use form
// a list, newest first (gThingsUsed, 0x7cf56c); the free ones a stack (gThingsFree,
// 0x801bbc). The methods are __thiscall on the world (gWorldPtr, 0x7cf83c), which they ignore,
// and return handles through a pointer. Things are deleted in two steps: Thing_Delete
// (0x50b780) queues the handle (World_QueueDelete), and the slot is released later.
#pragma once

#include "../../../re/types.h"

struct ThingSlot {
    void *thing;         // +0
    u32 id;              // +4 the slot's index
    ThingSlot *next;     // +8 the next (older) slot in use
    ThingSlot *prev;     // +0xc
    ThingSlot *nextFree; // +0x10
};
static_assert(sizeof(ThingSlot) == 0x14, "ThingSlot is 20 bytes");

// A stack of handles (the delete queue, gThingsToDelete 0x801f38): 12-byte nodes, pushed
// and popped at the head.
struct HandleNode {
    u16 handle;        // +0 (+2 unset)
    HandleNode *next;  // +4 towards the tail
    HandleNode *prev;  // +8
};
struct HandleQueue {
    u32 unknown00;
    HandleNode *head;  // +4
    HandleNode *tail;  // +8
    u32 count;         // +0xc
};

extern "C" {
// 0x515f30: every slot free (1 to 0x27ff stacked in order), none in use.
void __cdecl Things_InitTable();
// 0x516130 (&handle): the newest thing (0: none). Returns the pointer.
u16 *__fastcall World_FirstThing(void *world, void *edx, u16 *out);
// 0x516160 / 0x5161b0 (&handle, thing): the thing after / before it in the list (0: none).
u16 *__fastcall World_NextThing(void *world, void *edx, u16 *out, const u8 *thing);
u16 *__fastcall World_PrevThing(void *world, void *edx, u16 *out, const u8 *thing);
// 0x516270 (&handle, thing): a free slot for the thing, put first in the list; the
// handle is 0 when the table is full.
u16 *__fastcall World_AddThing(void *world, void *edx, u16 *out, void *thing);
// 0x516200 (__thiscall on a slot in use): takes it off the list, frees it and deletes
// its thing (operator delete, 0x67acb0).
void __fastcall ThingSlot_Release(ThingSlot *slot);
// 0x50b350 (__thiscall on a thing; &handle): its handle (+0). Returns the pointer.
u16 *__fastcall Thing_GetHandle(const u8 *thing, void *edx, u16 *out);
// 0x516310 (__thiscall on the world; handle): pushes it on the delete queue.
void __fastcall World_QueueDelete(void *world, void *edx, u32 handle);
// 0x515fb0 (__thiscall on the world): deletes every thing in use (Thing_Delete, unless
// byte +3 says it is already deleted), then releases the slot of every handle on the
// delete queue. World +0x1da738 is 2 meanwhile, then 1.
void __fastcall World_DeleteAllThings(u8 *world);
// 0x518ea0 (cdecl): while the delete queue has handles, the head's handle is popped (inlined) and its slot released (ThingSlot_Release).
void __cdecl Things_ReleaseDeleted();
// 0x51ad40 (__thiscall; handle): a new node (operator new, 0x67acc0) at the head; the
// original writes through a null node when that fails.
void __fastcall HandleQueue_Push(HandleQueue *queue, void *edx, u32 handle);
// 0x51ad90 (__thiscall; &handle): the head's handle, its node deleted; an empty queue
// leaves the handle as it is. Returns the pointer.
u16 *__fastcall HandleQueue_Pop(HandleQueue *queue, void *edx, u16 *out);
}

// What the table calls in the game, so that it can run outside it.
struct ThingTableEnv {
    void(__cdecl *free)(void *block);    // operator delete (0x67acb0)
    void *(__cdecl *alloc)(u32 size);    // operator new (0x67acc0)
    void(__cdecl *destroy)(void *thing); // Thing_Delete (0x50b780, __thiscall)
};
// The table's calls go to env (nullptr: the game's).
void Things_SetEnv(const ThingTableEnv *env);
