// The landscape's ground drawn, a batch for each run of a texture (name invented, unconfirmed): reimplementation of 0x56f670.
#pragma once

#include "../../../re/types.h"

// Its calls (a bench's recorders in their place).
struct TerrainGroundEnv {
    void(__fastcall *drain)(u8 *node);                                                                    // ListNodes_Drain (0x576230)
    u32(__fastcall *setView)(u8 *object, void *, const u32 *matrix, u32 unused);                          // Meshes_SetView (0x5743a0)
    void(__fastcall *light)(u8 *self);                                                                    // 0x56ef10
    void(__fastcall *collect)(u8 *self, void *, u32 *ids, s32 *count, u32 *textures);                     // Bands_CollectTextures (0x56f380)
    u8 *(__fastcall *begin)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);   // Batch_Begin (0x565240)
    u8 *(__fastcall *open)(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);    // DrawBatch_Open (0x565310)
    u32(__fastcall *transform)(void *object, void *, u32 a, u32 b, u32 c, u32 d, u32 e, u32 f);           // Transformer_Transform (0x570d20)
    s32(__stdcall *clip)(s32 flags, u8 *dest, u8 *source, u32 *count, s32 stride);                        // ClipPoly_Vertices (0x562a70)
    void(__fastcall *fog)(u8 *object, void *);                                                            // Batch_Fog (0x565ee0)
    void(__fastcall *transformVertices)(u8 *object, void *, u32 arg);                                     // Batch_TransformVertices (0x565f50)
    void(__cdecl *pack)(u8 *verts, u32 count, s32 stride);                                                // FixedVerts_PackRounded (0x563130)
    u32(__fastcall *close)(u8 *object, void *, float depth);                                              // Batch_Close (0x5658c0)
};
void TerrainGround_SetEnv(const TerrainGroundEnv *env);

extern "C" {
// 0x56f670 (thiscall, no arguments; name invented, unconfirmed). Returns 1 at once with the rows +0x128 past +0x12c or the settings' top bits (0x8bd508) clear. 0x879dec = 0; the node at [[0x87a6d0] + 0x2c]
// drained, the view set (0x87b090), the grid's colours lit, the texture ids and textures collected. A batch (mode 1, 4, kind 0x1c4; flags 0x1256, with fog 0x101256; the first id; none: 1). Rows +0x128 to
// +0x12c as the water's: each row's points, columns byte +0x28 + row to byte +0xa8 + row (first 0x7f or over: none, the last row's stay), x and z from the grid at +0, y its heights' dwords, transformed and
// clip coded. Each cell of columns byte +0x130 + row up to byte +0x1b0 + row whose first dword lacks bit 0 takes the next texture, also when all four codes share a bit (no quad then). Its corners' UVs the
// table (0,0) (0,1) (1,1) (1,0) by Bytes4_Permute of 0, 1, 2, 3 with the cell's word, scaled by the texture's +0x20, +0x24, moved by its +0x28, +0x2c; the colours +0x2968 by row * 0x80 + column, halved
// with the cell's byte +1 bit 4 without the settings' 0x800 (render flags); the corner order by the cell's bit 1. No code set: its two triangles by their sides, a record of 4 or 3 vertices (the second alone
// moved down). Some code set: each facing triangle through ClipPoly_Vertices, a record of the two counts. Then the batch's count, fog with the render flags' 0x2000, the transformer, the fixed-point pack with
// the driver kind (0x8bd524) 4, the records sorted by id; each run of an id a batch (the first one kept, its +8 the id; then DrawBatch_Open), the run's first record's +8 at +0x34, the fans' indices, the
// flag 0x10 off and 0x100 on, all vertices its count (sic: each run), Batch_Close at vertices * 2000000 (as a signed dword); 0 from it: 0. Else 1. Sic: columns outside a row's points read the frame's stack.
u32 __fastcall TerrainGround_Draw(u8 *self);
}
