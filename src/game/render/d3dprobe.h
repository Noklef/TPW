// The Direct3D driver probes (0x567270, 0x5673d0): tests run while the renderer is set up (from
// 0x566a8d) that draw a pixel through the device and read it back to see whether the driver
// honours a feature. The probe object (+0 its virtual base table: the base at the offset in its
// second dword) draws through its base: slot 2 sets the state flags (D3dProbe_SetFlags; the flags
// set last are at base +0x14, and the probes store their complement there first so that every
// state is set again), slot 3 (stage, texture) sets a texture (the probes: none), slot 0x11 (20.0f, colour, specular, 0, 0, 0, 0) draws, slot
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
// Setting the flags (0x567620, the base's slot 2 through the thunk 0x569cc0: on the probe + 0x2c):
// each flag that differs from the flags set last (base +0x14) sets its render state and then
// that bit at +0x14: 0x400 ALPHATESTENABLE, 0x200 ALPHAREF (the dword pair at 0x7012d8 + the
// probe's +8 * 4: the second when the flag is set), 0x8000 ZENABLE (off when set), 0x800
// ZWRITEENABLE (off when set), 0x80000 SPECULARENABLE, 0x100000 FOGENABLE, 0x1000000
// DITHERENABLE (off when set), 0x2000000 state 0x1f (off when set), 0x4000000
// TEXTUREPERSPECTIVE (off when set); then the stage states through Shadow_SetBits3 (0x20000,
// 0x40000, 0x10000, 0x400000, and 0x4000 twice), stage 0 state 0x11 (2 with 0x400000, else 1)
// always. With bit 31 of the render caps 0x8bcbc8 it forces state 0x1b on (Shadow_SetBits2 with
// -1) and the low byte to 0x52 when the flags' is 0x52, else 0x22; otherwise, unless the flags
// set last are now the flags, the low byte to the flags' and state 0x1b by flag 0x100. Answers 1.
//
// The texture stages of a pass (0x5686d0, only when the probe's +0x18 is 4): mode 0 sets stage 0
// (modulate the texture by the diffuse colour) and stage 1 (select the texture's colour); mode 1
// turns state 0x1b off (the flags set last too), stage 0 to the texture set +8 holds and stage 1
// to blend in +0xc's; mode 2 turns state 0x1b on and the low byte to 0x93 (states 0x13 = 9,
// 0x14 = 3), stage 0 to +0xc's texture and stage 1 to modulate +8's; modes 1 and 2 then disable
// stage 2. Other modes do nothing. The textures go through the base's slot 3 (stage, texture).
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
// 0x5686d0 (thiscall; mode, the textures, pops 8): nothing unless the probe's +0x18 is 4.
void __fastcall D3dProbe_SetTextureStages(u8 *probe, void *, u32 mode, const u8 *textures);
// 0x567620 (thiscall on the probe + 0x2c; flags, pops 4): 1.
u32 __fastcall D3dProbe_SetFlags(u8 *self, void *, u32 flags);
}
