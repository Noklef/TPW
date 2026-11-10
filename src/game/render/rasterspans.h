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
}
