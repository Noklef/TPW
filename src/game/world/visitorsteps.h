// A visitor's steps in some of its states (the update's per-state calls, 0x501a00 and on): arriving at the park gate, leaving a ride,
// walking to and waiting at the bus stop, finding a place in an object's queue. Names invented (unconfirmed) but Visitor_FindQueueDestination,
// after the original's own debug string ("QQQ - FindQueueDestination ...").
#pragma once

#include "../../../re/types.h"

extern "C" {
// The calls these make through an environment (the fncmp driver's recorders; the game's by default, the ports):
struct VisitorStepsEnv {
    u32(__fastcall *walkNav)(u8 *visitor);                                                // Sprite_WalkNav 0x4fa2a0
    u32(__fastcall *setDestination)(u8 *visitor, void *, s32 x, s32 y);                   // Sprite_SetDestination 0x4fa5f0
    u32(__fastcall *setDestinationCell)(u8 *visitor, void *, u32 cell);                   // Sprite_SetDestinationCell 0x4fa530
    void(__fastcall *spawnParticle)(u8 *visitor, void *, s32 effect);                     // Sprite_SpawnParticle 0x4fa470
    void(__fastcall *thingDelete)(u8 *thing);                                             // Thing_Delete 0x50b780
    void(__fastcall *setState)(u8 *visitor, void *, u32 state);                           // Visitor_SetState 0x501db0
    u16 *(__fastcall *backOfQueueTo)(u8 *object, void *, u16 *out);                       // Object_BackOfQueueTo 0x4de110
    s32(__fastcall *queuePosition)(u8 *object, void *, u32 person);                       // Object_QueuePosition 0x4ddf50
    u32(__fastcall *anyQueueSpot)(u8 *object, void *, u32 place, u16 *cell, u8 *x, u8 *y);  // Object_AnyQueueSpot 0x4de7e0
};
void VisitorSteps_SetEnv(const VisitorStepsEnv *env);

// 0x4ff7f0 (thiscall): with the park's dword +0x1da710 set, or element 1 of the ride record [0x80239c]'s +0x1da732 names other than 1:
// the countdown +0x200 down by one while set, else +0x188 = 1, the walk to a random crossing park side (state 0x12) and +0x1bc 0. Else, not
// yet in (+0x204 0): the walk to the ticket booth A when the visitor stands on it, else B when on it, else either (state 2, +0x200 0); in
// (+0x204 set) and the visitor's cell's word -0x20 its own handle: the walk to a random entrance (state 5). Name invented, unconfirmed.
void __fastcall Visitor_StepArriving(u8 *visitor);
// 0x500900 (thiscall): one step of the walk off a ride (Sprite_WalkNav): 2 clears +0x1dc for state 6, other non-zero answers wait. Arrived:
// a pending destination +0x1de becomes +0x1dc (cleared); one of thing kind 3 (the table 0x7cfb90's slot by the whole word) gives its back
// of the queue, and a walk there that starts is state 10; another kind clears +0x1dc. Otherwise state 6. Name invented, unconfirmed.
void __fastcall Visitor_StepLeavingRide(u8 *visitor);
// 0x500ad0 (thiscall): the bus stop's four cells (the stop, the one after, the row before and that one's next); unless Game_PollRideElement1
// says 0, the walk to a random one of them at a random spot in the cell (state 0x14). Name invented, unconfirmed.
void __fastcall Visitor_GoToBusStop(u8 *visitor);
// 0x500bd0 (thiscall): Game_TakeRideElement1 4 (the bus in): the visitor standing on its own cell's mark ((cell word -0x20) its handle)
// boards: the particle 0x13 and Thing_Delete. 1-3: the bus stop's four cells; with World_ArrivalVehicleNotSize1 the back two (else the
// front two): unless on one already, the walk to one of them at a random spot (state 0x14). Name invented, unconfirmed.
void __fastcall Visitor_StepAtBusStop(u8 *visitor);
// 0x501160 (thiscall): the object +0x1dc names (the table 0x7cfb90's slot by the whole word): the visitor's place in its queue (none: 0);
// stored at +0x1f1, the queue spot's cell and spot (Object_AnyQueueSpot), the walk there; started: state 0xc and 1, else 0.
u32 __fastcall Visitor_FindQueueDestination(u8 *visitor);
}
