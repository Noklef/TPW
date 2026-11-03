// The movie player's DirectSound helpers (the video library at 0x66e000-0x678000): the settings' (the player's dword +0) +0x54 the
// IDirectSound, +0x58 the primary buffer, +0x5c the streaming buffer; the player's +0x36c the stream's size in bytes, +0x370 the last
// write position, +0x3c8 / +0x3cc a simulated play position and its flag (no sound device). COM calls through each object's vtable
// (Release 8, GetCaps 0xc, GetCurrentPosition 0x10). Reimplementation of 0x672950, 0x6729e0, 0x672bf0, 0x672730 and 0x672990. The names are
// unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x672950 (thiscall on an IDirectSoundBuffer; name unconfirmed): its size: GetCaps into a DSBCAPS (0x14 bytes, cleared, its size set),
// whose +8 (dwBufferBytes) is returned whatever GetCaps answers.
u32 __fastcall MovieSound_BufferBytes(void *buffer, void *edx);

// 0x6729e0 (thiscall on a COM pointer's place, returns 0; name unconfirmed): the object there released (Release) and the place cleared,
// unless it is null.
u32 __fastcall MovieSound_Release(void **place, void *edx);

// 0x672bf0 (thiscall on the player, returns 0; name unconfirmed): the settings' streaming buffer (+0x5c) released and cleared (the
// settings read again), unless it is null.
u32 __fastcall MovieSound_ReleaseStream(u8 *player, void *edx);

// 0x672990 (thiscall on the player, returns 0; name invented): with flag 0x800000 of the settings' +4: when the player's +0x3d8 is set,
// the primary buffer (+0x58) released and cleared unless null; when +0x3dc is set, the device (+0x54) likewise (MovieSound_Release).
u32 __fastcall MovieSound_ReleaseAll(u8 *player, void *edx);

// 0x672a00 (thiscall; returns 0; name invented): the stream's secondary buffer. With the device (settings +0x54): CreateSoundBuffer (0xc) into the settings' +0x5c: flags 0x100e0, the bytes +0x3d4, 16-bit stereo PCM at the rate +0x5c. SetFrequency (0x44) with the rate
// and with the settings' flag 0x2000 SetVolume (0x3c) with their +0x8c, results ignored; GetCaps (0xc) into a cleared DSBCAPS: its bytes in +0x36c, +0x370 and +0x3cc 0. Lock (0x2c) all of it: when that answers 0 both spans zeroed (the second if any) and Unlock (0x4c). Play (0x30)
// looping. GetCaps or Play failing: Release (8). Without a buffer: the settings' +0x5c 0, +0x3c8 0, +0x36c 0x8000, +0x3cc 1 (simulated), +0x370 0. Always +0x368 1.
u32 __fastcall MovieSound_CreateStream(u8 *player, void *edx);

// 0x672730 (thiscall on the player; name unconfirmed): how far the play position is past the last write (+0x370), wrapped by the
// stream's size (+0x36c): the position the simulated one (+0x3c8 moved on by 0x372 and, at or past the size (signed), back by it) while
// +0x3cc is set, else the streaming buffer's play cursor (GetCurrentPosition; on a failure the original reads its uninitialised local);
// then position - last when last is below it (signed), else position - last + size.
s32 __fastcall MovieSound_PlayDistance(u8 *player, void *edx);
}
