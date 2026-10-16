// The texture cache's surface classes: the virtual methods behind the adjustor thunks at 0x58c2b0
// and 0x58c900-0x58c910, reimplementation of 0x58c2c0, 0x58c2e0, 0x58c330, 0x58c8a0 and 0x58c8b0.
//
// Each thunk subtracts [ECX - 4] and jumps here, so ECX is a sub-object of the surface (the
// cache's surfaces derive virtually from the base at 0x702198, see texcache.h). The class at
// 0x58c2c0 keeps its vbtable pointer at ECX - 0x3c, and the one at 0x58c8a0 at ECX - 0x10; the
// vbtable's second dword (+4) is the offset from ECX to the virtual base's fields.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x58c2c0 (thiscall on the sub-object): the dword at ECX - 0x38.
u32 __fastcall TexSurfaceA_Field38(const u8 *sub);

// 0x58c2e0: the virtual base's two dwords (at ECX + off - 0x34 and - 0x30, off = vbtable[1] of
// the vbtable at ECX - 0x3c) multiplied: an area; 32 bits.
u32 __fastcall TexSurfaceA_Area(const u8 *sub);

// 0x58c330: the dword at ECX - 0x10.
u32 __fastcall TexSurfaceA_Field10(const u8 *sub);

// 0x58c8a0 (the other class): the dword at ECX - 0xc.
u32 __fastcall TexSurfaceB_FieldC(const u8 *sub);

// 0x58c8b0: the same area from ECX + off - 8 and - 4, off = vbtable[1] of the vbtable at ECX - 0x10.
u32 __fastcall TexSurfaceB_Area(const u8 *sub);
}
