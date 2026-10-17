// The screen's two surfaces as a surface source (vtable 0x6fef78, built at 0x47e775; its
// abstract base 0x6fef90 has only the destructor and four pure virtuals): reimplementation of
// 0x47e2f0, 0x47e390 and 0x47e570. Two Surfaces at +4 and +0x54 wrap, on first use, the
// display's (0x879a24) +0x14 and +0x10 DirectDraw surfaces (the virtuals 0x47e510 / 0x47e4b0;
// done flags +0xa4 / +0xa8, the interfaces +0xb0 / +0xac).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x47e2f0: both Surfaces' colour keys, attached and surface pointers cleared and their infos
// constructed (SurfaceInfo_Ctor), nothing wrapped yet; returns source.
u8 *__fastcall ScreenSurfaces_Ctor(u8 *source);

// 0x47e390: the base's destructor (its vtable back).
void __fastcall SurfaceSource_Dtor(u8 *source);

// 0x47e570 (virtual): out = {0, 0, screen width - 1, screen height - 1} (0x7a1968, 0x7a1930);
// returns out.
s32 *__fastcall ScreenSurfaces_Bounds(const u8 *source, void *, s32 *out);
// 0x47e3e0 (thiscall): the vtable 0x6fef78, the words at +0xa8 and +0xa4 cleared, the COM objects at +0xb0 and +0xac released (slot 2 at +8) and cleared, the vtable 0x6fef90.
void __fastcall ScreenSurfaces_Dtor(u8 *object);

// 0x47e3c0 (thiscall; flags, pops 4): ScreenSurfaces_Dtor, then the object freed when bit 0 of the flags is set. Returns the object.
u8 *__fastcall ScreenSurfaces_ScalarDtor(u8 *object, void *, u32 flags);
}
