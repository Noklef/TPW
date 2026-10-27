// The sound manager [0x802bcc]: a handle (vtable 0x70a288, made by 0x6b5dc0; +4 the engine object it holds, 0x34 bytes with vtable
// 0x70a2a0) whose methods forward to the engine's slots. Reimplementation of 0x6b5ab0-0x6b5b80 and the deleting destructor 0x6b5bf0;
// the command post 0x6b5bd0 is the exe's. Names past Release / Query / Post (the slots soundset and soundplay call) are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b5ab0 (thiscall): the destructor: the vtable reset to 0x70a288; the engine (when there is one) destroyed through its slot 0
// (deleting: 1) and +4 cleared.
void __fastcall SoundManager_Dtor(u8 *manager);

// 0x6b5af0 (slot 0; thiscall (out), pops 4): the manager (unless null) destroyed through its own slot 3 (deleting: 1); *out = 0.
// Returns out.
u32 *__fastcall SoundManager_Release(u8 *manager, void *, u32 *out);

// 0x6b5b10 (slot 1; thiscall (out, a, b, c), pops 0x10): the engine's slot 2 (out, a, b, c). Returns out.
u32 *__fastcall SoundManager_Query(u8 *manager, void *, u32 *out, u32 a, u32 b, u32 c);

// 0x6b5b40 (slot 2; thiscall (owner, command), pops 8): the engine's slot 4 (owner, command), whose result is returned.
u32 __fastcall SoundManager_Post(u8 *manager, void *, u32 owner, u32 command);

// 0x6b5b80 (thiscall (a, b, c), pops 0xc; called by 0x51bc40): the engine's slot 3 (a, b, c), whose result is returned.
u32 __fastcall SoundManager_Call3(u8 *manager, void *, u32 a, u32 b, u32 c);

// 0x6b5b60 (thiscall (a, b), pops 8; called by 0x51c290 with a position as floats): the engine's slot 5 (a, b), whose result is
// returned.
u32 __fastcall SoundManager_Call5(u8 *manager, void *, u32 a, u32 b);

// 0x6b5ba0 (thiscall (out, position, direction, value), pops 0x10; called at 0x51c25d on the manager [0x802bcc]; name invented): SoundObject_SetPlacement on the
// engine (+4: the sound object) with the same arguments. Returns out.
u32 *__fastcall SoundManager_SetPlacement(u8 *manager, void *, u32 *out, const u32 *position, const u32 *direction, const u32 *value);

// 0x6b5bd0 (thiscall (out, functor), pops 8; called on the manager [0x802bcc] with stack functors such as 0x700b2c's; name invented):
// SoundChannel_Apply on the engine (+4: the sound object) with the same arguments (the functor's slot 0 (out, engine)). Returns out.
u32 *__fastcall SoundManager_Apply(u8 *manager, void *, u32 *out, u8 *functor);

// 0x6b5ad0 (slot 4; thiscall (out, a), pops 8): the engine's slot 1 (out, a). Returns out.
u32 *__fastcall SoundManager_Call1(u8 *manager, void *, u32 *out, u32 a);

// 0x6b5dc0 (stdcall (out, &manager, engine, a), pops 0x10; name invented): a new manager (Mem_Alloc(8): vtable 0x70a288, +4 = 0, then
// the engine) put in *manager (0 when the block fails, then *out = -1); its slot 4 (SoundManager_Call1, called directly: its vtable was
// just stored) (out, a). Returns out.
u32 *__stdcall SoundManager_Create(u32 *out, u8 **manager, u8 *engine, u32 a);

// 0x6b5bf0 (slot 3; thiscall (flags), pops 4): SoundManager_Dtor, then the manager freed for flags bit 0. Returns the manager.
u8 *__fastcall SoundManager_DeletingDtor(u8 *manager, void *, u32 flags);

}  // extern "C"
