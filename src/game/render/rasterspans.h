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
}
