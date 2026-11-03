// One material group of a model's mesh into the open batch: reimplementation of 0x580530 (name invented, unconfirmed).
#pragma once

#include "../../../re/types.h"

extern "C" {

// What ModelGroup_AddToBatch calls through a table (the port by default; its bench runs them as recorders).
struct ModelGroupEnv {
    void(__cdecl *texCoords)(u8 *out, const u8 *settings, s32 index, const u8 *texture, s32 stride, s32 mode);  // Model_TexCoords (the original pushes two zeros more, which it ignores)
    void(__cdecl *modulate)(u8 *verts, u32 colour, u32 count, s32 stride);  // FixedVerts_Modulate
    void(__fastcall *fog)(u8 *object, void *, u8 *vertices, u32 count);  // Batch_FogVertices
    u16 *(__stdcall *fan)(s32 flags, u16 *end, u8 *vertices, u32 *vertexCount, s32 stride);  // ClipPoly_TriangleFan
};
void ModelGroup_SetEnv(const ModelGroupEnv *env);  // nullptr: the port's

// 0x580530 (thiscall on a model (+4 its settings, +8 flags, +0xc 16-byte vertices, +0x10 colours, or none); batch, group, stride; pops 0xc; always returns 1; name invented, unconfirmed;
// x87 in the original's order). R is the object at [0x879ab0]. Nothing happens unless the settings' whole mesh (the words +0x5c faces, +0x5e vertices) fits R's vertex and index room, counted
// from R's batch (+0x20) and this batch's counts (+0x20 vertices, +0x24 indices). Group is an entry of 16 bytes from the settings' +0x6c: +0 the material (+2 the texture number, read only when
// [0x8bd550] has bit 6, +4 the texture set), the words +4 first face and +6 faces (8 bytes each from the settings' +0x70: the words +2, +4, +6 its corners), +8 first and +0xa last vertex, +0xc
// bit 0 no colours. A texture without an id (+8 bit 0x1000, +0x1c) or none: zeros for UV (at each vertex's +0x18). Else u from the block at the settings' +0x68 (u[4], v[4], 32 bytes each) times
// the texture's scale +0x20 / +0x24 plus its offset +0x28 / +0x2c; with +8 bit 0x8000000 a sphere map instead, from the normal (the settings' +0x64, through the u16 table +0x94) and the
// view's ([0x87afe8]) rows 0 and 1: (dot + 1) * 0.5 for u, 0.5 - dot * 0.5 for v. (The original has a third way for flags 2 that those flags cannot reach, left out here: its local is 0 or 1.)
// The settings' dword +0 bit 0x2000000: a second layer at +0x20 the same way from the texture set at [+0x98]+4 (a texture is taken as there: none faults, sic), the batch's +0xc its id or 0.
// Else a texture with +8 bit 0x400 and no +0x3c: Model_TexCoords (+0x20, mode 2), the batch's +0xc the id at +0x68's flags and +0x7c, with flag 0x800000 its +0x10 the id at +0xc8 and +0xdc.
// The vertices through the table copied with colour (the colour & [[0x87a248]+0x2c], or that mask alone); a texture without an id: FixedVerts_Modulate by its object's (+0x40, with +8 bit
// 0x10000000) slot 3, else -1. The faces with this model's flags & 0x3f: clip codes for the vertices (all out: the batch's count put back, return), each face not out on one plane that
// Tri_WindingTest passes goes to the indices and, with a code set, ClipPoly_TriangleFan, until R's room less ten vertices or 0x30 bytes of index ends; no index at all: the count put back,
// return. Without flags: each face that passes, all of them. Then Batch_FogVertices, the transformer's ([0x87b1d8]) slot 3 (cdecl, a float) added to the batch's +0x30, which the batch's +4
// bit 0x800000 overwrites (0 with the model's flag bit 0x2000000, else count * [0x8bcc14] + 0.01); a texture with +8 bit 0x4000000 sets the batch's +0 bit 0x400.
u32 __fastcall ModelGroup_AddToBatch(u8 *model, void *, u8 *batch, u32 group, s32 stride);
}
