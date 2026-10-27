// The sound object (vtable 0x70a588, made by 0x6b8380; the target the sound commands run on; the class name is invented): +0x1c a
// voice count (SoundObject_SetVoices), +0x20 its sound mixer (0x54 bytes, made by 0x6b6050), +0x24 its channel (soundchannel). The commands' out is a status
// dword: 0, or -1 when the object lacks the part. +4, +0x10 and +0x28 three vectors (the placement: 0x6b8b60). Reimplementation of
// 0x6b85f0, 0x6b8640, 0x6b8930, 0x6b8b00, 0x6b84f0, 0x6b8b60, 0x6b8690 and 0x6b8720; the names are unconfirmed.
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
// 0x6b89b0 / 0x6b8a10 / 0x6b8a70 (thiscall (out, id), pops 8; names invented): with an id (not 0) and a mixer (+0x20), the mixer's voice
// for it (SoundMixer_FindVoice) gets its slot 6 (out, a stack functor: vtable 0x70a2d0 / 0x70a5b8 / 0x70a5c0, whose slot 0 answers
// 1 / 2 / 3; data the voice's +0x24); else (no voice too) *out = -1. Returns out.
// 0x6b8d00 (thiscall; an SEH frame; name invented): the destructor: vtable 0x70a588, the channel's (+0x24) slot 0 (no arguments) and the
// mixer's (+0x20) slot 0 (1: its deleting destructor), each when there is one.
void __fastcall SoundObject_Dtor(u8 *object);
u32 *__fastcall SoundObject_SendVoiceFunctor1(u8 *object, void *, u32 *out, u32 id);
u32 *__fastcall SoundObject_SendVoiceFunctor2(u8 *object, void *, u32 *out, u32 id);
u32 *__fastcall SoundObject_SendVoiceFunctor3(u8 *object, void *, u32 *out, u32 id);

// 0x6b8930 (thiscall (out, id, value), pops 0xc): *out = -1 for id 0; with a channel whose flag 0x400 is set, the mixer's
// SoundMixer_SendVoice (out, id, value); else the object's own slot 6 (out, id). Returns out.
u32 *__fastcall SoundObject_SendVoice(u8 *object, void *, u32 *out, u32 id, u32 value);

// 0x6b84f0 (thiscall (out, count), pops 8): nothing but *out = 0 when the count (+0x1c) is already that; else +0x1c = it, the mixer's
// +0x4c = it, 0x1e at most (Obj_Set4cMin1e; when there is a mixer), and the channel (+0x24) given a settings record with the count (+0x40,
// bit 7: the voices' count, ChannelVoices_SetCount) with the functor 0x70a278 (SoundChannel_Apply). Returns out.
u32 *__fastcall SoundObject_SetVoices(u8 *object, void *, u32 *out, u32 count);
// 0x6b8480 (thiscall (out, value), pops 8; name invented): its own slot 9; the channel's (+0x24) slot 7 (out: value's own stack slot,
// value, the count +0x1c) gives a status pointer: negative, *out = -1. Else SoundCategory_Get, the sound system globals [0xfb1ffc] and
// [0xfb1f24] = the channel, the channel's +0x60 = &+0x28, its own slot 7 (value's slot, 1: SoundObject_SetVoices) and *out = 0.
// Returns out.
u32 *__fastcall SoundObject_Open(u8 *object, void *, u32 *out, u32 value);

// 0x6b8b60 (thiscall (out, position, direction, value: three pointers to three dwords), pops 0x10): *out = -1 without a channel
// (+0x24). Else the position scaled by the channel's float +0x2c (Vec3_Scale, on a copy); when that, the direction and the value are
// each bytewise what +0x28, +4 and +0x10 hold, nothing but *out = 0. Else those three stored there and the channel's placement set from
// them (soundchannel, 0x6b9930: a local out, +0x28, +4, +0x10); *out = 0. Returns out.
u32 *__fastcall SoundObject_SetPlacement(u8 *object, void *, u32 *out, const u32 *position, const u32 *direction, const u32 *value);

// 0x6b8690 (thiscall (path, string), pops 8): an absolute path (a drive's "X:\" or "X:/", or two slashes of either kind first) into the
// string by its slot 5 (assign, path); else the string assigned the channel's (+0x24) base directory (its +0x30, slot 6) and the path
// appended (slot 8). Returns the last slot's answer.
u32 __fastcall SoundObject_ResolvePath(u8 *object, void *, const char *path, u8 *string);
// 0x6b8720 (thiscall (out, path, name, index), pops 0x10; an SEH frame; name invented): without a channel (+0x24) *out = -1. Else the
// path resolved into a String (SoundObject_ResolvePath) and SoundCategory_Load (out, its text, name, index) on the category singleton.
// Returns out.
s32 *__fastcall SoundObject_LoadCategory(u8 *object, void *, s32 *out, const char *path, const char *name, u32 *index);

// The channel's (+0x24) forwards (names invented); without a channel *out = -1. Each returns out.
// 0x6b8c70 (thiscall (out, x, y, z, radius, &node), pops 0x18): SoundChannel_TryAddOccluder with them.
// 0x6ba1b0 (thiscall (out, index), pops 8; name invented): when the channel's (+0x24) flags (+0x28) have bits 0 and 1
// (Obj_Flags28_Bit1IfBit0), its EAX record index applied (SoundChannel_ApplyEaxRecord); else *out = -1. Returns out.
u32 *__fastcall SoundObject_ApplyEaxRecord(u8 *object, void *, u32 *out, u32 index);

u32 *__fastcall SoundObject_AddOccluder(u8 *object, void *, u32 *out, u32 x, u32 y, u32 z, u32 radius, u8 **node);
// 0x6b8cb0 (thiscall (out, &node), pops 8): SoundChannel_RemoveOccluder with them.
u32 *__fastcall SoundObject_RemoveOccluder(u8 *object, void *, u32 *out, u8 **node);
// 0x6b8570 (thiscall (out, record), pops 8; slot 8): with a record too, SoundCategory_ReleaseRecordBanks (out, record); else *out = -1.
u32 *__fastcall SoundObject_ReleaseRecordBanks(u8 *object, void *, u32 *out, const u8 *record);
// 0x6ba090 (thiscall, this unused (out, index), pops 8; slot 7 of 0x70a2a0): with an index (not 0), SoundCategory_ReleaseRecordAt (out,
// index) on the category singleton (SoundCategory_Get); else *out = -1.
u32 *__fastcall SoundObject_ReleaseRecordAt(u8 *object, void *, u32 *out, u32 index);

// 0x6b8ce0 (thiscall (flags), pops 4; slot 0 of 0x70a588): SoundObject_Dtor, then the object freed with flags bit 0. Returns the object.
u8 *__fastcall SoundObject_DeletingDtor(u8 *object, void *, u32 flags);
// 0x6b8ad0 (thiscall (id, a, b), pops 0xc; slot 3 of 0x70a2a0 and 0x70a588; name invented): with an id (not 0) and a mixer (+0x20),
// SoundMixer_CallVoice10 (id, a, b); else 0.
u32 __fastcall SoundObject_CallVoice10(u8 *object, void *, u32 id, u32 a, u32 b);


// The sound object's save and load (names invented); each returns out with *out = 0.
// 0x6ba0c0 (thiscall (out, stream), pops 8): +0x28, +0x2c and +0x30 written (Stream_Write, its done the stream argument's slot), then the
// mixer's (+0x20) voices (SoundMixer_Save, out a local).
u32 *__fastcall SoundObject_Save(u8 *object, void *, u32 *out, u8 *stream);
// 0x6ba150 (thiscall (out, stream, restore), pops 0xc): three dwords read through the stream's read (+8) and dropped (the first into the
// stream argument's slot, sic), then the mixer's (+0x20) voices (SoundMixer_Load (restore), out the restore argument's slot).
u32 *__fastcall SoundObject_Load(u8 *object, void *, u32 *out, u8 *stream, u32 restore);

}  // extern "C"
