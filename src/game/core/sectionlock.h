// A critical section in its own block, and a guard over one: the wrappers the input code's wait thread uses. Reimplementation of 0x633900-0x633991. Names invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x633900 (thiscall): a block of 0x18 bytes (Mem_Alloc, not checked) kept at *self and initialised as a critical section. Returns self.
u32 *__fastcall OwnedSection_Ctor(u32 *self);
// 0x633920 (thiscall): the section at *self deleted and its block freed.
void __fastcall OwnedSection_Dtor(u32 *self);

// 0x633940 (thiscall): the guard's destructor: Leave.
void __fastcall SectionGuard_Dtor(u32 *self);
// 0x633950 (thiscall): the section at *self entered; +4 = 1.
void __fastcall SectionGuard_Enter(u32 *self);
// 0x633970 (thiscall): held (+4) and not +8: the section left, +4 = 0.
void __fastcall SectionGuard_Leave(u32 *self);

}  // extern "C"
