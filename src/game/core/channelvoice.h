// A sound channel's voice (0x8c bytes, in an array the channel's pool holds; the class name is invented): +4 its sound (an object whose
// slot 2 releases it), +0x10 flags, +0x2c an id, +0x58 a value cleared on release, +0x74 a critical section; its vtable's slot 11
// takes a value on release. The pool (the channel's +0x14): +0 its capacity, +4 the array, +8 the count in use, +0xc the channel. The
// voices' count (the dword at 0xfb2058) is kept with the Interlocked imports. Reimplementation of 0x6c5060, 0x6c5070, 0x6c5080,
// 0x6bb7a0, 0x6bb7c0, 0x6b8330, 0x6bb3e0, 0x6babc0, 0x6bac30, the constructor 0x6bb270 and the array's new 0x6baf50; the names are
// invented.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6c5060: the count (0xfb2058) = 0.
void __cdecl ChannelVoiceCount_Reset();
// 0x6c5070 / 0x6c5080 (thiscall on the count): InterlockedIncrement / InterlockedDecrement (the imports 0x6fd0c0 / 0x6fd12c); returns
// what it answers.
s32 __fastcall ChannelVoiceCount_Increment(s32 *count);
s32 __fastcall ChannelVoiceCount_Decrement(s32 *count);

// 0x6bb7a0 (thiscall): the voice's sound (+4), read inside its critical section.
u8 *__fastcall ChannelVoice_Sound(u8 *voice);
// 0x6bb270 (thiscall; an SEH frame): the sound stream's base (+4 / +8 / +0xc 0, vtable 0x70a778), +0x10, +0x18 / +0x1c / +0x20 / +0x24,
// +0x2c, +0x34 / +0x38 0; +0x64 a block of 0x2420 bytes (Mem_Alloc) and +0x68 that 32-byte aligned up; +0x6c an event (CreateEventA
// (0, 1, 0, 0): manual reset, not signalled); the critical section +0x74 initialised; +0x58 0; vtable 0x70a7a8. Returns the voice.
u8 *__fastcall ChannelVoice_Ctor(u8 *voice);
// 0x6bb860 (thiscall; name invented): the rate (+0x18) as it is when the modifier mask (+0x24) is 0; else the rate (fild qword: as
// unsigned) times the product of the modifiers (+0x34, four floats) whose bits (0-3) are set (1.0 for none of them; the first loaded,
// the rest multiplied in at the game's 24-bit precision), stored to a float and fistp'd (to nearest; 0x80000000 past the range).
u32 __fastcall ChannelVoice_ScaledRate(const u8 *voice);
// 0x6bb8d0 (thiscall (value), pops 4; name invented): the dword at this keeps its bits below the shift (the byte at 0x782db8) + 1 (that
// mask made once: the guard 0xfb1f18 bit 0, the mask at 0xfb1f1c), stored, then the value's low word shifted left by the shift added in.
void __fastcall VoiceId_SetHigh(u32 *id, void *, u32 value);
// 0x6baf50 (thiscall (count), pops 4; an SEH frame): new[] of count voices: a block of count * 0x8c + 4 (Mem_Alloc), the count at +0,
// the voices from +4 constructed (CRT_VectorCtorIterator with ChannelVoice_Ctor; the destructor 0x6bb050 for the unwinding); the array
// (null when the allocation fails) into the holder's +0. Returns the holder.
u8 *__fastcall ChannelVoices_New(u8 *holder, void *, u32 count);
// 0x6bb7c0 (thiscall (value), pops 4): inside its critical section: with a sound, slot 11 (value), the sound's slot 2, +4 = 0 and the
// count decremented; then flag 2 (+0x10) cleared and +0x58 = 0 either way. Returns 1 when there was a sound, else 0 (a byte).
u8 __fastcall ChannelVoice_Release(u8 *voice, void *, u32 value);
// The voice's own slots (vtable 0x70a7a8; names invented). 0x6bb050 (thiscall; an SEH frame): the destructor: vtable 0x70a7a8, the
// critical section deleted, the event (+0x6c) closed (CloseHandle), the buffer block (+0x64) freed (Mem_Free), vtable 0x70a778.
// 0x6baff0 (thiscall (flags), pops 4; slot 0): flags bit 1 the array's delete (CRT_VectorDtorIterator over the count at -4 with the
// destructor, the block from -4 freed; sic: bit 0 not tested), else the destructor and the block freed for bit 0; returns the voice.
// Each of the rest inside the voice's critical section: 0x6bb730 (thiscall (next), pops 4; slot 1) SoundStream_SetNext; 0x6bb6c0
// (thiscall (id pointer), pops 4; slot 4) SoundStream_Find of *id; 0x6bb470 (thiscall (out, record), pops 8; slot 9) SoundStream_Query
// (its out the record argument's own stack slot), +0x58 the record's +0x2c with its flag 0x400 (+0x14), then *out = 0 (sic: the answer
// dropped), out returned; 0x6bb5e0 (thiscall (buffer, bytes, out, flag), pops 0x10; slot 7) *out = 0, the sound's (+4) slot 7 (buffer,
// bytes, out, 0) when there is one, ChannelVoice_Release (0) when it answers 1; the buffer past *out to bytes zeroed; returns 1 after a
// release, else 0.
void __fastcall ChannelVoice_Dtor(u8 *voice);
u8 *__fastcall ChannelVoice_DeletingDtor(u8 *voice, void *, u32 flags);
u8 *__fastcall ChannelVoice_SetNext(u8 *voice, void *, u8 *next);
u8 *__fastcall ChannelVoice_Find(u8 *voice, void *, const u32 *id);
u32 *__fastcall ChannelVoice_Query(u8 *voice, void *, u32 *out, u8 *record);
u32 __fastcall ChannelVoice_Read(u8 *voice, void *, u8 *buffer, u32 bytes, u32 *out, u32 flag);
// 0x6b8330 (thiscall (on, mask), pops 8): the mask set in the flags (+0x10) when the byte on is set, else cleared.
void __fastcall ChannelVoice_SetFlag(u8 *voice, void *, u8 on, u32 mask);
// 0x6bb3e0 (thiscall (record, out), pops 8; a sound record's settings taken): without flag 2 (+0x10), +0x24 and +0x1c 0, the flags 2 and
// +0x14 the record's +0xc. The record's flag 0x400 (+0x14): +0x58 its +0x2c, the flag cleared. +0x1c the record's +8; flag 2: +0x18 its
// +0x28; flag 0x40000: +0x34 / +0x38 the pair at its +0x94, +0x24 its +0x98. *out the id (+0x2c) shifted right by the byte at 0x782db8;
// returns the id.
u32 __fastcall ChannelVoice_Apply(u8 *voice, void *, u8 *record, u32 *out);

// 0x6babc0 (thiscall): the pool's channel's voice release (soundchannel 0x6b7e60: a local out, the voice, 1) for each voice in use, the first even
// with none in use (sic: tested after it), none without an array.
void __fastcall ChannelVoices_ReleaseAll(u8 *pool);
// 0x6bac30 (thiscall (settings), pops 4): the settings' +0x40 or the capacity, the smaller (unsigned), becomes the count in use (+8),
// after ChannelVoices_ReleaseAll.
void __fastcall ChannelVoices_SetCount(u8 *pool, void *, const u8 *settings);
// 0x6ba230 (thiscall (channel, count), pops 8; an SEH frame; name invented): the pool: +0 the count, +4 the voices (ChannelVoices_New),
// +8 0, +0xc the channel, +0x10 the voices' events (Mem_Alloc 0x18, VoiceEvents_Ctor (the voices, the count); null when it fails).
// Returns the pool.
u8 *__fastcall ChannelVoices_Ctor(u8 *pool, void *, u8 *channel, u32 count);
// 0x6ba2c0 (thiscall (out, requested), pops 8; name invented): the count in use (+8) = requested or the capacity, the smaller (unsigned). For each voice (the index compared
// by its low word) its id's high part (VoiceId_SetHigh of +0x2c: the index), the events' (+0x10) handle i (their +0x10) = CreateEventA (0, 1, 0, 0), its +0x70 = the
// handle's address. The handle after them another such event. The thread (0x67e050: 0x6ba220, the proc for the channel's service loop 0x6c6180; 0; the pool) at
// SetThreadPriority 0 whether made or not. *out = 0, or -1 without a thread. Returns out.
u32 *__fastcall ChannelVoices_Start(u8 *pool, void *, u32 *out, u32 requested);
// 0x6bb590 (thiscall; from the channel's service loop 0x6c6180; name invented): the voice's buffer (+0x68) refilled by its slot 7 (the
// read: the buffer, 0x2400 bytes, a local out, 0); +0x30 = 1 unless the read answered 1 (the end); ResetEvent (*+0x70), SetEvent (+0x6c).
// Returns +0x30.
u32 __fastcall ChannelVoice_Refill(u8 *voice);

// The pool's lookups by a voice's handle (*handle shifted right by the byte at 0x782db8: the voice's index in the array +4, not checked;
// the voice's slot 12 (0x6bb6c0: (handle)) says whether the handle is the voice's; names invented). 0x6baa60 (thiscall (&handle), pops
// 4): the voice when its slot 12 answers nonzero, else 0.
u8 *__fastcall ChannelVoices_Find(u8 *pool, void *, const u32 *handle);
// 0x6baad0 (thiscall (&handle, out), pops 8): as ChannelVoices_Find, slot 12's answer into *out.
u8 *__fastcall ChannelVoices_FindOut(u8 *pool, void *, const u32 *handle, u32 *out);
// 0x6bab50 (thiscall (&handle, out), pops 8): *out = the index; 1 when slot 12 answers nonzero, else 0.
u32 __fastcall ChannelVoices_Holds(u8 *pool, void *, const u32 *handle, u32 *out);

// 0x6ba390 (thiscall; name invented): ChannelVoice_Release (0) on each of the pool's voices (+4, the count +8; with an array, the first
// whatever the count); returns the sum of their bytes.
u32 __fastcall ChannelVoices_ReleaseEach(u8 *pool);

// 0x6ba3e0 (thiscall; an SEH frame; name invented): the pool's destructor: each voice released (ChannelVoices_ReleaseEach), the voice
// events (+0x10) stopped (VoiceEvents_Stop; not checked for null, sic), a spin while their flags' (+0x14) bit 1 is set, then their slot 0
// (1: delete) unless null and the voices array's (+4) slot 0 (3: delete the array) unless null.
void __fastcall ChannelVoices_Dtor(u8 *pool);
// 0x6ba610 (thiscall (voice, play), pops 8; the pool unused; name invented): a sample stream for the play record (+0 the sample's index,
// +4 its resource reference, +0x14 flags): the sample's 0x28-byte header read (SoundResourceHandle_ReadRecord through a local pointer
// the read may move; none, or its +4 zero: nothing). With a voice: the header copied to its +0x3c, and for play flag 0x8000 its +0x20 =
// the play's +0x1c. By the header's +0x1b: 0x20 an MP2 sample, 0x10 a voice-coded one (each streamed from the reference's stream (slot
// 11; at its offset (slot 9) + 0x28 for MP2) when the reference's slot 13 says so, else over memory, from their pools), 0x40 the
// reference's slot 15 (out the play's stack slot) and 0 returned (no voice released, sic), else PCM (a file sample likewise, or a memory
// sample from its pool: SoundSample_Ctor, the vtable 0x70a6f8); the source the reference's +8. Then the filters
// (SoundStream_BuildChain), for play flag 0x800 a 0x70a6c8 stream (+0x18 the play's +0x4c; null chained anyway, sic) in front and the
// flag cleared; the voice's +0x2c counter stepped into the stream's slot 10 (no voice: a fault, as in the original); the stream's slot 9
// (out the play's stack slot, the play): not negative: for an MP2 sample with a voice, its +0x54 word = slot 6's rate when it differs;
// the stream returned. Else the stream closed (slot 2). Failures release the voice (ChannelVoice_Release (0)) and return 0.
u8 *__fastcall ChannelVoices_CreateStream(u8 *pool, void *, u8 *voice, u32 *play);

// 0x6b7380 (thiscall; name invented; reached only from the exception unwind funclets 0x6e57e3...): LeaveCriticalSection of the section at *lock.
void __fastcall CriticalSectionLock_Leave(void **lock);
// 0x6bb3b0 (thiscall (sound, value), pops 8; name invented): the voice's sound (+4) and its +0x58, read inside its critical section.
void __fastcall ChannelVoice_SoundAndValue(u8 *voice, void *, u32 *sound, u32 *value);
// 0x6bb320 (thiscall (out, handle, value), pops 0xc; an SEH frame; name invented): inside the critical section: the voice's slot 12 (&handle); its
// answer's slot 9 (out, value), or *out = -1. Returns out.
u32 *__fastcall ChannelVoice_Send(u8 *voice, void *, u32 *out, u32 handle, u32 value);
// 0x6bb510 (thiscall (dest, size), pops 8; name invented): waits 500 ms for the voice's event (+0x6c). Timed out: size bytes of silence, returns 1.
// Else the copier (0xfb2018) slot 0 (dest, the buffer +0x68, size), the event reset, the second event (*+0x70) set when +0x30 is set; returns +0x30.
u32 __fastcall ChannelVoice_Mix(u8 *voice, void *, u8 *dest, u32 size);
// 0x6baa20 (thiscall (out, handle, value), pops 0xc; name invented): ChannelVoice_Send on the handle's voice, or *out = -1. Returns out.
u32 *__fastcall ChannelVoices_Send(u8 *pool, void *, u32 *out, u32 handle, u32 value);
// 0x6bac60 (thiscall (desc), pops 4; name invented): a voice for a play record. Walks the voices (a null array: none; one at least, whatever the count):
// with the record's flags +0xc 0x200 and +0x14 bit 2 and the voice's +0x1c the record's +8: that voice, released (value 0). A voice without a sound
// whose index the channel's source's slot 17 answers 0 for: that voice as it is. Else the lowest +0x58 (below -1), if the record's +0x2c is above it:
// that voice, released (value 1), the record's +0x10 and +8 from its +0x2c and +0x1c. Else 0.
u8 *__fastcall ChannelVoices_Acquire(u8 *pool, void *, u8 *desc);
// 0x6baee0 (thiscall (handle, out, desc), pops 0xc; name invented): with a handle, ChannelVoices_FindOut: *out the voice; returns slot 12's answer (or 0).
// Without: ChannelVoices_Acquire into *out; a voice gets the record's +0x2c at +0x58 and the voices' count a step. Returns *out.
u8 *__fastcall ChannelVoices_AcquireBy(u8 *pool, void *, u32 handle, u8 **out, u8 *desc);
// 0x6badd0 (thiscall (handle, record, out), pops 0xc; name invented): for record flags 0x80 and 8 (else 0). The resource's slot 12 answering 0: a stream
// with no voice, returned. Else a voice (AcquireBy; 0 without one), its stream made, handed to the answer's slot 1 (0 when it answers 0), and
// ChannelVoice_Apply's result.
u32 __fastcall ChannelVoices_PlayRecord(u8 *pool, void *, u32 handle, u8 *record, u32 *out);
// 0x6bae60 (thiscall (handle, record), pops 8; name invented): the resource's slot 12 not 0 and record flag 0x2000 (else 0): a voice (AcquireBy), a
// notify stream handed to the answer's slot 1; what that answers (0: 0) gets slot 10 (the voice's +0x2c stepped); returns its answer.
u32 __fastcall ChannelVoices_PlayNotify(u8 *pool, void *, u32 handle, u8 *record);

// The pool's thread, so that the bench can record it (nullptr: the game's).
struct ChannelVoicesEnv {
    u32(__cdecl *beginThread)(u32 entry, u32 stack, void *arg);   // 0x67e050 (agent B's, unported)
    u8 *(__fastcall *find)(u8 *pool, void *, const u32 *handle);  // ChannelVoices_Find, for ChannelVoiceHandle_Resume
};
void ChannelVoices_SetEnv(const ChannelVoicesEnv *env);

// 0x6c5b20 (thiscall; pops 8; name invented, unconfirmed; a method of the class at 0x70a61c): under the semaphore 0xfb205c: the handle +4 found in the owner's pool +0x14, the voice's events +0x6c reset and *(+0x70) set, then the owner's +0x18 object's method 0x24 (out, voice).
// Any step failing: *out = -1. Answers out.
u32 *__fastcall ChannelVoiceHandle_Resume(const u8 *self, void *, u32 *out, u8 *owner);

}  // extern "C"

#include "gamedata.h"

// The vtables hold only ports: tpw.exe's loader points their slots at them (.docs/09-globals.md).
GAME_DATA(void *[12], Class70a6c8_vtable, 0x0070a6c8, 0x30);
GAME_DATA(void *[20], Class70a6f8_vtable, 0x0070a6f8, 0x50);
GAME_DATA(void *[12], Class70a778_vtable, 0x0070a778, 0x30);
GAME_DATA(void *[13], Class70a7a8_vtable, 0x0070a7a8, 0x34);
