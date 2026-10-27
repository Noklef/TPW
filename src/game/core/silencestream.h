// The silence stream (vtable 0x70ab58, made by 0x6c6730 near 0x6ba5d3; the class and the names are invented): a sound stream that, once
// its next stream (+4) has read to its end, plays silence: +0x14 the silent bytes played, +0x18 how many in all, +0x2c set once the next
// stream has ended. Reimplementation of 0x6c67f0 and 0x6c67d0.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c67f0 (thiscall (buffer, bytes, done, flag), pops 0x10; slot 7): without +0x2c the next stream's slot 7 (the same arguments);
// 0 when that answers 0, else +0x2c set. Then zeros at the buffer + *done, as many as bytes less *done, at most the silence left
// (+0x18 less +0x14; unsigned), *done and +0x14 on by that; 1 once +0x14 is not below +0x18. Else 0.
u32 __fastcall SilenceStream_Read(u8 *stream, void *, u8 *buffer, u32 bytes, u32 *done, u32 flag);

// 0x6c67d0 (thiscall (flags), pops 4; slot 0): VoiceSample_Dtor (the sound samples' destructor), then with bit 0 of the flags the
// block back to the pool (SilenceStreamPool_Delete, 0x30 bytes). Returns the stream.
u8 *__fastcall SilenceStream_DeletingDtor(u8 *stream, void *, u32 flags);

}  // extern "C"
