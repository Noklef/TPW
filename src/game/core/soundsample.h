// The sound samples' shared slots (the sound streams of src/game/core/soundstream that are sources, vtables 0x70a6c8-0x70aec0; the
// names are invented): +0x1c the time (the game's clock, ms) at which a sample whose data has run out has finished playing (0 while it
// still has data), +0x20 the format byte (bit 1 stereo, bit 6 16-bit, bits 3 / 4 the rate, see Obj_SampleRate20). Reimplementation of
// 0x6bb1d0, 0x6c7010, 0x6c7070, 0x6c6ef0, the constructor 0x6c6e70, the deleting destructors 0x6c70c0 / 0x6c70a0, the memory
// sample's read 0x6c7250 and the file sample's read and rewind 0x6c7890 / 0x6c7920.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6bb1d0 (thiscall (record), pops 4; slot 5): SoundStream_Describe (record).
void __fastcall SoundSample_Describe(u8 *sample, void *, u8 *record);
// 0x6c7010 (thiscall (flag, bytes), pops 8; slot 19): with the flag's low byte 0, the time the bytes left take to play (halved for 16-bit
// and again for stereo, * 1000 / the sample rate (slot 6), unsigned) + 200 ms from now (Time_GetMs) into +0x1c; returns 1. Else 0.
u8 __fastcall SoundSample_ScheduleEnd(u8 *sample, void *, u32 flag, u32 bytes);
// 0x6c7070 (thiscall): 1 when the game's clock (Time_GetMs) has reached +0x1c (unsigned), else 0.
u32 __fastcall SoundSample_Drained(u8 *sample);
// 0x6c6ef0 (thiscall; the destructor of 0x70ac50): vtable 0x70ac50; the object at +0x24, if any, its slot 0 (the release); vtable
// 0x70a778 (the sound stream's, whose destructor has nothing more to do).
void __fastcall SoundSample_Dtor(u8 *sample);

// 0x6c6e70 (thiscall (record, source), pops 8; the sample stream 0x70ac50's constructor; an SEH frame for the source): +4 / +8 / +0xc,
// +0x14 and +0x1c 0 (the vtable 0x70aca0 first), +0x10 the record + 0x28, +0x18 the record's +4, +0x24 the source and its slot 1 (the
// AddRef); then the vtable 0x70ac50 and SoundStream_SetFormat with the record. Returns the sample.
u8 *__fastcall SoundSample_Ctor(u8 *sample, void *, u8 *record, u8 *source);
// 0x6c70c0 (thiscall (flags), pops 4; slot 0 of 0x70ac50): SoundSample_Dtor, the block freed (Mem_Free) for flags bit 0. Returns the sample.
u8 *__fastcall SoundSample_DeletingDtor(u8 *sample, void *, u32 flags);
// 0x6c70a0 (thiscall (flags), pops 4; slot 0 of 0x70aca0, slot 20 of 0x70ac50): the base's vtable (Vtbl70a778_Set_6c7090), the block
// freed (Mem_Free) for flags bit 0. Returns the stream.
u8 *__fastcall SoundSampleBase_DeletingDtor(u8 *stream, void *, u32 flags);

// 0x6c7250 (thiscall (buffer, bytes, out, flag), pops 0x10; slot 7 of 0x70a6f8, the memory sample: +0x10 the data, +0x14 the position,
// +0x18 the length): once ended (+0x1c set), *out 0 and SoundSample_Drained. Else the bytes left or fewer (unsigned) into *out, copied
// from the data + the position by the copier at [0xfb2018] (its slot 0: (buffer, source, count)), the position advanced; at the end
// (unsigned) slot 19 (the end's schedule, (flag, *out)): 0 when it answers nonzero, else 1. Returns 0 otherwise.
u32 __fastcall SoundSample_ReadMemory(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *out, u32 flag);
// 0x6c7890 (thiscall (buffer, bytes, out, flag), pops 0x10; slot 7 of 0x70acf0, the file sample: +0x2c a window on the file, SubStream
// (vtable 0x70ad40), its position +0x30; +0x18 the length): as SoundSample_ReadMemory, the bytes read by the window's slot 1 (buffer,
// count), which advances the position; the end tested on the position read again.
u32 __fastcall SoundSample_ReadFile(u8 *sample, void *, u8 *buffer, u32 bytes, u32 *out, u32 flag);
// 0x6c7920 (thiscall; slot 8 of 0x70acf0): SubStream_Rewind on the window (+0x2c); returns its answer.
u32 __fastcall SoundSample_RewindFile(u8 *sample);
// The file sample (vtable 0x70acf0; the window on its file, a SubStream, at +0x2c with the vtable 0x70ad40): 0x6c7780 (thiscall (stream,
// record, start, source), pops 0x10): SoundSample_Ctor (record, source), the vtable 0x70a6f8, SubStream_CtorAt (+0x2c, stream, start), its
// vtable 0x70ad40, the sample's 0x70acf0; returns the sample. 0x6c7820 (thiscall): the window's (vtable 0x70ad50; Semaphore_Dtor (+0x14),
// StreamBase_Dtor (+0xc)), then SoundSample_Dtor. 0x6c7800 (thiscall (flags), pops 4; slot 0): it, and with flags bit 0 the block back
// to its pool (FileSamplePool_Delete, 0x48); returns the sample. (Names invented.)
u8 *__fastcall FileSample_Ctor(u8 *sample, void *, const u8 *stream, u8 *record, u32 start, u8 *source);
void __fastcall FileSample_Dtor(u8 *sample);
u8 *__fastcall FileSample_DeletingDtor(u8 *sample, void *, u32 flags);

// The memory samples' copier [0xfb2018] (names invented): a 4-byte object whose slot 0 copies (dest, source, count): vtable 0x70aa34 the
// plain one (Mem_CopyForward 0x6c4010), 0x70aa38 the MMX one. 0x6c3f70 (thiscall; called by the sound system's constructor 0x6b7360 on its
// +0x24, which it leaves alone): a new copier (Mem_Alloc(4)), the MMX one when [0xfb2704] (the MMX switch) is set, else the plain one,
// kept in [0xfb2018] (0 when the block fails). Returns the object.
u8 *__fastcall SampleCopier_Install(u8 *self);
// 0x6c3fc0 (thiscall (dest, source, count), pops 0xc; slot 0 of 0x70aa38; `this` unused): count / 32 blocks of 32 bytes, each read as four
// qwords (MMX) before they are written, then EMMS (when there were blocks) and the count % 32 bytes left a byte at a time (rep movsb), all
// forwards (so overlapping runs repeat as the original's do).
void __fastcall SampleCopier_MmxCopy(void *self, void *, u8 *dest, const u8 *source, u32 count);

}  // extern "C"
