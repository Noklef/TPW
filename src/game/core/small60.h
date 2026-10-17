// Three small callers of other ports: reimplementation of 0x4254c0, 0x4de110 and 0x4ddf40. Named by what they do until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x4254c0 (cdecl; the animator): the holder at +0x24 sleeps (Holder_Sleep) and flag 0x4000 of the animator's flags (+0) is cleared. Returns 1.
u32 __cdecl Animator_Sleep(u8 *animator);

// 0x4de110 (thiscall on the object; out word, pops 4): Object_BackOfQueue; returns out.
u16 *__fastcall Object_BackOfQueueTo(u8 *object, void *, u16 *out);

// 0x4ddf40 (thiscall on the object): Object_QueuePosition of the person whose handle word is 0 (the stack word it passes is the object's pointer with its low word cleared (sic)).
s32 __fastcall Object_QueuePositionOfNone(u8 *object);

}  // extern "C"
