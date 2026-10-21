// The sound object (vtable 0x70a588, made by 0x6b8380; the target the sound commands run on; the class name is invented): +0x1c a
// voice count (SoundObject_SetVoices), +0x20 its sound mixer (0x54 bytes, made by 0x6b6050), +0x24 its channel (soundchannel). The commands' out is a status
// dword: 0, or -1 when the object lacks the part. +4, +0x10 and +0x28 three vectors (the placement: 0x6b8b60). Reimplementation of
// 0x6b85f0, 0x6b8640, 0x6b8930, 0x6b8b00, 0x6b84f0 and 0x6b8b60; the names are unconfirmed.
#pragma once

#include "../../../re/types.h"

extern "C" {

// 0x6b85f0 (thiscall (out), pops 4): the mixer (when there is one) runs every active voice (SoundMixer_ForEachActive (0)); then
// the channel's slot 4 (out, a stack functor: vtable 0x70a5b0, its data unset). *out = -1 with no channel. Returns out.
u32 *__fastcall SoundObject_Broadcast(u8 *object, void *, u32 *out);

// 0x6b8640 (thiscall (out, value), pops 8): with a channel whose flag 0x400 is set, the mixer's SoundMixer_ForEachActive (value)
// (nothing without a mixer); else SoundObject_Broadcast (out: the value's own stack slot). *out = 0. Returns out.
u32 *__fastcall SoundObject_BroadcastValue(u8 *object, void *, u32 *out, u32 value);

// 0x6b8b00 (thiscall (out, id, result), pops 0xc): the mixer's voice for the id (SoundMixer_FindVoice); with one whose +0xc is set,
// *result = that +0xc's +8 and *out = 0, else *out = -1 (also with no mixer). Returns out.
u32 *__fastcall SoundObject_VoiceInfo(u8 *object, void *, u32 *out, u32 id, u32 *result);

// 0x6b8930 (thiscall (out, id, value), pops 0xc): *out = -1 for id 0; with a channel whose flag 0x400 is set, the mixer's
// SoundMixer_SendVoice (out, id, value); else the object's own slot 6 (out, id). Returns out.
u32 *__fastcall SoundObject_SendVoice(u8 *object, void *, u32 *out, u32 id, u32 value);

// 0x6b84f0 (thiscall (out, count), pops 8): nothing but *out = 0 when the count (+0x1c) is already that; else +0x1c = it, the mixer's
// +0x4c = it, 0x1e at most (Obj_Set4cMin1e; when there is a mixer), and the channel (+0x24) given a settings record with the count (+0x40,
// bit 7: the voices' count, ChannelVoices_SetCount) with the functor 0x70a278 (SoundChannel_Apply). Returns out.
u32 *__fastcall SoundObject_SetVoices(u8 *object, void *, u32 *out, u32 count);

// 0x6b8b60 (thiscall (out, position, direction, value: three pointers to three dwords), pops 0x10): *out = -1 without a channel
// (+0x24). Else the position scaled by the channel's float +0x2c (Vec3_Scale, on a copy); when that, the direction and the value are
// each bytewise what +0x28, +4 and +0x10 hold, nothing but *out = 0. Else those three stored there and the channel's placement set from
// them (soundchannel, 0x6b9930: a local out, +0x28, +4, +0x10); *out = 0. Returns out.
u32 *__fastcall SoundObject_SetPlacement(u8 *object, void *, u32 *out, const u32 *position, const u32 *direction, const u32 *value);

}  // extern "C"
