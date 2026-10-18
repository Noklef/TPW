// Starting a draw batch for a primitive kind: reimplementation of 0x565240.
#pragma once

#include "../../../re/types.h"

extern "C" {

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
}
