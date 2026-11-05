// A thread waiting on a list of event handles and running commands sent to it (the objects whose handles it waits on are in the second list): reimplementation of 0x632470, 0x632750-0x632960, 0x632ef0.
// The object: +4 the command (1 add, 2 remove, 3 quit), +8 its argument, +0xc its result, +0x10 quit, +0x18 the command event, +0x1c the answer event, +0x24 vector<handle> (the first the command event),
// +0x34 vector<object>. Names invented.
#pragma once

#include "../../../re/types.h"
#include "addrcalls.h"

extern "C" {

// 0x632460 (stdcall (self), pops 4): the thread's procedure: the loop.
u32 __stdcall WaitThread_Proc(u8 *self);
// 0x6324e0 (thiscall): until +0x10 is set: a wait on all the handles (+0x28, none: a count of 0). A failed wait keeps only the first handle and no objects; handle 0 runs
// the command; another's object (+0x38, one less) has its slot 0xc called, and it and its handle change places with the next (so that none starves). Returns 0.
u32 __fastcall WaitThread_Loop(u8 *self);

// 0x632470 (thiscall): the command run on the thread, its result at +0xc (another command: 0), the answer event set.
void __fastcall WaitThread_RunCommand(u8 *self);
// 0x632750 (thiscall; result out, pops 4): the two events (CreateEventA) when missing, the command event first in the handles. A failure: the last error. Returns out.
u32 *__fastcall WaitThread_OpenEvents(u8 *self, void *, u32 *out);
// 0x6327e0 (thiscall): the events closed; with an answer event there, the first handle erased (without one nothing more, sic).
void __fastcall WaitThread_CloseEvents(u8 *self);
// 0x632840 (thiscall; result out, pops 4): the command event set and the answer waited for; no thread (+4 = 0): -1; returns out with the thread's result, or the last error.
u32 *__fastcall WaitThread_Send(u8 *self, void *, u32 *out);
// 0x6328b0 / 0x6328d0 (thiscall; result out, object, pops 8): command 1 / 2 on the object, sent.
u32 *__fastcall WaitThread_Add(u8 *self, void *, u32 *out, u32 object);
u32 *__fastcall WaitThread_Remove(u8 *self, void *, u32 *out, u32 object);
// 0x6328f0 (thiscall; result out, pops 4): command 1: the object at the end of the objects, its handle (+8) at the end of the handles. No object: E_POINTER.
u32 *__fastcall WaitThread_DoAdd(u8 *self, void *, u32 *out);
// 0x632960 (thiscall; result out, pops 4): command 2: the object and its handle (one on, past the command event) erased. Not in the list: 0xcffe0101; no object: E_POINTER; the index past the handles: the
// vector's range error 0x632a40 (does not come back in the game).
u32 *__fastcall WaitThread_DoRemove(u8 *self, void *, u32 *out);
// 0x632ef0 (thiscall on a vector<u32>; position, count, pointer to the value, pops 12): ImeVector_InsertN's code again, another copy of the template.
void __fastcall WaitThread_InsertObjects(u8 *vector, void *, u32 *position, u32 count, const u32 *value);

}  // extern "C"

void WaitThread_SetCalls(const AddrCall *table, u32 count);
