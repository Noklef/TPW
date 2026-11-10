// The Direct3D setup: its opening, closing, clearing and format picks. Reimplementation of 0x572d50, 0x572f70, 0x573060-0x573720, 0x573740. All names invented, unconfirmed.
// The object: +4 the IDirect3D3, +8 the device, +0xc the viewport (render/d3dviewport), +0x10 the material, +0x14 its handle, +0x18 the device's GUID, +0x28, +0x124 the device's caps,
// +0x220 flags (0x10: a depth buffer), a virtual base at +0x228 (the offset in the table at +0). "Kept" as in render/d3dresult.h.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x573080 (thiscall; flags, pops 4), called at 0x572cf9 and 0x57372d: a driver of the adapter [0x879a24] (+0x428 0x121a, +0x42c 1) refused. QueryInterface on its DirectDraw object (+8) for the Direct3D at +4 (0x707520). Flag 0x10: EnumZBufferFormats
// (slot 0x28, D3dSetup_OnZFormat, 0x20 bytes with +4 = 0x400); no format of size 0x20 fails; a surface of the screen's size 0x8bcbd0, 0x8bcbd4 made (slot 0x18, description 0x7c, flags 0x1007, the format at +0x48, caps 0x24000) and attached (Obj_AddAttached),
// +0x220 bit 0x10 set; no flag 0x10: that bit cleared. CreateDevice (slot 0x20, member +0x14 of the adapter), GetCaps (0xc, sizes 0xfc, zeroed), the viewport (D3dSetup_CreateViewport), EnumTextureFormats (0x20, D3dSetup_OnFormat, the object). No format so
// far (0x8bd598 of 0): stop. CreateMaterial (0x14), SetMaterial (0xc, 0x50 bytes: five 1.0s from +0x10, +0x4c 1), GetHandle (0x14, into +0x14), SetBackground (viewport 0x20). The virtual base's slot 0: 0 stops. Every result kept and any but 0 stops. Done: render states 7, 0xe
// (and 0x17 = 4) by bit 0x10 of +0x220, 1. Stopped: the debug flag 0x2000000's texts, the attached surface let go (Obj_DeleteAttached) and the four released and cleared (material, viewport, device, Direct3D), 0.
// The log texts: flag 0x1000000 through EngineDebug_Log, 0x2000000 through EngineDebug_LogFallback.
u32 __fastcall D3dSetup_Open(u8 *self, void *, u32 flags);
// 0x5736a0 (thiscall on the virtual base): the attached surface let go, the four released and cleared, 1.
u32 __fastcall D3dSetup_Close(u8 *base);
// 0x573720 (thiscall on the virtual base): D3dSetup_Open with bit 0x10 of +0x220.
u32 __fastcall D3dSetup_Reopen(u8 *base);
// 0x573740 (thiscall on the virtual base): the viewport's Clear2 (slot 0x50) of one rectangle: the corner from 0x8bcbf8 - 0x8bcc00, 0x8bcbfc - 0x8bcc04 (__ftol), plus the size 0x8bcbe8, 0x8bcbec; targets 1 unless the caps 0x8bcbc8 have the top bit,
// | 2 by bit 0x10 of +0x220; the colour 0x8bd47c when the caps have any of 0x4002000, else 0; z 1.0, stencil 0. Kept, pushed (kind 2): whether it was 0.
u32 __fastcall D3dSetup_Clear(u8 *base);
// 0x572d50 (the game jumps here at 0x583718): with an object at [0x879da8]: 0x8bd52c = 0, closed as D3dSetup_Close, freed, the pointer cleared: 1. None: 0.
u32 __cdecl D3dSetup_Teardown();
// 0x573060 (stdcall (format, context)): D3dSetup_PickFormat(context, format, 0).
u32 __stdcall D3dSetup_OnFormat(const u32 *format, u32 context);
// 0x572f70 (stdcall (unused, format, unused)): each of the four formats in the table at 0x7015f8 (four signed words of bits: alpha, red, green, blue) whose bits the masks at +0x1c, +0x10, +0x14, +0x18 have takes a copy (0x20 bytes) at 0x8bd558 + 0x20 i,
// logged with flag 0x1000000. Returns 1.
u32 __stdcall D3dSetup_PickFormat(u32 unused, const u32 *format, u32 unused2);
// 0x573660 (stdcall (format, wanted)): either null: 0. The dwords +4 differing: 1. Else the format copied (0x20 bytes) to the wanted one, 0 when its +0xc is 0x10, else 1.
u32 __stdcall D3dSetup_OnZFormat(const u32 *format, u32 *wanted);
}

#include "../core/gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(char[40], gText_D3dSetup_PickFormat_7688c4, 0x007688c4, 0x28);
GAME_DATA(char[36], gText_D3dSetup_Open_7688ec, 0x007688ec, 0x24);
GAME_DATA(char[72], gText_D3dsetup_768910, 0x00768910, 0x48);
GAME_DATA(char[20], gText_D3dsetup_768958, 0x00768958, 0x14);
GAME_DATA(char[28], gText_D3dsetup_76896c, 0x0076896c, 0x1c);
GAME_DATA(char[16], gText_D3dsetup_768988, 0x00768988, 0x10);
GAME_DATA(char[24], gText_D3dsetup_768998, 0x00768998, 0x18);
GAME_DATA(char[32], gText_D3dsetup_7689b0, 0x007689b0, 0x20);
GAME_DATA(char[40], gText_D3dsetup_7689d0, 0x007689d0, 0x28);
