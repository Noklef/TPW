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
// 0x6baf50 (thiscall (count), pops 4; an SEH frame): new[] of count voices: a block of count * 0x8c + 4 (Mem_Alloc), the count at +0,
// the voices from +4 constructed (CRT_VectorCtorIterator with ChannelVoice_Ctor; the destructor 0x6bb050 for the unwinding); the array
// (null when the allocation fails) into the holder's +0. Returns the holder.
u8 *__fastcall ChannelVoices_New(u8 *holder, void *, u32 count);
// 0x6bb7c0 (thiscall (value), pops 4): inside its critical section: with a sound, slot 11 (value), the sound's slot 2, +4 = 0 and the
// count decremented; then flag 2 (+0x10) cleared and +0x58 = 0 either way. Returns 1 when there was a sound, else 0 (a byte).
u8 __fastcall ChannelVoice_Release(u8 *voice, void *, u32 value);
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
// 0x6bb590 (thiscall; from the channel's service loop 0x6c6180; name invented): the voice's buffer (+0x68) refilled by its slot 7 (the
// read: the buffer, 0x2400 bytes, a local out, 0); +0x30 = 1 unless the read answered 1 (the end); ResetEvent (*+0x70), SetEvent (+0x6c).
// Returns +0x30.
u32 __fastcall ChannelVoice_Refill(u8 *voice);

}  // extern "C"
