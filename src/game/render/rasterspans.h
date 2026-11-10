// The software rasteriser's hand-written MMX span routines in cseg (0x6f1d70-0x6f6155), entered by the triangle set-ups' JMP [0xfbe044]
// with the row pointer in EBP and ECX EBP EBX EDX ESI EDI saved on the stack, which they pop before returning (src/hook/regthunks.h).
// Each draws the triangle's two parts, [0xfbe204] rows from the edges [0xfbe208] (left) and [0xfbe20c] (right), then [0xfbe200] rows
// from [0xfbe210] and [0xfbe214] (stored to 0xfbe208 / 0xfbe20c first); the row pointer steps by -[0xfbe02c], the depth row [0xfbe228]
// by -[0xfbe030]. Names invented (unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6f1d70 (table 0x6f19f0, the rasteriser's default; EBP the row pointer): 32-bit texels ([0xfbe038], indexed by the top bytes of u
// and v, masked by [0xfbe008]) times four shade words, kept where the 16-bit depth passes: [0xfbe034] 0 draws where the buffer's is
// at least the pixel's, else at most. Its row loops 0x6f1de0 and 0x6f1fc0 are RasterSpan_TexShadeZ's own. The depth written is the
// next pixel's (sic), and [0xfbe218] (the original's saved ESP) is not written.
void __cdecl RasterSpan_TexShadeZ(u32 row);

// 0x6f2140 (table 0x6f19f4, the blend with flag 0x800; row loops 0x6f21b0, 0x6f23c0): as RasterSpan_TexShadeZ, but each pixel the
// shaded texel (8 * byte, times its shade word) times the row of 0xfbe2e0 for the texel's top 6 bits, plus the pixel under it (4 *
// byte) times the row of 0xfbe4e0, saturated; the depth is not written.
void __cdecl RasterSpan_TexBlend(u32 row);

// 0x6f2570 (table 0x6f1a20, the blend without flag 0x800; row loops 0x6f25e0, 0x6f2800): RasterSpan_TexBlend's pixel, kept with
// the (next pixel's, sic) depth only where the texel's top 6 bits are above [0xfbe010] (signed).
void __cdecl RasterSpan_TexBlendAlphaZ(u32 row);

// 0x6f29d0 (table 0x6f19fc; row loops 0x6f2a40, 0x6f2c40): the texel unmasked and shaded as RasterSpan_TexShadeZ's, then 4 * each
// shaded word and 4 * the pixel under it times their tables' rows, chosen by the top 6 bits of the second shaded word (sic); each run
// of drawn pixels takes its first pixel's texel (sic); the depth is not written, and the second depth test is below (not at most).
void __cdecl RasterSpan_TexBlendLit(u32 row);

// 0x6f2de0 (table 0x6f19f8; row loops 0x6f2e50, 0x6f3030): RasterSpan_TexShadeZ's pixel added to the pixel under it, each byte
// saturated; the depth is not written.
void __cdecl RasterSpan_TexAdd(u32 row);

// 0x6f31c0 (table 0x6f1a04; row loops 0x6f3230, 0x6f34b0): RasterSpan_TexShadeZ filtered: u and v less [0xfbe0e4] at each row's
// start, the four texels about them (the index stepped by 1, 0x100, -1, masked each time) weighted by their bits 10-25 masked by
// [0xfbe1b8]; the depth written is the pixel's own.
void __cdecl RasterSpan_BilinearShadeZ(u32 row);

// 0x6f36d0 (table 0x6f1a00; row loops 0x6f3740, 0x6f39c0): RasterSpan_BilinearShadeZ's pixel added to the pixel under it, each
// byte saturated (as RasterSpan_TexAdd's); the depth is not written.
void __cdecl RasterSpan_BilinearAdd(u32 row);

// 0x6f3be0 (table 0x6f1a08; row loops 0x6f3c50, 0x6f3ef0): RasterSpan_TexBlend's blend of the filtered texel (the tables' rows by
// its fourth word >> 2); the depth is not written.
void __cdecl RasterSpan_BilinearBlend(u32 row);

// 0x6f4140 (table 0x6f1a0c; row loops 0x6f41b0, 0x6f4450): the filtered texel (the index masked, its neighbours not: + 1, 0x100,
// 0x101) blended as RasterSpan_TexBlendLit's, by the fourth shaded word >> 2; the depth written is the pixel's own.
void __cdecl RasterSpan_BilinearBlendLitZ(u32 row);

// 0x6f4690 (table 0x6f1a10; row loops 0x6f4700, 0x6f4890): untextured, each pixel the shade words >> 7, saturated, with the next
// pixel's depth (sic); the shade is not prestepped (sic), and after a drawn pixel the next is tested against the drawn one's depth
// (sic); the edges' u and v are not stepped.
void __cdecl RasterSpan_ShadeZ(u32 row);
}
