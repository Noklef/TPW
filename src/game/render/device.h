// The renderer's device classes: reimplementation of the constructors of the device interface (vtable 0x701260, all of its slots but the
// deleting destructor pure) and of the Direct3D device (vtable 0x701378, whose virtual base's table 0x701330 holds D3dProbe_SetFlags; the
// object [0x879ac0] made by Global879ac0_Create) at 0x5651c0 and 0x5667d0. The software rasteriser's device ([0x879acc], virtual base table
// 0x7013c8) derives from the interface too; its methods are in render/rasteriser.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5651c0 (thiscall): vtable 0x701260, +4 = +8 = +0x18 = +0x1c = +0x20 = +0x24 = 0, +0x14 = -1; and
// 0x879ab4 = 1 when the display adapter record at [0x879a24] has vendor +0x428 = 0x10de and the driver
// kind 0x8bd524 is 1. Returns the object.
u8 *__fastcall Class701260_Ctor(u8 *object);

// 0x5667d0 (thiscall; most derived flag, pops 4): with the flag set the vtable 0x701378 is stored and the base class constructor (Class701260_Ctor) runs on the part at +0x2c;
// then the virtual base (offset from the word at +4 of the vtable) gets the vtable 0x701330, the word before it the distance to that base less 0x2c, and the fields +8, +0x18, +4 are
// cleared, +0x10 is 2, +0x14 is 1, +0x20 is -1 and the word at the virtual base + 0xc is 0x3000. Returns the object.
u8 *__fastcall Class701378_Ctor(u8 *object, void *, u32 mostDerived);
}
