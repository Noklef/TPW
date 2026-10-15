// The mouse modes (what a click in the park does: CMMPlaceStaff, CMMOnline, ...): a base class
// (vtable 0x6fe9b0, 12 slots: 0 the deleting destructor) and its derived classes, each with its
// own vtable. Reimplementation of their constructors, destructors and empty methods in
// 0x46c450-0x46d000. All __thiscall (self in ECX), written as __fastcall with an unused EDX.
//
// Their layouts: +0 the vtable, then by class
//   0x6fe9e0 (the tile modes): +4 1, +8 the argument, +0xc 0 or 1, +0x10 (u16) a kind;
//   0x6fea10 (the base of CMMOnline, 0x6fe980): +4 1;
//   0x6fea40 (CMMPlaceStaff): +4, +8 0, +0xc, +0x10 / +0x11 (bytes), +0x14 the arguments,
//     +0x18 (byte) 0;
//   0x6fea70 (its OnLeftClick says "Can't put staff here"): +4, +8 0, +0xc, +0x10 (byte);
//   0x6feaa0 and 0x6fead0: +4 0.
#pragma once

#include "../../../re/types.h"

struct MouseModeEnv {
    void(__cdecl *free)(void *block);  // Mem_Free (0x67acb0)
};
// The deleting destructors' frees go to env (nullptr: the game's).
void MouseModes_SetEnv(const MouseModeEnv *env);

extern "C" {

// 0x46c450 / 0x46c650: empty methods (taking 4 and 2 arguments).
void __fastcall MouseMode_Nop4(void *self, void *edx, u32, u32, u32, u32);
void __fastcall MouseMode_Nop2(void *self, void *edx, u32, u32);

// 0x46ced0: the base destructor: the base vtable.
void __fastcall MouseMode_Dtor(u8 *self);

// 0x46c460 / 0x46c710: the base's and the derived classes' deleting destructors: the base
// vtable, then freed when flags has bit 0. Return self.
u8 *__fastcall MouseMode_DeletingDtor(u8 *self, void *edx, u32 flags);
u8 *__fastcall MouseMode_DerivedDeletingDtor(u8 *self, void *edx, u32 flags);

// 0x46c580 / 0x46c5a0: a tile mode for arg (+0xc 0) / for arg and kind (+0xc 1).
u8 *__fastcall MouseModeTile_Ctor(u8 *self, void *edx, u32 arg);
u8 *__fastcall MouseModeTile_CtorKind(u8 *self, void *edx, u32 arg, u16 kind);

// 0x46c6a0: CMMOnline's base.
u8 *__fastcall MouseModeOnlineBase_Ctor(u8 *self);

// 0x46c6d0: CMMPlaceStaff (a, b, c, d).
u8 *__fastcall MMPlaceStaff_Ctor(u8 *self, void *edx, u32 a, u8 b, u8 c, u32 d);

// 0x46cbc0: the 0x6fea70 mode (a, b).
u8 *__fastcall MMStaffDrop_Ctor(u8 *self, void *edx, u32 a, u8 b);

// 0x46cfc0 / 0x46cff0: the 0x6feaa0 / 0x6fead0 modes.
u8 *__fastcall MouseMode6feaa0_Ctor(u8 *self);
u8 *__fastcall MouseMode6fead0_Ctor(u8 *self);
}
