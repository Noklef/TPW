// Starting a draw batch for a primitive kind, and one for a model's material: reimplementation of 0x565240 and 0x581880.
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
}
