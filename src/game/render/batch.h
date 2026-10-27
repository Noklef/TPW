// Starting a draw batch for a primitive kind, and one for a model's material: reimplementation of 0x565240 and 0x581880; a model's
// triangles added to a batch (0x581a00).
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x581880 (thiscall on a model (+4 its settings, +8 its flags); index, &size; pops 8; name unconfirmed): a draw batch for the model's
// material entry index (16 bytes each from the settings' +0x6c; its first dword the material: +2 the texture's index in the set at
// +4, used only with 0x40 in the byte at 0x8bd550). The set's texture (TexSet_Texture), when there is one, is "shaded" with flag 0x400
// and no dword at +0x3c. Batch_Begin on the object at [0x879ab0]: mode 2 for settings flags 0xc0000 (else 1), texture 4, kind 0x2c4 for
// settings flag 0x2000000 or a shaded texture (else 0x1c4), flags TexLoad_DriverFlags(model, entry, texture), extra the texture's
// +0x1c with its flag 0x1000 (else 0). The batch's +0 = the model's flag 0x2000000 >> 24, with 0x80 when shaded; +0x34 the texture's
// +0x44 (no texture: -1); *size = the vertex size (0x20 for 0x1c4, else 0x28). Returns the batch; none: 0, the rest skipped.
u8 *__fastcall Model_BeginBatch(u8 *model, void *, u32 index, u32 *size);

// 0x565240 (thiscall on the caller's object; mode, texture, kind, flags, extra, pops 20): asks the draw buffer
// ([0x8bcbc4]) for a batch (DrawBuffer_Begin) with the mode, which also has bit 4 when it has bit 2, the
// object's mask at +0xc meets the flags and the scene flags (0x8bd508) lack 0x4000000. The batch is kept at
// +0x20 (null: 0 is returned and nothing else done). Its +4 = flags, +0x18 = texture, +0x1c = kind, +8 = extra,
// +0x34 = -1. The object's +0x1c is the vertex size for the kind (0xc4: 0x18, 0x1c4: 0x20, 0x2c4: 0x28,
// 0x3c4: 0x30, else 0), +0x28 and +0x2c the buffer's vertex and index limits. Returns the batch.
u8 *__fastcall Batch_Begin(u8 *object, void *, u32 mode, u32 texture, u32 kind, u32 flags, u32 extra);

// 0x565e70 (thiscall on the same object; vertices, count, pops 8; name unconfirmed): with fog in the render flags (0x8bcbc8 bit
// 0x2000): colour fog (bit 0x4000) is applied to the vertices (VertexFog_Colour, the object's vertex size +0x1c as the stride) and
// the batch's (+0x20) flags at +4 lose 0x100000 and gain 0x80000; else alpha fog (bit 0x8000) is applied (VertexFog_Alpha).
void __fastcall Batch_FogVertices(u8 *object, void *, u8 *vertices, u32 count);

// 0x565ee0 (thiscall on the same object; name unconfirmed): with the driver kind (0x8bd524) 1, the same on the batch's own vertices
// (+0x28) and count (+0x20), without the fog bit's test.
void __fastcall Batch_Fog(u8 *object, void *);

// 0x581a00 (thiscall on a model (+4 its settings); batch, matrix, vertices; pops 12; returns 1; name unconfirmed): the transformer
// ([0x87b1d8]) slot 2 (cdecl: transformer, matrix, vertices, the settings' +0x60, 0x10, 0, the vertex count +0x58) first. Then for each
// material entry (16 bytes from the settings' +0x6c, read once; as many as the word +0x5a, read again each time): the vertex count's
// vertices (0x10 bytes each) appended to the batch's (+0x28, 0x20 bytes each, from its count +0x20, which then grows by the count): x and
// z (the dwords 0 and 8) times the int at 0x8bd4ac (x87, as floats) to +0 and +4, the dwords 4 and 0xc to +8 and +0xc, -1 to +0x10. Then
// the entry's faces (8 bytes each from the settings' +0x70, from the entry's word +4, as many as its word +6): the corners (the words
// +2, +4, +6, less the entry's word +8, through the settings' u16 table +0x94) of each face that Tri_WindingTest would pass (its x, y
// and w copied to 0x8c63b0-0x8c63d0 the same way, the first x through the x87; the same products) go to the batch's indices (+0x2c, from
// its count +0x24, which grows by 3) plus the vertex count the entry's vertices started at. The settings and the batch's vertices are
// read again for each face.
u32 __fastcall Model_AddTriangles(u8 *model, void *, u8 *batch, const void *matrix, const u8 *vertices);
}
