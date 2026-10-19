// The GUI library's window subclass with the vtable 0x705ac0 (built by 0x66a1e8 over Window_Ctor: the handlers 0x66a31c, the dword +0x138
// -1): it keeps the id of one selected child at +0x138 and sets the state (Window705a50_SetState) of that child, which it takes to be a
// 0x705a50 window. What it is is unconfirmed (a group of radio-like buttons). Reimplementation of 0x66a1b0 and 0x66a295.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x66a1b0 (thiscall (state: a byte), pops 4; name unconfirmed): the child whose id is the selected one (+0x138; Window_FindChild), when
// there is one, gets Window705a50_SetState(state, 1).
void __fastcall RadioGroup_SetSelectedState(u8 *group, void *, u8 state);

// 0x66a295 (thiscall (id), pops 4; name unconfirmed): -1 (AX) when `id` is the selected one already. For -1: the selected child's state
// 0 (RadioGroup_SetSelectedState) and none selected, -1. Otherwise, without a child of that id, -1; with one, the selected child's state
// 0, `id` selected, its state 1, and the id selected before (AX).
u16 __fastcall RadioGroup_Select(u8 *group, void *, u32 id);
}
