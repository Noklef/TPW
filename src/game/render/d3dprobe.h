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
// 0x567130 (thiscall on the probe + 0x2c, the base's slot 0x11; size, diffuse, specular, u0, v0, u1, v1 (floats but the colours); pops
// 0x1c; name unconfirmed): the base's slot 7, then one triangle through the device (DrawPrimitive, COM slot 0x70: TRIANGLELIST, FVF 0x1c4
// (xyzrhw, diffuse, specular, one uv pair), three vertices, flags 1): (-0.5, -0.5) uv (u0, v0), (size - 0.5, -0.5) uv (u1, v0) and
// (-0.5, size - 0.5) uv (u0, v1), each at z 50, rhw 0.02, with the colours; then the base's slot 8. 1. The floats go through the x87 (a
// signalling NaN comes out quiet) but the first vertex's x, a plain -0.5.
u32 __fastcall D3dProbe_Draw(u8 *self, void *, u32 size, u32 diffuse, u32 specular, u32 u0, u32 v0, u32 u1, u32 v1);

// The renderer's drawing on the same object, thiscall on the probe + 0x2c. All names unconfirmed. A device call's result goes to the result
// global 0x879cd4 where noted ("kept"): a negative one's message (DxResult_Message) written to the engine debug log; "pushed": any non-zero one also on the
// engine's error list 0x879ad0 with the kind given. Vertices are FVF 0x1c4 (xyzrhw, diffuse, specular, one uv pair), 0x20 bytes each.

// 0x566fa0: stage 0's state 0x13 = [0x8bd4a8], then BeginScene (COM slot 0x24), kept, pushed (kind 3): whether it was 0.
u32 __fastcall D3dProbe_BeginScene(u8 *self);
// 0x567010: EndScene (COM slot 0x28), kept, pushed (kind 3): whether it was 0.
u32 __fastcall D3dProbe_EndScene(u8 *self);
// 0x567060 (the base's slot 0x10; x, y, out; pops 0xc; this is not used): locks the surface at [0x879a24] + 0x14 (COM slot 0x64; flags 1), kept; a non-zero result answers 0. Reads the
// pixel at (x, y) by the desc's pitch and surface and the bits per pixel 0x8bcbe4 (16: a word, else a dword), *out = it & [0x8bd4dc], unlocks (COM slot 0x80), 1.
u32 __fastcall D3dProbe_ReadPixel(u8 *self, void *, s32 x, s32 y, u32 *out);
// 0x5675c0 (stage, texture; pops 8): 1 when the texture is the one at the probe's +0x20 (whatever the stage). Otherwise SetTexture (COM slot 0x98), kept: a non-zero result
// answers 0; else the texture stored, 1.
u32 __fastcall D3dProbe_SetTexture(u8 *self, void *, u32 stage, u32 texture);
// 0x567ad0 (start, end (floats), colour; pops 0xc): Obj_SetColour8b4 of the render caps object 0x8bcbc8; 0x8bd498 = 1 / (end - start) when that is above 0, else 0; render
// states 0x30 = 0, FOGCOLOR = colour; with a render caps bit 0xc000 FOGTABLEMODE 0, else FOGTABLEMODE 3, start and end. 1. The results are not looked at: the original's test of them never fires (sic).
u32 __fastcall D3dProbe_SetFog(u8 *self, void *, u32 start, u32 end, u32 colour);
// 0x568ac0: the base's slot 7, no texture, a fixed four-coloured quad (DrawIndexedPrimitive, COM slot 0x74, whose result is not looked at), the base's slot 8: slot 8's answer.
u32 __fastcall D3dProbe_DrawTestQuad(u8 *self);
// 0x568c50 (x0, y0, x1, y1 (floats), colour, texture or null (its slot 2 gives the handle); pops 0x18): a textured quad, the states it needs only where the flags set last differ:
// alpha not 0xff blends (low byte 0x56: states 0x13 = 5, 0x14 = 6; state 0x1b on; ALPHAREF off), opaque state 0x1b off; then ZENABLE on, ALPHATESTENABLE off. Kept, pushed (kind 2): whether it was 0.
u32 __fastcall D3dProbe_DrawQuad(u8 *self, void *, u32 x0, u32 y0, u32 x1, u32 y1, u32 colour, u8 *texture);
// 0x568f50 (points (pairs of floats), count, colour, scale; pops 0x10): no texture; a line strip (DrawPrimitive, type 3) of the points times the scale at z 1e-6, rhw 1. Room for 16 on
// the stack, past that an overrun (sic). Kept, pushed (kind 2): whether it was 0.
u32 __fastcall D3dProbe_DrawLines(u8 *self, void *, const float *points, s32 count, u32 colour, float scale);
// 0x569040: no texture, flags 0x9d56; a frame 5 wide round the screen (0x8bcbd0, 0x8bcbd4): black outer corners and transparent inner, all 0x80000000 with the render caps bit 0x8000000.
// Kept, pushed (kind 2). 1.
u32 __fastcall D3dProbe_DrawFrame(u8 *self);
// 0x569410 (eight corners of four floats each, unused, flags; pops 0xc): the corners white, those on a side the flags' low six bits select red; the vertices given to the transform
// object [0x87b1d8] (its slot 3, cdecl, (object, vertices, 0x20, 8, 0), a float answer dropped); no texture, flags 0x56; states 0x1b off, ZENABLE on, 0xe on, 0x1d and 0x1c off where the flags
// set last differ; a line strip through 16 indices (flags 9), kept. 1.
u32 __fastcall D3dProbe_DrawBox(u8 *self, void *, const u32 *corners, u32, u32 flags);
// 0x569720: pixels per tick: no texture, the base's slot 7 (0: answers 0), flags 0x9256, a screen quad drawn three times (48 indices); a non-zero result global after a draw
// (the draws do not set it, sic) stops there: slot 8, 0. Then slot 8 (0: answers 0), else width * height * 50 / (the ticks taken * 16), in dwords that wrap, by fild and fidiv.
float __fastcall D3dProbe_FillRate(u8 *self);
// 0x567c10 (thiscall on the probe, pops 4; name invented): with the probe's +4 bit 0 or the batch's flags bit 0x100: the batch's vertices copied to the draw buffer's free ones (past themselves when they are those),
// their colours 0xffff0000, then drawn by the batch's flags: 0x556 and 0x2756 blended, 0x22 and 0x52 not at all. Returns 1.
u32 __fastcall D3dProbe_DrawBatchTinted(u8 *probe, void *, u8 *batch);
// 0x5680b0 (thiscall on the probe + 0x2c, pops 4; name invented): a batch through DrawIndexedPrimitive (flags 0xd), kept each time; first, with bit 31 of the render caps and the probe's +0x18 4, the batch's flags lose bits
// 0x180000 and go through the base's slot 2 and D3dProbe_DrawBatchTinted. Its flags go through slot 2 and +8's texture to stage 0. Flags bit 0x4000, by the probe's +4 & 3: 0 nothing, 1 +0xc's texture to stage 1 and one draw, 2 and 3
// a draw, then flags 0x5156 / 0x556, +0xc's texture, stage 0's texture coordinate set 1, a draw, the set back. Without that bit nothing unless the index count (+0x24) is above 0: one draw, or with bit 7 of the first byte by the
// base's +0x24: 3 D3dProbe_SetTextureStages 1, a draw, 2, a draw, 0; 1 three draws: +0x10's texture, not blended; +0xc's, low byte 0x22; +8's, low byte 0x93; others nothing. Pushed (kind 2): whether the kept result is 0.
u32 __fastcall D3dProbe_DrawBatch(u8 *self, void *, u8 *batch);
// 0x566840 (thiscall on the probe + 0x2c, a virtual; name invented): the device from the caps object [0x879da8] + 8 to the probe's +0x24; its GetCaps (COM slot 3; a failure logged) on two stack structures of 0xfc bytes.
// A destination blend caps (+0x78 of the first) of exactly 0x20 logs " -- Detected PowerVR1, ..." (with 0x8bd508 bit 0x2000000; EngineDebug_LogFallback) and answers 0. Otherwise +4 from the caps (more than one stage, +0xf8, and bit 3 at +0xf4: 1; else
// blend bits: 2), the fixed render and stage 0 states, the filters (+0xc, +0x10, +0x14) by the options 0x8bd550 and the caps at [0x879da8] + 0xb0, +0x1c by +0xb4, +0x18 = 4; quirks by vendor and chip at [0x879a24] + 0x428 / 0x42c in the render caps 0x8bcbc8; the two probes;
// the texture stages (1 and 2 only when +4 & 3 is 1); the flags set last -1, the base's slot 2 (0). 1. Debug flag 0x1000000 of 0x8bd508: "Alpha ref %x" and "Fog algorithm %x" to the engine debug log.
u32 __fastcall D3dProbe_SetUpDevice(u8 *self);
}

// The clock D3dProbe_FillRate reads (default Cpu_Ticks16). nullptr restores it.
struct D3dProbeEnv {
    u32(__cdecl *ticks)();
};
void D3dProbe_SetEnv(const D3dProbeEnv *e);
