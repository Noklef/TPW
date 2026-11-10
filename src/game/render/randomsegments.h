// An object of two random segments (vtable 0x702cb0; its constructor is called at 0x5e3dfb and 0x5e3e9d): reimplementation of its
// constructor 0x5dfcd0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5dfcd0 (thiscall; an SEH frame; name invented, unconfirmed): Class702d48_Ctor, the vtable 0x702cb0; +8 a new block of 0x10 bytes (Mem_Alloc;
// none: 0): the vtable 0x702cac and three zero dwords; +0xc to +0x24 zero; +0x28 and +0x34 the vector at +8 of [0xf83778], +0x40 and +0x4c the
// one at its +0x14; the centre +0x58 (500, 75, 500), the size +0x64 (400, 50, 400), +0x70 1.0, +0x74 100.0, +0xa8 and +0xac 2.0, +0xb0 50.0. Two
// segments (start +0x78, end +0x84, direction +0x90; start +0xb4, end +0xc0, direction +0xcc), each point's axes centre + size * (draw *
// 2^-30 - 0.5) (x87 at 24 bits) with draw the low 30 bits of the next number of the generator at [0xf82884] +0x34 (as World_Rand's); the direction
// end - start, normalised, (1, 0, 0) when it is zero (fcomp's C3, so with NaN too).
u8 *__fastcall RandomSegments_Ctor(u8 *object);
}

#include "../core/gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[1], Class702cac_vtable, 0x00702cac, 0x4);
GAME_DATA(void *[20], Class702cb0_vtable, 0x00702cb0, 0x50);
