// The render settings written to the engine's debug log: reimplementation of 0x5833c0, 0x5835c0 and 0x584790. Names invented, unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x5833c0 (thiscall on the settings): "Display info", the flags' (+0) triple buffer, fog and sprite shadow bits (each a masked value, not 0 or 1), the four Z buffer floats +0x48 to +0x54 and the
// sprite shadow line again, logged twice (sic). The maximum view distance (+0x8f8) is made into a line that the original never logs (sic), so it is left out here.
void __fastcall Settings_LogDisplay(const u8 *settings);

// 0x5835c0 (thiscall on the settings): "System info", the clock +0x14 over 1000 (signed), the RAM +0x18, the renderer +0x1c, the maths +0x20 and the "RELEASE VERSION" line.
void __fastcall Settings_LogSystem(const u8 *settings);

// 0x584790 (thiscall on the settings): "Texturing info", the flags' (+0) procedural (4), mipmap (2), trilinear (8) and anisotropic (0x10) bits (masked values) and the floats +0x40d8, +0x40e0, +0x40e4, +0x40e8 and +0x40ec.
void __fastcall Settings_LogTexturing(const u8 *settings);
}
