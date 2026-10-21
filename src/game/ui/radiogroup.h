// The GUI library's window subclass with the vtable 0x705ac0 (built by 0x66a1e8 over Window_Ctor: the handlers 0x66a31c, the dword +0x138
// -1): it keeps the id of one selected child at +0x138 and sets the state (Window705a50_SetState) of that child, which it takes to be a
// 0x705a50 window. What it is is unconfirmed (a group of radio-like buttons). Reimplementation of 0x66a1b0, 0x66a295 and 0x66a31c.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a1e8 (thiscall (parent, flags, id, left, top, right, bottom: words), pops 28; kind 8 of the factory 0x65dd7b; name unconfirmed):
// Window_Ctor, then the vtable 0x705ac0, none selected (+0x138 -1), both handlers 0x66a31c. Returns the group.
u8 *__fastcall RadioGroup_Ctor(u8 *group, void *, u8 *parent, u32 flags, u32 id, u32 left, u32 top, u32 right, u32 bottom);

// 0x66a1b0 (thiscall (state: a byte), pops 4; name unconfirmed): the child whose id is the selected one (+0x138; Window_FindChild), when
// there is one, gets Window705a50_SetState(state, 1).
void __fastcall RadioGroup_SetSelectedState(u8 *group, void *, u8 state);

// 0x66a295 (thiscall (id), pops 4; name unconfirmed): -1 (AX) when `id` is the selected one already. For -1: the selected child's state
// 0 (RadioGroup_SetSelectedState) and none selected, -1. Otherwise, without a child of that id, -1; with one, the selected child's state
// 0, `id` selected, its state 1, and the id selected before (AX).
u16 __fastcall RadioGroup_Select(u8 *group, void *, u32 id);

// 0x66a31c (cdecl (group, message, a, b); the group's handler +0x114 / +0x118; name unconfirmed): 0x15: with the flags +0x48 bit 4 set
// and none selected (+0x138 -1 as a dword), the first child (+0x5c), when there is one and its slot 3 (the kind) answers 2, selected
// (RadioGroup_Select of its id +0x8c). 0x100 (a: a child's id; b: 1 pressed): only with a child of id a (Window_FindChild) and a group:
// for b 1, the parent (+0x60) posted 0x101 (the selected id, 0) when one is selected, a selected and the parent posted 0x101 (a, 1); else
// with bit 4 of +0x48 clear, none selected and the parent posted 0x101 (a, 0); with it set, a selected (again) and the child's state 1
// (Window705a50_SetState(1, 1)). Then, for every message, Window_DefaultProc (group, message, a, b), whose answer it returns.
u32 __cdecl RadioGroup_Proc(u8 *group, u32 message, u32 a, u32 b);
}
