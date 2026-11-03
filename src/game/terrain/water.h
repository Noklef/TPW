// The landscape's water surface drawn into a batch (name invented, unconfirmed): reimplementation of 0x570d90.
#pragma once

#include "../../../re/types.h"

// Its calls (a bench's recorders in their place).
struct TerrainWaterEnv {
    u8 *(__fastcall *begin)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra); // Batch_Begin (0x565240)
    u32(__fastcall *close)(u8 *object, void *, float depth);                                            // Batch_Close (0x5658c0)
    u32(__fastcall *transform)(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);         // Transformer_Transform (0x570d20)
    s32(__stdcall *clip)(s32 flags, u8 *dest, u8 *source, u32 *count, s32 stride);                      // ClipPoly_Vertices (0x562a70)
};
void TerrainWater_SetEnv(const TerrainWaterEnv *env);

extern "C" {
// 0x570d90 (thiscall, no arguments; name invented, unconfirmed). Returns 1 at once with no heights at +0x12970. With the settings' top bits (0x8bd508) clear and a batch at +0x2958 kept from before: its vertices
// (+0x20, +0x28) only moved by the landscape point's change in x (their +0x1c) and z (+0x18), the point kept at +0x295c. Else the landscape's texture (none: 1), a batch (mode 1, 4, kind 0x1c4; none: 1) with +0 0x60 for the
// settings' 0x2000 (else 0x20). Rows +0x128 to +0x12c: each row's points, columns byte +0x28 + row to byte +0xa8 + row (0x7f or -1: none), x and z from the grid at +0 (+8, +0xc, steps +0x10, +0x14), y the height plus
// 0x8bd4ec, transformed and clip coded; each cell of columns byte +0x130 + row up to byte +0x1b0 + row with a quad unless all four codes share a bit. Its UVs from the point, its alpha off along row 0 and next row 0, its first
// corner by the cell's bit 1. No code set: the quad's two triangles by their sides, a fan of 3 or 4 (the second triangle alone moved down). Some code set: each facing triangle through ClipPoly_Vertices, a fan of what comes
// back. The vertex or index room used up (the renderer's +0x28, +0x2c) ends it. No vertices or no indices: +0x2958 cleared and 1; else Batch_Close with vertices * 1900000 (as a signed dword), its answer 0 or 1.
// Sic: the columns of a row unset, or under 0 or over 126, read and write the original's frame as they fall; a row's first column past its last + 1 and a run of 0 points run away.
u32 __fastcall TerrainWater_Draw(u8 *self);
}
