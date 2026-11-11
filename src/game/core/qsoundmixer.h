// The QSound mixer source (vtable 0x711920; core/soundsource.h has its config, flags, rate and destructors): its calls on the QMixer library, the session handle at +0x18. +4 the channels, +8 bit 0: y and z as they are (else swapped),
// +0xc the rate, +0x1c the array of wave handles, +0x20 the mixer config, +0x24 the open flags. Reimplementation of 0x6d2110-0x6d2d20; the names are invented, unconfirmed. Most answer *out = -1 when the library's
// answer is not 0, else 0, and return out ("answered" below). "Open" is the source's slot 2 answering not 0; closed, *out = -1 and no call.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6d2110 (slot 6, (out, info)): SoundSource711920_Configure; QSWaveMixInitEx (+0x20) into +0x18, null a failure; QSWaveMixActivate (session, 1); the device's primary buffer made (SoundDevice_CreatePrimaryOf);
// QSWaveMixOpenChannel (session, +4, 2); QSWaveMixSetListenerPosition (session, a zero vector, 0), its answer ignored; slot 25 (a stack out, a stack slot holding 0x70a27c), SetFlags, *out = 0. A failure: *out = -1.
u32 *__fastcall SoundSource711920_Open(u8 *source, void *, u32 *out, const u8 *info);
// 0x6d2220 (slot 5, (out, flags)): QSWaveMixSetOptions (session, the mask of bits 4 -> 1, 8 -> 2, 2 -> 8, -1), answered.
u32 *__fastcall SoundSource711920_SetOptions(u8 *source, void *, u32 *out, u32 flags);
// 0x6d2260 (slot 4, (which, out)): *out = 0; with a session QSWaveMixGetDirectSoundBuffer (session, which, out), *out = 0 again unless it answers 0. Returns its answer (no session: 0).
u32 __fastcall SoundSource711920_GetBuffer(u8 *source, void *, u32 which, u32 *out);
// 0x6d22b0 (slot 7): with a session QSWaveMixCloseSession and +0x18 = 0.
void __fastcall SoundSource711920_Close(u8 *source);
// 0x6d2320 (slot 26, (out, channel, flags)): QSWaveMixSetPanRate (session, channel, 0, 500 with flags' 0x400, else 33); QSWaveMixEnableChannel (session, channel, 8, Obj_RemapFlags (flags)), answered.
u32 *__fastcall SoundSource711920_EnableChannel(u8 *source, void *, u32 *out, u32 channel, u32 flags);
// 0x6d2380 (slot 9, (out, info); needs open): channel = info's +0x2c >> [0x782db8]. QSWaveMixConfigureChannel (session, channel, +0x24 | 0x1000000 for info's +0x14 bit 0, else 0x2000000, the format of the record at info's +0x3c, 0);
// QSWaveMixOpenWaveEx (session, {format, Obj_Get20Or2400 (info), the callback 0x6bb4f0, info, 0}, 0x11), null a failure. Slot 26 (a stack out, channel, info's +0x14) negative: the wave freed, a failure. An older wave in the channel
// freed; the new one in the array; QSWaveMixPlayEx (session, channel, 0x20, wave, 0, 0): not 0: the array's entry 0, the wave freed, a failure. Else *out = 0.
u32 *__fastcall SoundSource711920_PlayWave(u8 *source, void *, u32 *out, u8 *info);
// 0x6d24c0, 0x6d2500, 0x6d2540 (slots 10-12, (out, channel); need open): QSWaveMixStopChannel, PauseChannel, RestartChannel (session, channel, 0), answered.
u32 *__fastcall SoundSource711920_StopChannel(u8 *source, void *, u32 *out, u32 channel);
u32 *__fastcall SoundSource711920_PauseChannel(u8 *source, void *, u32 *out, u32 channel);
u32 *__fastcall SoundSource711920_RestartChannel(u8 *source, void *, u32 *out, u32 channel);
// 0x6d2580, 0x6d25a0, 0x6d25c0 (slots 13-15, (out)): the same (session, 0, 1): all channels, open or not.
u32 *__fastcall SoundSource711920_StopAll(u8 *source, void *, u32 *out);
u32 *__fastcall SoundSource711920_PauseAll(u8 *source, void *, u32 *out);
u32 *__fastcall SoundSource711920_RestartAll(u8 *source, void *, u32 *out);
// 0x6d25e0 (slot 30, (out, channel, vector)): a channel not below +4: *out = -1. QSWaveMixGetSourcePosition (session, channel, a stack vector); not 0: *out = -1. Else vector = its x, z, y (whatever +8 says), *out = 0.
u32 *__fastcall SoundSource711920_GetSourcePosition(u8 *source, void *, u32 *out, u32 channel, u32 *vector);
// 0x6d2640 (slot 29, (out, channel, vector, relative)): a channel not below +4: *out = -1. relative: a NaN in the vector, *out = -1; else QSWaveMixSetSourcePosition (session, channel, 0, the vector, +8's swap). Not: QSWaveMixSetPosition with the
// vector as it is. Answered.
u32 *__fastcall SoundSource711920_SetSourcePosition(u8 *source, void *, u32 *out, u32 channel, const u32 *vector, u32 relative);
// 0x6d27b0 (slot 33, (out, from, to)): QSWaveMixSetListenerVelocity (session, to - from with +8's swap, 0), answered.
u32 *__fastcall SoundSource711920_SetListenerVelocity(u8 *source, void *, u32 *out, const float *from, const float *to);
// 0x6d2820 (slot 36, (out, frequency, channel, step)): QSWaveMixSetFrequency (session, channel, 0, the frequency; step above 0: times the float [0x782f40 + 4 * step], below: [0x783540 - 4 * step], rounded through a float), answered.
u32 *__fastcall SoundSource711920_SetFrequencyStep(u8 *source, void *, u32 *out, u32 frequency, u32 channel, s32 step);
// 0x6d28a0 (slot 37, (out, channel, frequency)): QSWaveMixSetFrequency (session, channel, 0, frequency), answered.
u32 *__fastcall SoundSource711920_SetFrequency(u8 *source, void *, u32 *out, u32 channel, u32 frequency);
// 0x6d28d0 (slot 31, (out, channel, cone)): QSWaveMixSetSourceCone (session, channel, 0, the cone's vector, its float moves through the x87, with +8's swap, cone's +0xc, +0x10), answered.
u32 *__fastcall SoundSource711920_SetSourceCone(u8 *source, void *, u32 *out, u32 channel, const u32 *cone);
// 0x6d2930 (slot 24, (out, channel, mapping)): QSWaveMixSetDistanceMapping (session, channel, 0, {0x10, mapping's three dwords}), answered.
u32 *__fastcall SoundSource711920_SetDistanceMapping(u8 *source, void *, u32 *out, u32 channel, const u32 *mapping);
// 0x6d2980 (slot 32, (out, channel, volume); needs open, a channel up to +4 (sic), a volume of 0-0x7f): QSWaveMixSetVolume (session, channel, 0, the word [0x782e40 + 2 * volume]), answered.
u32 *__fastcall SoundSource711920_SetVolume(u8 *source, void *, u32 *out, u32 channel, s32 volume);
// 0x6d29f0 (slot 38, (flags)): SoundSourceBase_Dtor, the source freed for flags bit 0. Returns the source.
u8 *__fastcall SoundSourceBase_DeletingDtor(u8 *source, void *, u32 flags);
// 0x6d2a10 (slot 34, (out, previous, vector, moving, follow); needs open): no moving and follow: slot 33 (a stack out, previous, vector), negative a failure. A NaN in the vector a failure. QSWaveMixSetListenerPosition (session, the vector, +8's swap, 0), answered.
u32 *__fastcall SoundSource711920_SetListenerPosition(u8 *source, void *, u32 *out, const u32 *previous, const u32 *vector, u32 moving, u32 follow);
// 0x6d2b20 (slot 35, (out, front, up); needs open): a NaN in either a failure. QSWaveMixSetListenerOrientation (session, front, up, each with +8's swap, 0), answered.
u32 *__fastcall SoundSource711920_SetListenerOrientation(u8 *source, void *, u32 *out, const u32 *front, const u32 *up);
// 0x6d2ca0 (slot 28, (out, rate); needs open): QSWaveMixSetPanRate (session, 0, 1, rate), answered.
u32 *__fastcall SoundSource711920_SetPanRateAll(u8 *source, void *, u32 *out, u32 rate);
// 0x6d2ce0 (slot 27, (out, speed); needs open): QSWaveMixSetSpeedOfSound (session, speed, 0), answered.
u32 *__fastcall SoundSource711920_SetSpeedOfSound(u8 *source, void *, u32 *out, u32 speed);
// 0x6d2d20 (slot 25, (out, object); needs open): QSWaveMixSetSpeakerPlacement (session, the object's slot 0's answer), answered.
u32 *__fastcall SoundSource711920_SetSpeakerPlacement(u8 *source, void *, u32 *out, u8 *object);
}

#include "gamedata.h"

// Globals by address, by hand (.docs/09-globals.md; a name ending in its address is named for its use, unconfirmed).
GAME_DATA(u16[128], gQsoundmixer_782e40, 0x00782e40, 0x100);
GAME_DATA(float[0x300], gQsoundmixer_782f40, 0x00782f40, 0xc00);   // (the factors up, then down at +0x600: one object, the step is not clamped)
