// A holder's teardown, the mouse button and move handlers and a COM-holding object's release: reimplementation of 0x45bc60, 0x46c210, 0x46c2a0 and 0x47e3e0. Named by what they do
// until the classes are known.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x45bc60 (cdecl; the holder): with flag 1 the holder's part sleeps (Holder_Sleep); the count at +0x34 of the owner (the holder at +8, else itself) is lowered; the part at +0x1c
// (adjusted by the virtual base offset in the word at +4 of its vtable) is deleted through its slot 0 (with 1) and the pointer cleared.
void __cdecl Holder_Destroy(u8 *holder);

// 0x46c210 (cdecl; event 0xf0 / 0xf1, state, a, b): through the mouse object at [0x7b05d8] the left (0xf0, slots 1 and 2 at +4 and +8) or right (0xf1, +0xc and +0x10) button goes
// down (state 1) or up with (a, b), and the flag [0x7b05e4] / [0x7b05f0] is set to 1 / 0. Returns 1.
u32 __cdecl Mouse_HandleButton(u32 event, u32 state, u32 a, u32 b);

// 0x46c2a0 (stdcall; x, y, pops 8): when the position differs from the last one ([0x7b05d4], [0x7b05e0]) the mouse object (slot 6 at +0x18) gets (x, y, [0x7b05e4], [0x7b05f0]);
// then slot 7 (+0x1c) with (x, y) while the left button flag is set, else slot 8 (+0x20) while the right one is; both last positions are then set to x (sic).
void __stdcall Mouse_Moved(s32 x, s32 y);

// 0x47e3e0 (thiscall): the vtable 0x6fef78, the words at +0xa8 and +0xa4 cleared, the COM objects at +0xb0 and +0xac released (slot 2 at +8) and cleared, the vtable 0x6fef90.
void __fastcall Class6fef78_Release(u8 *object);

}  // extern "C"
