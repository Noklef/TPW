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
    void(__fastcall *placeSprite)(u8 *visitor, void *, float t);                          // Person_PlaceSprite 0x4fa030
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
// 0x4feb10 (thiscall): the sprite script +0x210, when set, killed (SpriteScript_Kill) and cleared, the reference +0xc's killed
// (Obj_KillSlot), state 0x11. Name invented, unconfirmed.
void __fastcall Visitor_KillScripts(u8 *visitor);
// 0x4ff520 (thiscall): unless Game_PollRideElement1 says 0, the walk to a random ticket booth at a random spot (state 2). Name invented,
// unconfirmed.
void __fastcall Visitor_GoToTicketBooth(u8 *visitor);
// 0x4ff730 (thiscall): the walking speed +0xc2: [0x75c7f4] when no other arrival is due (World_ArrivalVehicleNotSize1 0) and
// Game_TakeRideElement1 says 3 ("The bus is coming, RUUUUUUUUUUN"), else [0x75c7f2] for a handle whose low 2 bits are 0, [0x75c7f0] for
// others; one step (Sprite_WalkNav), arrived: state 3 with the park's dword +0x1da710 set, else 4. Name invented, unconfirmed.
void __fastcall Visitor_StepWalkingIn(u8 *visitor);
// 0x4ffb20 (thiscall): with +0x1da710 set or the ride element 1 of +0x1da732 not 1, back to a random ticket booth (state 2); else one step,
// arrived: +0x1d8 the visitor count to date (World_AddVisitorToDate), state 6. Name invented, unconfirmed.
void __fastcall Visitor_StepEntering(u8 *visitor);
// 0x500a50 (thiscall): with +0x1bc above 0, the low byte of __ftol(happiness +0x19c) set, +0x1da710 0 and a plain cell under it
// (AnalyserCell_TypePlain) the visitor stays ("Make up your mind!"); otherwise one step: arrived, state 0x13; stuck (2), it stays too:
// +0x188 0, state 6. Name invented, unconfirmed.
void __fastcall Visitor_StepLeavingPark(u8 *visitor);
// 0x4fc890 (thiscall; called by 0x501d3f): the visitor's state set again to its own state +0x224 (Visitor_SetState). Name invented, unconfirmed.
void __fastcall Visitor_ReenterState(u8 *visitor);

// 0x4faec0 (thiscall (cell), pops 4; the constructor, its name invented): the moving person (MovingPerson_Ctor (cell, 1)), the visitor's
// fields 0x19c-0x22a cleared (+0x220 / +0x224 6), +0x1d4 the park's +0x1da70c; the exit level +0x1bc gBalanceLevel's (0x78504c) give or
// take its variance (0x785050: roll % (2 * var) - var); the type +0x1f0 roll % the type count (0x7851d4), its starting cash (0x7850e8 + 12 *
// type) give or take the percentage 0x785054 into +0x1a0 (at least 0), happiness +0x19c 50.0, +0x1a4 / +0x1a8 roll % 50, +0x1ac roll % 30;
// a roll passed over, the prankery byte +0x1c0 100 + handle % 3 when roll % 100 is below 0x7850dc, else 0. The park's random state set to
// the handle (World_SetSeed), the sprite (+0x20, folder 0) started (ScriptRef_Start 1), the speed +0xc2 [0x75c7f2]; on a plain cell
// (AnalyserCell_TypePlain) +0x1d8 the visitor count to date and state 6, else the walk to a random crossing (bus stop side) at x a roll's
// byte, y 0xc8 (state 0, or 6 when it does not start), the sprite placed (0.0) and the particle 0x13; +0x204 and the words +0x1e0-+0x1ee
// cleared; message 0x1c of its handle posted. Returns the visitor.
u8 *__fastcall Visitor_Ctor(u8 *visitor, void *, u32 cell);
}
