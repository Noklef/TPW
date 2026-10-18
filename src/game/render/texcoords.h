// A model's texture coordinates written into a draw batch: reimplementation of 0x581410.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x581410 (cdecl; out, settings, index, texture, stride, mode, and two dwords it ignores (0x580bd3 passes 0, 0); name unconfirmed):
// for the vertices first..last of the settings' entry index (16 bytes each from +0x6c: u16 +8 first, u16 +0xa last; their count
// last - first + 1 as a u32), a float pair (u, v) at out, then out + stride and so on. x87 in the original's order. Without a texture,
// or one without flag 0x1000 (+8) or a dword at +0x1c, zeros. Else with the texture's u scale +0x20, v scale +0x24, u offset +0x28, v
// offset +0x2c: mode 1 maps the normals (+0x64, 12 bytes each, through the u16s at +0x94 from first) by the view at [0x87afe8]: u =
// ((n . row 0) + 1) * 0.5 * u scale + u offset, v = (0.5 - (n . row 1) * 0.5) * v scale + v offset. Otherwise the u and v from +0x68
// (packs of four: 32 bytes, the u at +4 * lane, the v at +0x10 + 4 * lane) times the scales plus the offsets, and with mode bit 2
// shifted by s and t, the dots of the light at [[0x87a248] + 0x30] with the two vectors of each vertex's 24 bytes from +0x74, times
// [0x8c1618] and clamped to +-[0x8c161c] (NaNs to the minus side), times u scale / s16 +0x14 and -(v scale / s16 +0x16).
void __cdecl Model_TexCoords(u8 *out, const u8 *settings, s32 index, const u8 *texture, s32 stride, s32 mode);

// 0x566690 (thiscall on a transformed vertex (0x20 bytes as its callers from 0x566220 on fill them: x, y, z, rhw, colour, specular, (u, v));
// stage, u, v; pops 12; name unconfirmed): the stage's (u, v) at +0x18 + 8 * stage, through the x87 (no bound on stage).
void __fastcall Vertex_SetTexCoords(u8 *vertex, void *, s32 stage, float u, float v);
}
