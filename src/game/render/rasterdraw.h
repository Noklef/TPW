// The software rasteriser device's drawing methods (with render/rasteriser): a primitive's triangles, the test quad, a filled rectangle and a mapped triangle, through the hand-written MMX routines 0x6ef7f0, 0x6f02e0, 0x6f0a50 and the fans 0x6792f0, 0x679350. Reimplementation of 0x56a030-0x56a641; the names are invented (unconfirmed).
//   A primitive: +4 its flags, +8 its span setting, +0x18 its kind, +0x20 its vertices' count, +0x24 its indices' count (3 a triangle), +0x28 the vertices (0x18 bytes each), +0x2c the indices (u16), +0x34 the colour.
#pragma once

#include "../../../re/types.h"
#include "../core/addrcalls.h"

// Test seam: the routines replaced by address (tests/fncmp/rasterdraw.py).
void RasterDraw_SetCalls(const AddrCall *table, u32 count);

extern "C" {

// 0x56a030 (thiscall (primitive), through the virtual base; pops 4): flag 0x4000 draws nothing. The virtual base's slots 8 (the flags) and 0xc (0, the setting +8, or 0 with bit 0 of 0x8bd550); 0xfbe008 = +0x34; each triangle by 0x6f02e0 with flag 0x10000000 and not 0x100, else 0x6ef7f0. EMMS with flag 0x1000 unless bit 26 of 0x8bd508. Returns 1.
u32 __fastcall RasterDraw_Primitive(u8 *self, void *, const u8 *primitive);

// 0x56a180 (stdcall (primitive), pops 4): 0xfbe044 = [0x6f19cc]; each triangle by 0x6ef7f0, then the whole by 0x6792f0 (+0x18, 3, +0x28, +0x20, +0x2c, +0x24, 0); EMMS. Returns 1.
u32 __stdcall RasterDraw_Outlined(const u8 *primitive);

// 0x56a230 (cdecl): 0xfbe044 = [0x6f1a10]; a fixed quad of four colours by 0x6792f0; EMMS. Returns the fan's answer. (The vertices' last two dwords are left unset.)
u32 __cdecl RasterDraw_TestQuad();

// 0x56a2f0 (thiscall (x0, y0, x1, y1, colour, texture), pops 0x18): the rectangle, its corners * [0x7013b0] by __ftol, at depth 1000. No texture: 0xfbe044 = [0x6f19cc], the mapping left unset. A texture: 0xfbe044 = [0x6f19b0], the virtual base's slot 0xc (0, the texture's slot 8), the mapping to its +8 and +0xc * 16. By 0x679350; EMMS. Returns 1.
u32 __fastcall RasterDraw_Rect(u8 *self, void *, float x0, float y0, float x1, float y1, u32 colour, const u8 *texture);

// 0x56a540 (stdcall (scale, colour, specular, u0, v0, u1, v1), pops 0x1c): a triangle over the screen from the origin to scale - [0x7013b4], at depth 50 and 1/z 0.02, its mapping the three points (u0, v0), then u1 and v1 stretched by (u1 - u0) / scale, all * [0x7013c4]; 0xfbe008 = 0x7f7f; FixedVerts_PackRounded; by 0x6f0a50; EMMS. Returns 1.
u32 __stdcall RasterDraw_MappedTriangle(float scale, u32 colour, u32 specular, float u0, float v0, float u1, float v1);
}
