// The event queue (0x5fbde0-0x5fc31f, thiscall unless noted): a function-static object at
// 0xfa7cb0 holding six channels of listeners, each a context and a __stdcall callback, behind a
// critical section. Post calls every listener on a channel at once (nothing is queued); the
// screen posts window events on channel 0.
//
// The originals' quirks are kept: channel 6 is "none" and anything above it is not checked; the
// constructor initializes the critical section even when allocating it failed; Post enters it
// even when there is none (only leaving is guarded); Listen writes through a null node when the
// node's allocation fails; Unlisten with context 0 removes every listener with that callback,
// whatever its context; Clear spins forever on a channel whose count is above zero with no head;
// the lists' destructors free the nodes but not their listeners.
#pragma once

#include <cstddef>

#include "../../../re/types.h"

struct EventListener {
    void *context;
    void(__stdcall *callback)(u32 channel, const void *event, void *context);  // +4
};
struct EventNode {
    EventListener *listener;
    EventNode *next;  // +4
    EventNode *prev;  // +8
};
struct EventList {
    u32 vtable;       // 0x703648 (its base's 0x70364c while being destroyed)
    EventNode *head;  // +4
    EventNode *tail;  // +8
    u32 count;        // +0xc
};
const u32 kEventChannels = 6;
const u32 kEventNone = 6;
struct EventQueue {     // 0xfa7cb0
    void *lock;         // a CRITICAL_SECTION (0x18 bytes) of its own allocation
    EventList lists[kEventChannels];  // +4
};
static_assert(sizeof(EventList) == 0x10, "EventList is 0x10 bytes");
static_assert(sizeof(EventQueue) == 0x64, "EventQueue is 0x64 bytes");

struct EventQueueEnv {
    EventQueue *instance;              // 0xfa7cb0
    u32 listVtable;                    // 0x703648
    u32 baseVtable;                    // 0x70364c
    void *(__cdecl *alloc)(u32);       // 0x67acc0
    void(__cdecl *free)(void *);       // 0x67acb0
    s32(__cdecl *atExit)(void(__cdecl *)());  // 0x67a7d0
    void(__cdecl *onExit)();           // 0x5fbe10, what RegisterExit registers
    void(__stdcall *const *initialize)(void *);  // 0x6fd144 InitializeCriticalSection
    void(__stdcall *const *remove)(void *);      // 0x6fd0d0 DeleteCriticalSection
    void(__stdcall *const *enter)(void *);       // 0x6fd140 EnterCriticalSection
    void(__stdcall *const *leave)(void *);       // 0x6fd130 LeaveCriticalSection
};
// nullptr restores the game's.
void EventQueue_SetEnv(const EventQueueEnv *e);
const EventQueueEnv *EventQueue_GetEnv();

extern "C" {
void __cdecl EventQueue_StaticInit();      // 0x5fbde0: Construct, RegisterExit
EventQueue *__cdecl EventQueue_Construct();  // 0x5fbdf0: the constructor on 0xfa7cb0
void __cdecl EventQueue_RegisterExit();    // 0x5fbe00
void __cdecl EventQueue_AtExit();          // 0x5fbe10: the destructor on 0xfa7cb0
EventQueue *__fastcall EventQueue_Ctor(EventQueue *q);  // 0x5fbe20
EventList *__fastcall EventList_Ctor(EventList *l);     // 0x5fbe90
void __fastcall EventList_Dtor(EventList *l);           // 0x5fbeb0: frees the nodes
EventList *__fastcall EventList_DeletingDtor(EventList *l, void *edx, u32 flags);      // 0x5fbef0
void __fastcall EventQueue_Dtor(EventQueue *q);         // 0x5fbf10: Clear; the critical section
// 0x5fbf80 (channel, &event): each listener's callback(channel, &event, context), in order.
void __fastcall EventQueue_Post(EventQueue *q, void *edx, u32 channel, const void *event);
void __fastcall EventQueue_GuardRelease(void **guard);  // 0x5fc010: leaves *guard, if any; 0
// 0x5fc030 (channel, callback, context): a new listener at the channel's tail.
void __fastcall EventQueue_Listen(EventQueue *q, void *edx, u32 channel, void *callback, void *context);
// 0x5fc100 (channel, callback, context): removes the matching listeners (context 0: any context).
void __fastcall EventQueue_Unlisten(EventQueue *q, void *edx, u32 channel, void *callback, void *context);
void __fastcall EventQueue_Clear(EventQueue *q);  // 0x5fc220: every listener and node freed
EventList *__fastcall EventListBase_DeletingDtor(EventList *l, void *edx, u32 flags);  // 0x5fc2d0
// 0x47e9d0 (cdecl; value; name unconfirmed): an event {4, value} (0x14 bytes on the stack, the rest unset) posted on channel 0 of the queue;
// the game's window procedure (0x46b752) posts one.
void __cdecl EventQueue_PostKind4(u32 value);
// 0x47ea00 (cdecl; name unconfirmed): on channel 0 of the queue an event {6, 0} (0x14 bytes on the stack, the rest unset), then the same
// bytes as {7, the screen's width 0x7a1968, its height 0x7a1930, byte 0x10 (the dword's other bytes left), 0}; then the two COM objects
// of 0x7b4ac0 released and cleared (Obj_ReleaseTwoComMembers inlined: +0xa8 / +0xa4 cleared first).
void __cdecl EventQueue_PostScreenReset();
}
