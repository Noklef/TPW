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
    void(__cdecl *free)(void *block);                  // Mem_Free (0x67acb0)
    s32(__cdecl *atExit)(void(__cdecl *fn)());        // crt_atexit (0x67a7d0)
    void(__cdecl *clickTile)(s32 x, s32 y, u32 flags);   // 0x5234d0 (unported; as MapClickEnv's click5234d0)
    void(__cdecl *f52f200)(u32 a, u32 b);              // GameMode_Set (0x52f200, agent A's)
};
// The deleting destructors' frees, MouseMode_Set's atexit and MouseModeTile_Assign's two calls go to env (nullptr: the game's, GameMode_Set
// the port).
void MouseModes_SetEnv(const MouseModeEnv *env);

extern "C" {

// 0x40bed0 (cdecl; name unconfirmed; also inlined in 0x481a10 and 0x485b70): the one-byte object at [0x7b05e8] made the first time
// (Mem_Alloc(1), its byte left as it is) and constructed (MouseModeHolder_Ctor, inlined there). Its allocation failing: [0x7b05e8] stays
// 0 and nothing else is done. Returns [0x7b05e8] (EAX; the game's callers ignore it).
u8 *__cdecl MouseModes_EnsureHolder();

// 0x46d550 (thiscall; name unconfirmed): the one-byte mode holder's constructor: the mouse mode set to a new CMMOnline base
// (Mem_Alloc(8), MouseModeOnlineBase_Ctor; 0 when that failed; MouseMode_Set), 0x7b05e0 and 0x7b05d4 = -1. Returns the holder.
u8 *__fastcall MouseModeHolder_Ctor(u8 *holder);

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

// 0x46ce80 (name unconfirmed; its vtable's slot 10 prints "Assigning CMMOnline"): CMMOnline (vtable 0x6fe980): +4 0, then the cursor
// 0 (Obj7b51f0_SetCursor). Returns self.
u8 *__fastcall MMOnline_Ctor(u8 *self);

// 0x46c350 (stdcall; mode, pops 4; name unconfirmed): the mouse mode switched to `mode`. The current one ([0x7b05d8]) left (slot 0x2c),
// then (re-read) deleted (slot 0, 1). The mode setting at 0xfb3b7c is a function-local static (the guard 0xfb3518 & 1: Setting_ReadMode,
// atexit(0x5d1750)); when it is 1, a mode given whose slot 0x24 answers 1 is deleted (slot 0, 1) and replaced by a new CMMOnline (8 bytes
// from the game's heap, MMOnline_Ctor; 0 when that failed). The mode (or 0) is stored at 0x7b05d8 and, if any, assigned (slot 0x28).
void __stdcall MouseMode_Set(u8 *mode);

// 0x481a10 (cdecl; name unconfirmed): the window [0x7c24c8] closed (Window7c24c8_Close), then a new 0x6fead0 mode (Mem_Alloc(8),
// MouseMode6fead0_Ctor; 0 when that failed) made, the holder [0x7b05e8] made when missing (MouseModes_EnsureHolder) and the mode set
// (MouseMode_Set).
void __cdecl MouseMode_Enter6fead0();

// 0x40c5e0 (cdecl; name invented, unconfirmed): a new tile mode (Mem_Alloc(0x14), MouseModeTile_Ctor (0x3a); 0 when that failed) made,
// the holder [0x7b05e8] made when missing (MouseModes_EnsureHolder, inlined) and the mode set (MouseMode_Set). Returns 1.
u32 __cdecl MouseMode_EnterTile3a();

// Slot 10 of the modes (Assign, by CMMOnline's debug print; names unconfirmed). 0x46d590 (the 0x6fead0 mode): the cursor 0x13.
void __fastcall MouseMode6fead0_Assign(u8 *self);
// 0x46c730 (CMMPlaceStaff): the cursor 9, then for the kind +0xc 4, 5, 6, 7 or 8 a sprite script (SpriteScript_Start code 0x74ed98, folder
// 6, 5, 4, 7 or 8, sprite the byte +0x11, the rest 0) goes to +8, the one there before (if any) killed after it starts. Other kinds: only
// the cursor (and a debug print, left out).
void __fastcall MMPlaceStaff_Assign(u8 *self);
// 0x46cbf0 (the 0x6fea70 mode): the same with folder +0xc and sprite the byte +0x10.
void __fastcall MMStaffDrop_Assign(u8 *self);
// 0x46d5a0 (the tile modes, vtable 0x6fe9e0): with the flag +0xc the definition of the id +0x10 (ObjectDefs_Find on 0x7890a0); the
// env's clickTile on the tile [0x7b05cc] (a word) - 1: x its low 7 bits, y the rest (unsigned: tile 0 gives 0x7f, 0x1ffffff), flags 0;
// the env's f52f200 (+8, 1). Then with the flag (re-read): Global8186e0_SetRecorded (&+0x10, the definition's +0x9c when its +0x288 is
// 1, else 0); and when +8 is 4 or 0x3b too, Global81b0cc_SetRecorded (its +0x40 not 0) and Global81ecac_SetRecorded (its +0x9c). (No
// definition found: the original reads through null.)
void __fastcall MouseModeTile_Assign(u8 *self);

// 0x46cf10 (cdecl; name invented; in CMMOnline's unit, before its destructor; an SEH frame for the constructor): the online chat
// [0x7cddec], made when there is none: 0xf0c bytes from the game's heap (GameHeap_Alloc) constructed by OnlineChat_Ctor (online/chat), or
// 0 when that failed; stored and returned.
u8 *__cdecl OnlineChat_Instance();

// 0x497bc0 (cdecl; called from 0x498430-0x498500; name unconfirmed): the holder [0x7b05e8] made when missing (MouseModes_EnsureHolder);
// then unless the current mode ([0x7b05d8], read after that) answers 8 through its slot 9 (none: 0), a new 0x6feaa0 mode (Mem_Alloc(8),
// MouseMode6feaa0_Ctor; 0 when that failed) made, the holder made when missing again and the mode set (MouseMode_Set).
void __cdecl MouseMode_Ensure6feaa0();

// 0x497d20 and 0x497e10 (cdecl; names invented): the focus cell updated (FocusCell_Update(0)), a tile mode (Mem_Alloc(0x14),
// MouseModeTile_Ctor with 0x15 or 0x16; 0 when that failed) made, the holder [0x7b05e8] made when missing (MouseModes_EnsureHolder), the
// mode set (MouseMode_Set), then Global81ecac_SetRecorded(3).
void __cdecl MouseMode_EnterTile15();
void __cdecl MouseMode_EnterTile16();

// 0x4a2ac0 (cdecl (leave); name invented): nothing in the game object's mode 4 ([0x7cf83c] +0x1da738) or with the word 0x7cf4f4 set
// (Word7cf4f4_NonZero); else message 6 (leave, 0) to the window [0x7cb2ac] and (1 - leave, 0) to [0x7cb2b0] (Window_SendMessage); the
// mode setting 0xfb3b7c read once (its guard 0xfb34c1 & 1: Setting_ReadMode, atexit(0x5d1750) through the env); when it is 1 the chat
// left (Window7cc8bc_LeaveChat) or, leave 0, entered (Window7cc8bc_EnterChat); then the focus restored (Focus_Restore).
void __cdecl ChatButtons_Select(u32 leave);
// 0x481470 (stdcall (on), pops 4; name invented): ChatButtons_Select(on == 0).
void __stdcall ChatButtons_SetChat(u32 on);
}
