// The Direct3D driver probes (0x567270, 0x5673d0): tests run while the renderer is set up (from
// 0x566a8d) that draw a pixel through the device and read it back to see whether the driver
// honours a feature. The probe object (+0 its virtual base table: the base at the offset in its
// second dword) draws through its base: slot 2 sets the state flags (whose complement it keeps at
// base +0x14), slot 3 (0, 0) clears, slot 0x11 (20.0f, colour, specular, 0, 0, 0, 0) draws, slot
// 0x10 (0, 0, &pixel) reads the pixel back (0 on failure). The device is the IDirect3DDevice3 at
// +0x24 (SetRenderState, COM slot 0x58; SetTextureStageState, 0xa0).
//
// Alpha test (0x567270): without the caps bit 0x40 at [0x879da8] + 0xa4 it only sets base +0xc
// to 0x1000 and answers 1. Otherwise +8 = 0, then it draws one pixel, then an alpha-tested pixel
// of alpha 0x80 over it (ALPHAFUNC GREATEREQUAL, ALPHATESTENABLE, ALPHAREF 0xff; stage 0 colour
// and alpha from the texture); a pixel read back as 0 sets +8 = 2. 0 when the read fails.
//
// Fog (0x5673d0): clears bits 0x4000 / 0x8000 of the render caps (0x8bcbc8); without the caps
// bit 0x80000 at [0x879da8] + 0xa8 it sets 0x4000 and answers 1. Otherwise it draws white with
// linear table fog (black, 0 to 100) and reads the pixel (masked with 0x8bd4dc): a failed read
// sets 0x8000 and answers 0. A pixel neither 0 nor the whole mask stops there; else it draws
// again without table fog, fog colour white, specular alpha 0x80: a pixel 0 or the whole mask sets
// 0x4000, another 0x8000 (a failed read neither). When neither bit is set, 0x800000. Answers 1.
//
// The originals' quirks are kept: the pixel a read writes is not set before (the alpha test's
// starts as the probe's own address, the fog test's as whatever the stack held); it is read only
// when the read succeeds.
#pragma once

#include "../../../re/types.h"

extern "C" {
// 0x567270 (thiscall): 1, or 0 when the read back fails.
u32 __fastcall D3dProbe_AlphaTest(u8 *probe);
// 0x5673d0 (thiscall): 1, or 0 when the first read back fails.
u32 __fastcall D3dProbe_FogTest(u8 *probe);
}
