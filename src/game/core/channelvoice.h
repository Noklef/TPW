// A sound channel's voice (0x8c bytes, in an array the channel's pool holds; the class name is invented): +4 its sound (an object whose
// slot 2 releases it), +0x10 flags, +0x2c an id, +0x58 a value cleared on release, +0x74 a critical section; its vtable's slot 11
// takes a value on release. The pool (the channel's +0x14): +0 its capacity, +4 the array, +8 the count in use, +0xc the channel. The
// voices' count (the dword at 0xfb2058) is kept with the Interlocked imports. Reimplementation of 0x6c5060, 0x6c5070, 0x6c5080,
// 0x6bb7a0, 0x6bb7c0, 0x6b8330, 0x6babc0 and 0x6bac30; the names are invented.
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
// 0x6bb7c0 (thiscall (value), pops 4): inside its critical section: with a sound, slot 11 (value), the sound's slot 2, +4 = 0 and the
// count decremented; then flag 2 (+0x10) cleared and +0x58 = 0 either way. Returns 1 when there was a sound, else 0 (a byte).
u8 __fastcall ChannelVoice_Release(u8 *voice, void *, u32 value);
// 0x6b8330 (thiscall (on, mask), pops 8): the mask set in the flags (+0x10) when the byte on is set, else cleared.
void __fastcall ChannelVoice_SetFlag(u8 *voice, void *, u8 on, u32 mask);

// 0x6babc0 (thiscall): the pool's channel's voice release (soundchannel 0x6b7e60: a local out, the voice, 1) for each voice in use, the first even
// with none in use (sic: tested after it), none without an array.
void __fastcall ChannelVoices_ReleaseAll(u8 *pool);
// 0x6bac30 (thiscall (settings), pops 4): the settings' +0x40 or the capacity, the smaller (unsigned), becomes the count in use (+8),
// after ChannelVoices_ReleaseAll.
void __fastcall ChannelVoices_SetCount(u8 *pool, void *, const u8 *settings);

}  // extern "C"
