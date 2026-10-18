// The mouse modes (what a click in the park does: CMMPlaceStaff, CMMOnline, ...): a base class
// (vtable 0x6fe9b0, 12 slots: 0 the deleting destructor) and its derived classes, each with its
// own vtable. Reimplementation of their constructors, destructors and empty methods in
// 0x46c450-0x46d000, and of the statics and frame step before them (0x46c070-0x46c100). All __thiscall (self in ECX), written as __fastcall with an unused EDX.
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

// 0x46c070 / 0x46c090: static initialisers of two floats nothing reads: 0x7b05a0 = 1 and
// 0x7b05a4 = 1 / 2^23 (as camera's). 0x46c0b0: the pointer state's tile (0x7b05cc, u16) = 1
// and 0x7b05d0 = 0.
void __cdecl MouseModes_InitUnusedOne();
void __cdecl MouseModes_InitUnusedEpsilon();
void __cdecl MouseModes_InitTile();

// 0x45d560 (cdecl; position; name unconfirmed): the pointer state's tile (0x7b05cc) from a position, when there is a terrain map
// (0x7a0854) with a grid (+0x6c): x = __ftol((position[0] - grid +8) / grid +0x10), z the same from position[2] with +0xc / +0x14;
// each then clamped to 0 (signed), below the grid's width / depth (+0x18 / +0x1c, unsigned: else that - 1), to 0 again and below
// 0x7a0314 / 0x7a0374 (signed); the tile is (z << 7) + x + 1 when Map_InBounds, else 0.
void __cdecl MouseState_SetTileFromPosition(const float *position);

// 0x46c100: a new frame for the pointer state (0x7b05a8): its +0xc count stepped and its
// picked thing (+0) cleared.
void __fastcall MouseState_NewFrame(u8 *self);

// 0x46c0d0 (thiscall; fps, pops 4; name unconfirmed): "FPS: %4g\n" (0x74d85c) of fps (as a double) printed (sprintf) into the
// pointer state's +0x10 and that text given to the object at 0x8bba50 (Obj_Set14_18_10(0, 0, text)). Returns what that does.
u32 __fastcall MouseState_ShowFps(u8 *self, void *, float fps);

// 0x46c110 (thiscall; name unconfirmed): the pointer state's picks resolved. +4 becomes its picked thing (+0) when that is of kind (+2) 1, 4,
// 5, 6, 7, 8 or 0x12, else 0; +8 the thing (the dword of 0x7cfb90's 20-byte slot) of the handle Map_ObjectOrEntranceAt gives for its tile
// (+0x24: column (tile - 1) & 0x7f, row (tile - 1) >> 7, unsigned), else 0; and a picked thing of 0 becomes that +8.
void __fastcall MouseState_ResolvePick(u8 *self);

// 0x46c450 / 0x46c650: empty methods (taking 4 and 2 arguments).
void __fastcall MouseMode_Nop4(void *self, void *edx, u32, u32, u32, u32);
void __fastcall MouseMode_Nop2(void *self, void *edx, u32, u32);

// 0x46c480 (thiscall, 4 arguments unused; slot 6 of CMMPlaceStaff (0x6fea40) and the 0x6fea70 mode; name unconfirmed): with a sprite
// script slot at +8, the script moves to the highlight point (Highlight_Point) 5 up (y - -5) with r15 4. The cell under the pointer
// ([0x7cf83c] + 0x294 + the tile (0x7b05cc) * 0x44, its first word taken as a handle) gets a square face (DynFaces_AddSquare: x - 5, 1,
// z - 5, 0, tag 1, 0, 1, 0, 0, 0, 0) when MapHandle_IsTypeBDOr10Or12Or19, MapCell_HasFlags(0x40) or MapHandle_ParentIsType19, or else
// when it is not MapCell_IsType0, MapCell_IsType1 or MapCell_IsType3Or9.
void __fastcall MMPlaceStaff_Hover(u8 *self, void *edx, u32, u32, u32, u32);

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
