// A window on a stream (vtable 0x70ad50; the class name is invented): +4 the position within the window, +8 the window's start in the
// stream, +0xc the stream (a StreamBase sharing the source: handles.h; slot 2 read (buffer, count), 4 seek (offset, origin), 6 tell),
// +0x14 a semaphore (core/semaphore.h) held around a read. Reimplementation of 0x6d4d10, 0x6d4d80, 0x6d4df0, 0x6d4e90 and 0x6d4ec0;
// the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d4d10 (thiscall (stream); pops 4): +4 = +8 = 0, StreamBase_CopyCtor (+0xc, stream), Semaphore_Ctor (+0x14), vtable 0x70ad50.
// Returns the object.
u8 *__fastcall SubStream_Ctor(u8 *self, void *, const u8 *stream);
// 0x6d4d80 (thiscall (stream, start); pops 8): as SubStream_Ctor, the window starting at start (+8).
u8 *__fastcall SubStream_CtorAt(u8 *self, void *, const u8 *stream, u32 start);
// 0x6d4df0 (thiscall (buffer, count); pops 8; slot 1): under the semaphore (a SemaphoreLock acquired for good), the object's slot 2
// (seek) to its position unless the stream's tell answers start + position; then the stream's read (buffer, count), its answer added to
// the position (+4, read again after the seek). Returns the new position.
u32 __fastcall SubStream_Read(u8 *self, void *, void *buffer, u32 count);
// 0x6d4e90 (thiscall (offset); pops 4; slot 2): the stream sought to (offset + start, 0); the position = its answer - start. Returns
// the stream's answer.
u32 __fastcall SubStream_Seek(u8 *self, void *, u32 offset);
// 0x6d4ec0 (thiscall): the position = 0 and the stream sought to (start, 0). Returns the stream's answer.
u32 __fastcall SubStream_Rewind(u8 *self);

}  // extern "C"
