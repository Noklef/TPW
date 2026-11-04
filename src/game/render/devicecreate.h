// The device objects made and destroyed for the driver kinds: reimplementation of 0x572c40 (kind 1, Direct3D, [0x879da8]) and 0x573960, 0x573a30, 0x573b30, 0x573b90 (kind 4, the software renderer, [0x879dac]). Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// (Test seam: the fncmp driver's table replaces the calls by address.)
void DeviceCreate_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x572c40 (cdecl (kind)): when [0x879da8] is clear, a new object of 0x22c bytes (the game's heap), its virtual base's tables set and a GUID copied from 0x707550, opened (D3dSetup_Open: flags 0x10 for kind 1, else 0).
// Opened: [0x8bd52c] = its virtual base, 1. Failing: freed, cleared, the error (3, 0xb0754005), 0. Not allocated: the error (3, 0xb0754000), 0. There already: 0.
u32 __cdecl D3dSetup_Create(u32 kind);

// 0x573960 (cdecl (kind)): when [0x879dac] is clear, a new object of 0x18 bytes, opened (SoftSetup_Open). Opened: [0x8bd52c] = its virtual base, 1. Failing: freed but not cleared (sic), the error (3, 0xb0754010), 0.
// Not allocated: the error (3, 0xb0754000), 0. There already: 0.
u32 __cdecl SoftSetup_Create(u32 kind);

// 0x573a30 (cdecl): as SoftSetup_Create, its surface the DirectDraw object's +0x14 asked for the raster interface, square of side [0x8bd4ac], no Z buffer; the raster globals 0xfbe014- set; [0x8bd52c] untouched. 1.
u32 __cdecl SoftSetup_CreateSquare();

// 0x573b30 (cdecl): when [0x879dac] is set, its blocks +4 and +8 freed (when set; tracked) and cleared, the C library's 0x679270 called, the object freed, it and [0x8bd52c] cleared, 1. Else 0.
u32 __cdecl SoftSetup_Destroy();

// 0x573b90 (thiscall (kind), pops 4): logged when [0x8bd508] has 0x2000000. The raster interface asked of the DirectDraw object's +0x14; the raster set up at [0x8bcbd0] by [0x8bcbd4], a Z buffer into +4 when the kind is
// not 0; +8 the raster, +0xc its bytes; the raster globals 0xfbe014-0xfbe034 set. 1.
u32 __fastcall SoftSetup_Open(u8 *self, void *, u32 kind);
}
