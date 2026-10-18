// The game object at 0x7b9940 (built by 0x4855d0): a renderable with a virtual base (its vtable pointer +0, the base's offset at the
// vtable's +4) that the renderer holds in its tag 0x20 list while it is shown (0x48a410 adds it, 0x48a520 and 0x48a3d0 take it out).
// Reimplementation of 0x4855d0 and 0x48a3d0. The names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x48a3d0 (no arguments; also reached through the jump 0x481410, called with ECX 0x7b51f0 it ignores): the GUI manager's slot 4
// (GuiManager_CallSlot4), then the object's virtual base (0x7b9940 + the offset at its vtable's +4; the original's test of 0x7b9940
// against 0 always passes) taken out of the renderer's ([0x87a6d0]) tag 0x20 list (Renderer_RemoveFromSlot); returns what that returns.
u32 __cdecl Panel7b9940_Detach();

// 0x4855d0 (thiscall (complete), pops 4): the constructor. complete (the most derived object) sets the vbtables +0 0x6fdd00 and +0xc
// 0x6fdcf8 and the vtable +8 0x6fdce8, then through +0xc's vbtable the vtable 0x6fdcd8 and its displacement (offset - 0x60) and
// +0x5c 0. Then through +0's vbtable (each offset read again for every use): the vtable 0x6fdcc8 at the first base and its
// displacement (offset - 8) before it, and in the second base: +0x50 2, the scene flags touched (SceneFlags_Touch on 0x8bd508),
// +0x2c and +0x30 0, +0x34 1.0f, touched again, +4..+0x10 1.0f and +0x54 -1. Returns the object.
u8 *__fastcall Panel7b9940_Ctor(u8 *self, void *edx, u32 complete);

}
