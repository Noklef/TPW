// The queue of ids the front-end scene's object at +0x14 of [0xf85614] keeps (class 0x702da0, built by 0x5e4020's class; the ids
// 0x186, 0x187, 0x18e and so on that the front-end code pushes look like text ids): reimplementation of 0x5d6060 and 0x5d6110 (thiscall).
// The object's list sits at +8 (its head +0xc, tail +0x10, count +0x14; nodes {item, next, prev}), each item a 4-byte block holding an id.
// Named by the vtable until the class is known.
#pragma once

#include "../../../re/types.h"

// Class702da0_Step's callees (tests/fncmp/scenequeuestep.py gives them recorders).
struct SceneQueueEnv {
    u32(__cdecl *say)(s32 id);                            // Advisor_Say
    u32(__cdecl *now)();                                  // Time_GetMs
    u32(__fastcall *pushHead)(u8 *list, void *, u32 item);  // List_PushHead
};
void SceneQueue_SetEnv(const SceneQueueEnv *env);

extern "C" {

// 0x5d6060 (slot 4 of the vtable 0x702da0; name invented, unconfirmed): while the advisor speaks ([0xf7966c]) Advisor_StopSpeech; then
// while the count (+0x14) is not 0 the head node is taken off (the next node made the head, its back link +8 cleared, or the tail +0x10
// cleared with none; the count lowered; the node freed) and its item freed (Mem_Free; with no head node Mem_Free(0) and nothing else).
void __fastcall Class702da0_Clear(u8 *object);

// 0x5d6110 (thiscall (id, clear), pops 8; name invented, unconfirmed): with clear not 0, Class702da0_Clear's work (inlined); then a new
// 4-byte item (Mem_Alloc) holding id pushed at the head of the list at +8 (List_PushHead). A failed allocation faults (sic).
void __fastcall Class702da0_Push(u8 *object, void *, u32 id, u32 clear);

// 0x5d61a0 (thiscall (id, time), pops 8; name invented, unconfirmed): +0x18 = 1, +0x1c = time, +0x24 = id.
void __fastcall Class702da0_Show(u8 *object, void *, u32 id, u32 time);

// 0x5d5f80 (thiscall; name invented, unconfirmed): while the advisor speaks ([0xf7966c]) only +0x28 = 1. With a count (+0x14) the tail node is taken off (the tail +0x10 its back link +8, whose next +4 is
// cleared, or the head +0xc cleared), its item spoken (Advisor_Say; no tail node faults, sic) and freed, +0x28 = 1. Else with +0x18 set: with +0x28 set the time (+0x20) taken and +0x28 cleared; once
// the time is past +0x20 + +0x1c (unsigned, wrapping) +0x20 = now and the id +0x24 is pushed on the queue of [[0xf85614] + 0x14], which need not be this one (a failed allocation faults, sic). Last +0x28 = 0.
void __fastcall Class702da0_Step(u8 *object);
}
