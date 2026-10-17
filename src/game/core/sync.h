// The game's wrappers of the Win32 synchronisation objects (0x5c8b90-0x5c8c2b, thiscall): a critical section held in the object itself (0x18 bytes)
// and an auto-reset event whose handle is the object's first dword. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5c8b90: InitializeCriticalSection (0x6fd144) on the object. Returns the object.
u8 *__fastcall CritSec_Ctor(u8 *section);

// 0x5c8ba0: DeleteCriticalSection (0x6fd0d0) on the object.
void __fastcall CritSec_Dtor(u8 *section);

// 0x5c8bb0 / 0x5c8bc0: EnterCriticalSection / LeaveCriticalSection (0x6fd140 / 0x6fd130) on the object. Return 0.
u32 __fastcall CritSec_Enter(u8 *section);
u32 __fastcall CritSec_Leave(u8 *section);

// 0x5c8bd0: the handle (+0) of CreateEventA (0x6fd0d4) with (0, 0, 0, 0): an auto-reset event, not signalled, unnamed. Returns the object.
u32 *__fastcall SyncEvent_Ctor(u32 *event);

// 0x5c8bf0 (timeout; pops 4): WaitForSingleObject (0x6fd128) on the handle, INFINITE for a negative timeout; whether it answered WAIT_OBJECT_0
// (only AL is set: the rest of EAX is the wait's answer).
bool __fastcall SyncEvent_Wait(const u32 *event, void *, s32 timeout);

// 0x5c8c20: SetEvent (0x6fd124) on the handle. Returns 0.
u32 __fastcall SyncEvent_Set(const u32 *event);
}
