// The park's thing table: reimplementation of 0x515f30-0x516310, 0x50b350, 0x50b090, 0x509bc0 and the handle
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

struct Stream;

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

// 0x515660 (thiscall on the world block): the block (0x80239c) and its cells at +0x2d8 (0x8023a0) are recorded, the thing table initialised and the memory range 0 of 0x7cdb30
// set to {the block, 0x1da748} ("World Struture"). Returns 1.
u32 __fastcall World_Attach(u8 *world);
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
// 0x50b090 (thiscall on a thing; stream, save, version, unused): the thing's words +4, +6, +0xa and +8 (mX, mY, mMapChild, mMapParent) saved
// or loaded as a serializer does (Save_Field); a load that gets them all clears the byte at +3.
u32 __fastcall Thing_Serialize(u8 *thing, void *, Stream *stream, u32 save, s32 version, u32 unused);
// 0x509bc0 (thiscall; the same arguments): Thing_Serialize, its result as 0 or 1 (the base serializer of the tag system and message receiver models).
u32 __fastcall ThingBasic_Serialize(u8 *thing, void *, Stream *stream, u32 save, s32 version, u32 unused);
// 0x516310 (__thiscall on the world; handle): pushes it on the delete queue.
void __fastcall World_QueueDelete(void *world, void *edx, u32 handle);
// 0x515fb0 (__thiscall on the world): deletes every thing in use (Thing_Delete, unless
// byte +3 says it is already deleted), then releases the slot of every handle on the
// delete queue. World +0x1da738 is 2 meanwhile, then 1.
void __fastcall World_DeleteAllThings(u8 *world);
// 0x518ea0 (cdecl): while the delete queue has handles, the head's handle is popped (inlined) and its slot released (ThingSlot_Release).
void __cdecl Things_ReleaseDeleted();
// 0x5153c0 (cdecl; the atexit entry the static initialiser 0x515390 registers; name unconfirmed): once (bit 1 of the byte 0x7cf6d4), the
// delete queue destroyed: its vtable 0x700a9c, its tail 0, its nodes freed (operator delete) from the head on (each one's next read
// first), the head (cleared before) and the count 0.
void __cdecl ThingsToDelete_AtExit();
// 0x51ad40 (__thiscall; handle): a new node (operator new, 0x67acc0) at the head; the
// original writes through a null node when that fails.
void __fastcall HandleQueue_Push(HandleQueue *queue, void *edx, u32 handle);
// 0x51ad90 (__thiscall; &handle): the head's handle, its node deleted; an empty queue
// leaves the handle as it is. Returns the pointer.
u16 *__fastcall HandleQueue_Pop(HandleQueue *queue, void *edx, u16 *out);

// 0x50afe0 (thiscall on the thing; cell id, kind, pops 8; the print of the creation is left out): the handle word at +0 is cleared, the kind byte stored at +2, the byte at +3
// and the words at +8 and +0xa cleared; the thing takes a slot in the world's table (World_AddThing on [0x80239c]) and its handle; +4 and +6 are 0x80, the column byte at +5 is the cell
// id less 1 (to 7 bits) and the row byte at +7 that id less 1 shifted down 7; the thing goes at the head of the cell's chain (Obj_ReplaceHandle24 on the cell at [0x8023a0] +
// id * 0x44 - 0x44). Returns the thing.
u8 *__fastcall Thing_Construct(u8 *thing, void *, u32 cell, u32 kind);

// 0x50b6a0 (thiscall on the thing; x, y, pops 8): mX and mY (the words at +4 and +6) are set; when the cell (the high bytes of the new x and y, taken as column and row against
// the bytes at +5 and +7) changes the thing is first unlinked from the old cell's chain (Things_UnlinkFromChain on the cell id (row << 7) + column + 1 of [0x8023a0]) and
// afterwards put at the head of the new one's (Obj_ReplaceHandle24). The assert on the new cell is left out.
void __fastcall Thing_SetPos(u8 *thing, void *, u32 x, u32 y);

// 0x4fa930 (thiscall on the thing; x, y, pops 8): the navigation block at +0xd4 gets the position (x, y as 16-bit values shifted left 8) and no speed (Nav_SetPosVel), the
// position is read back to the words at +0x190 and +0x194 (Nav_GetPos), then the thing moves (Thing_SetPos with the same x and y).
void __fastcall Thing_PlaceAt(u8 *thing, void *, u32 x, u32 y);

// 0x4f7e20 (thiscall on the thing; x, y, pops 8; name unconfirmed): the dword at +0x39c = 0 and the one at +0x10 = 3 (Set4_4217f0 on +0xc), then
// Thing_PlaceAt(x, y). Its callers (0x4ea240-0x4ebfb0) place things they look up through 0x5da3c0.
void __fastcall Thing_ResetPlaceAt(u8 *thing, void *, u32 x, u32 y);

// 0x4fcac0 (thiscall on the object): with sprites in folder 3, the dword at +0x20 (with the top bit set when the dword at +0x24 is 2). Without, the world's random seed is set to
// the thing's handle (World_SetSeed on [0x80239c]) and a random sprite of folder 1 returned.
u32 __fastcall Thing_SpriteChoice(u8 *thing);

// 0x4e2200 (thiscall on the thing; the object definition by the id word at +0xe, ObjectDefs_Find on 0x7890a0): the definition's dword at +0x158 (1 to 4) picks the record 0 to
// 3 of the table at [0x78ba8c] (Records_DataIfTerminated); any other value gives 0.
u8 *__fastcall Thing_IngredientRecord(u8 *thing);
}

// What the table calls in the game, so that it can run outside it.
struct ThingTableEnv {
    void(__cdecl *free)(void *block);    // operator delete (0x67acb0)
    void *(__cdecl *alloc)(u32 size);    // operator new (0x67acc0)
    void(__cdecl *destroy)(void *thing); // Thing_Delete (0x50b780, __thiscall)
};
// The table's calls go to env (nullptr: the game's).
void Things_SetEnv(const ThingTableEnv *env);

// What Thing_Detach calls in the game that is not ported yet, so that it can run outside it (the benches point it at a recorder).
struct ThingDetachEnv {
    void(__cdecl *rideDestroy)(u32 ride);   // Ride_Destroy (0x463c80, agent B's)
};
// Thing_Detach's calls go to env (nullptr: the game's).
void Things_SetDetachEnv(const ThingDetachEnv *env);

// What Thing_Delete calls in the game that is not ported yet (agent C's, thiscall on the thing; the benches point them at recorders).
struct ThingDeleteEnv {
    void(__fastcall *kind3)(u8 *thing);      // 0x4dd0a0
    void(__fastcall *kind4)(u8 *thing);      // 0x4d9fe0
    void(__fastcall *kind5)(u8 *thing);      // 0x4d6c90
    void(__fastcall *kind6)(u8 *thing);      // 0x4d4620
    void(__fastcall *kind7)(u8 *thing);      // 0x4d5eb0
    void(__fastcall *kinds9to11)(u8 *thing); // 0x4c9040 (kinds 9-0xe, 0x10, 0x11)
};
// Thing_Delete's calls go to env (nullptr: the game's).
void Things_SetDeleteEnv(const ThingDeleteEnv *env);

extern "C" {

// 0x4d91f0 (thiscall; handle, pops 4): when the word at +0x24 is not 0 the thing it names (in the table at 0x7cfb90) gets the word at +8 = handle; the
// thing the handle names gets the words at +8 = 0 and +0xa = the old word at +0x24, and the word at +0x24 = handle.
void __fastcall Obj_ReplaceHandle24(u8 *object, void *, u32 handle);

// 0x4cd300 (stdcall; handle, pops 4): the thing the slot (handle & 0xffff) of the thing table at 0x7cfb90 holds (20-byte slots; the debug message of the original is gone).
u8 *__stdcall Thing_ByHandle(u32 handle);

// 0x502d20 (thiscall): the thing constructor (Thing_Construct) with cell 1 and kind 0xe, then the five 0x10-byte records at +0x1414 get -1 in their first dword and 1 in the byte at
// +4. Returns the object. (The cell argument the original pushes has the object's high address bits above the 1: Thing_Construct takes 16 bits of it.)
u8 *__fastcall ThingKindE_Ctor(u8 *object);

// 0x50b980 (thiscall; name invented, unconfirmed): a thing taken out of the world: unlinked from its map cell's chain (Things_UnlinkFromChain
// on the cell id (row +7 << 7) + column +5 + 1 of [0x8023a0], its handle word); a ride (kind +2 3) destroyed by its +0x20 (the env's Ride_Destroy,
// 0x463c80); a thing of kind 1, 4, 5, 6, 7, 8 or 0x12 that is the one at 0x877350 clears it; then erased from the message centre
// (MessageCentre_RemoveThing on MessageCentre_Instance). Returns the count erased.
u32 __fastcall Thing_Detach(u8 *thing);

// 0x50b780 (thiscall): a thing deleted: message 1b of its handle posted (Object_PostMessage: agent B's unported 0x40fb10 through the objects'
// env), then taken out of the world by its kind (+2): 1 Visitor_Detach, 8 Researcher_Detach, 0xf Weather_Detach, 0x12 Staff_Detach; 3, 4, 5,
// 6, 7 and 9-0xe, 0x10, 0x11 agent C's unported 0x4dd0a0, 0x4d9fe0, 0x4d6c90, 0x4d4620, 0x4d5eb0 and 0x4c9040 (through ThingDeleteEnv, the
// game's addresses in game); 2, 0x13 and others only the original's debug prints. Then +3 = 1, the handle queued for deletion
// (World_QueueDelete) and the kind +2 = 2.
void __fastcall Thing_Delete(u8 *thing);
}  // extern "C"
