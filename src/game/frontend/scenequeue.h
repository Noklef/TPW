// The queue of ids the front-end scene's object at +0x14 of [0xf85614] keeps (class 0x702da0, built by 0x5e4020's class; the ids
// 0x186, 0x187, 0x18e and so on that the front-end code pushes look like text ids): reimplementation of 0x5d6060 and 0x5d6110 (thiscall).
// The object's list sits at +8 (its head +0xc, tail +0x10, count +0x14; nodes {item, next, prev}), each item a 4-byte block holding an id.
// Named by the vtable until the class is known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5d6060 (slot 4 of the vtable 0x702da0; name invented, unconfirmed): while the advisor speaks ([0xf7966c]) Advisor_StopSpeech; then
// while the count (+0x14) is not 0 the head node is taken off (the next node made the head, its back link +8 cleared, or the tail +0x10
// cleared with none; the count lowered; the node freed) and its item freed (Mem_Free; with no head node Mem_Free(0) and nothing else).
void __fastcall Class702da0_Clear(u8 *object);

// 0x5d6110 (thiscall (id, clear), pops 8; name invented, unconfirmed): with clear not 0, Class702da0_Clear's work (inlined); then a new
// 4-byte item (Mem_Alloc) holding id pushed at the head of the list at +8 (List_PushHead). A failed allocation faults (sic).
void __fastcall Class702da0_Push(u8 *object, void *, u32 id, u32 clear);
}
